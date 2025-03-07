/* Storing sources' output separately
 *
 * Copyright 2023 Exeldro <https://github.com/exeldro>
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 * 
 * SPDX-License-Identifier: GPLv2
*/

#include "source-record.h"

const char *source_record_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Source Record";
}

bool EncoderAvailable(const char *encoder)
{
	const char *val;
	int i = 0;

	while (obs_enum_encoder_types(i++, &val))
		if (strcmp(val, encoder) == 0)
			return true;

	return false;
}

void calc_min_ts(obs_source_t *parent, obs_source_t *child, void *param)
{
	UNUSED_PARAMETER(parent);
	uint64_t *min_ts = param;
	if (!child || obs_source_audio_pending(child))
		return;
	const uint64_t ts = obs_source_get_audio_timestamp(child);
	if (!ts)
		return;
	if (!*min_ts || ts < *min_ts)
		*min_ts = ts;
}

void mix_audio(obs_source_t *parent, obs_source_t *child, void *param)
{
	UNUSED_PARAMETER(parent);
	if (!child || obs_source_audio_pending(child))
		return;
	const uint64_t ts = obs_source_get_audio_timestamp(child);
	if (!ts)
		return;
	struct obs_source_audio *mixed_audio = param;
	const size_t pos = (size_t)ns_to_audio_frames(
		mixed_audio->samples_per_sec, ts - mixed_audio->timestamp);

	if (pos > AUDIO_OUTPUT_FRAMES)
		return;

	const size_t count = AUDIO_OUTPUT_FRAMES - pos;

	struct obs_source_audio_mix child_audio;
	obs_source_get_audio_mix(child, &child_audio);
	for (size_t ch = 0; ch < (size_t)mixed_audio->speakers; ch++) {
		float *out = ((float *)mixed_audio->data[ch]) + pos;
		float *in = child_audio.output[0].data[ch];
		if (!in)
			continue;
		for (size_t i = 0; i < count; i++) {
			out[i] += in[i];
		}
	}
}

bool audio_input_callback(void *param, uint64_t start_ts_in,
				 uint64_t end_ts_in, uint64_t *out_ts,
				 uint32_t mixers,
				 struct audio_output_data *mixes)
{
	UNUSED_PARAMETER(end_ts_in);
	struct source_record_filter_context *filter = param;
	if (filter->closing || obs_source_removed((const obs_source_t*)filter->source)) {
		*out_ts = start_ts_in;
		return true;
	}

	obs_source_t *audio_source = NULL;
	if (filter->audio_source) {
		audio_source = obs_weak_source_get_source(filter->audio_source);
		if (audio_source)
			obs_source_release(audio_source);
	} else {
		audio_source = obs_filter_get_parent(filter->source);
	}
	if (!audio_source || obs_source_removed(audio_source)) {
		*out_ts = start_ts_in;
		return true;
	}

	const uint32_t flags = obs_source_get_output_flags(audio_source);
	if ((flags & OBS_SOURCE_COMPOSITE) != 0) {
		uint64_t min_ts = 0;
		obs_source_enum_active_tree(audio_source, calc_min_ts, &min_ts);
		if (min_ts) {
			struct obs_source_audio mixed_audio = {0};
			for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
				mixed_audio.data[i] = (uint8_t *)mixes->data[i];
			}
			mixed_audio.timestamp = min_ts;
			mixed_audio.speakers =
				audio_output_get_channels(filter->audio_output);
			mixed_audio.samples_per_sec =
				audio_output_get_sample_rate(
					filter->audio_output);
			mixed_audio.format = AUDIO_FORMAT_FLOAT_PLANAR;
			obs_source_enum_active_tree(audio_source, mix_audio,
						    &mixed_audio);

			for (size_t mix_idx = 0; mix_idx < MAX_AUDIO_MIXES;
			     mix_idx++) {
				if ((mixers & (1 << mix_idx)) == 0)
					continue;
				// clamp audio
				for (size_t ch = 0;
				     ch < (size_t)mixed_audio.speakers; ch++) {
					float *mix_data =
						mixes[mix_idx].data[ch];
					float *mix_end =
						&mix_data[AUDIO_OUTPUT_FRAMES];

					while (mix_data < mix_end) {
						float val = *mix_data;
						val = (val > 1.0f) ? 1.0f : val;
						val = (val < -1.0f) ? -1.0f
								    : val;
						*(mix_data++) = val;
					}
				}
			}
			*out_ts = min_ts;
		} else {
			*out_ts = start_ts_in;
		}
		return true;
	}
	if ((flags & OBS_SOURCE_AUDIO) == 0) {
		*out_ts = start_ts_in;
		return true;
	}

	const uint64_t source_ts = obs_source_get_audio_timestamp(audio_source);
	if (!source_ts) {
		*out_ts = start_ts_in;
		return true;
	}

	if (obs_source_audio_pending(audio_source))
		return false;

	struct obs_source_audio_mix audio;
	obs_source_get_audio_mix(audio_source, &audio);

	const size_t channels = audio_output_get_channels(filter->audio_output);
	for (size_t mix_idx = 0; mix_idx < MAX_AUDIO_MIXES; mix_idx++) {
		if ((mixers & (1 << mix_idx)) == 0)
			continue;
		for (size_t ch = 0; ch < channels; ch++) {
			float *out = mixes[mix_idx].data[ch];
			float *in = audio.output[0].data[ch];
			if (!in)
				continue;
			for (size_t i = 0; i < AUDIO_OUTPUT_FRAMES; i++) {
				out[i] += in[i];
				if (out[i] > 1.0f)
					out[i] = 1.0f;
				if (out[i] < -1.0f)
					out[i] = -1.0f;
			}
		}
	}

	*out_ts = source_ts;

	return true;
}

void *start_file_output_thread(void *data)
{
	struct source_record_filter_context *context = data;
	if (obs_output_start(context->fileOutput)) {
		if (!context->output_active) {
			context->output_active = true;
			obs_source_inc_showing(
				obs_filter_get_parent(context->source));
		}
	}
	context->starting_file_output = false;
	return NULL;
}

void *start_stream_output_thread(void *data)
{
	struct source_record_filter_context *context = data;
	if (obs_output_start(context->streamOutput)) {
		if (!context->output_active) {
			context->output_active = true;
			obs_source_inc_showing(
				obs_filter_get_parent(context->source));
		}
	}
	context->starting_stream_output = false;
	return NULL;
}

void *force_stop_output_thread(void *data)
{
	obs_output_t *fileOutput = data;
	obs_output_force_stop(fileOutput);
	obs_output_release(fileOutput);
	return NULL;
}

void *start_replay_thread(void *data)
{
	struct source_record_filter_context *context = data;
	if (obs_output_start(context->replayOutput)) {
		if (!context->output_active) {
			context->output_active = true;
			obs_source_inc_showing(
				obs_filter_get_parent(context->source));
		}
	}
	context->starting_replay_output = false;
	return NULL;
}

void ensure_directory(char *path)
{
#ifdef _WIN32
	char *backslash = strrchr(path, '\\');
	if (backslash)
		*backslash = '/';
#endif

	char *slash = strrchr(path, '/');
	if (slash) {
		*slash = 0;
		os_mkdirs(path);
		*slash = '/';
	}

#ifdef _WIN32
	if (backslash)
		*backslash = '\\';
#endif
}

void start_file_output(struct source_record_filter_context *filter,
			      obs_data_t *settings)
{
	obs_data_t *s = obs_data_create();
	char path[512];
	char *filename = os_generate_formatted_filename(
		obs_data_get_string(settings, "rec_format"), true,
		obs_data_get_string(settings, "filename_formatting"));
	snprintf(path, 512, "%s/%s", obs_data_get_string(settings, "path"),
		 filename);
	bfree(filename);
	ensure_directory(path);
	obs_data_set_string(s, "path", path);
	if (!filter->fileOutput) {
		filter->fileOutput = obs_output_create(
			"ffmpeg_muxer", obs_source_get_name(filter->source), s,
			NULL);
	} else {
		obs_output_update(filter->fileOutput, s);
	}
	obs_data_release(s);
	if (filter->encoder) {
		obs_encoder_set_video(filter->encoder, filter->video_output);
		obs_output_set_video_encoder(filter->fileOutput,
					     filter->encoder);
	}

	if (filter->aacTrack) {
		obs_encoder_set_audio(filter->aacTrack, filter->audio_output);
		obs_output_set_audio_encoder(filter->fileOutput,
					     filter->aacTrack, 0);
	}

	filter->starting_file_output = true;

	pthread_t thread;
	pthread_create(&thread, NULL, start_file_output_thread, filter);
}

#define FTL_PROTOCOL "ftl"
#define RTMP_PROTOCOL "rtmp"

void start_stream_output(struct source_record_filter_context *filter,
				obs_data_t *settings)
{
	if (!filter->service) {
		filter->service = obs_service_create(
			"rtmp_custom", obs_source_get_name(filter->source),
			settings, NULL);
	} else {
		obs_service_update(filter->service, settings);
	}
	obs_service_apply_encoder_settings(filter->service, settings, NULL);

	const char *type = obs_service_get_preferred_output_type(filter->service);
	if (!type) {
		type = "rtmp_output";
		const char *url = obs_service_get_connect_info(filter->service, OBS_SERVICE_CONNECT_INFO_SERVER_URL);
		if (url != NULL &&
		    strncmp(url, FTL_PROTOCOL, strlen(FTL_PROTOCOL)) == 0) {
			type = "ftl_output";
		} else if (url != NULL && strncmp(url, RTMP_PROTOCOL,
						  strlen(RTMP_PROTOCOL)) != 0) {
			type = "ffmpeg_mpegts_muxer";
		}
	}

	if (!filter->streamOutput) {
		filter->streamOutput = obs_output_create(
			type, obs_source_get_name(filter->source), settings,
			NULL);
	} else {
		obs_output_update(filter->streamOutput, settings);
	}
	obs_output_set_service(filter->streamOutput, filter->service);

	if (filter->encoder) {
		obs_encoder_set_video(filter->encoder, filter->video_output);
		obs_output_set_video_encoder(filter->streamOutput,
					     filter->encoder);
	}

	if (filter->aacTrack) {
		obs_encoder_set_audio(filter->aacTrack, filter->audio_output);
		obs_output_set_audio_encoder(filter->streamOutput,
					     filter->aacTrack, 0);
	}

	filter->starting_stream_output = true;

	pthread_t thread;
	pthread_create(&thread, NULL, start_stream_output_thread, filter);
}

void start_replay_output(struct source_record_filter_context *filter,
				obs_data_t *settings)
{
	obs_data_t *s = obs_data_create();

	obs_data_set_string(s, "directory",
			    obs_data_get_string(settings, "path"));
	obs_data_set_string(s, "format",
			    obs_data_get_string(settings,
						"filename_formatting"));
	obs_data_set_string(s, "extension",
			    obs_data_get_string(settings, "rec_format"));
	obs_data_set_bool(s, "allow_spaces", true);
	filter->replay_buffer_duration =
		obs_data_get_int(settings, "replay_duration");
	obs_data_set_int(s, "max_time_sec", filter->replay_buffer_duration);
	obs_data_set_int(s, "max_size_mb", 10000);
	if (!filter->replayOutput) {
		obs_data_t *hotkeys =
			obs_data_get_obj(settings, "replay_hotkeys");
		struct dstr name;
		obs_source_t *parent = obs_filter_get_parent(filter->source);
		if (parent) {
			dstr_init_copy(&name, obs_source_get_name(parent));
			dstr_cat(&name, " - ");
			dstr_cat(&name, obs_source_get_name(filter->source));
		} else {
			dstr_init_copy(&name,
				       obs_source_get_name(filter->source));
		}

		filter->replayOutput = obs_output_create(
			"replay_buffer", name.array, s, hotkeys);
		dstr_free(&name);
		obs_data_release(hotkeys);
	} else {
		obs_output_update(filter->replayOutput, s);
	}
	obs_data_release(s);
	if (filter->encoder) {
		obs_encoder_set_video(filter->encoder, filter->video_output);
		if (obs_output_get_video_encoder(filter->replayOutput) !=
		    filter->encoder)
			obs_output_set_video_encoder(filter->replayOutput,
						     filter->encoder);
	}

	if (filter->aacTrack) {
		obs_encoder_set_audio(filter->aacTrack, filter->audio_output);

		if (obs_output_get_audio_encoder(filter->replayOutput, 0) !=
		    filter->aacTrack)
			obs_output_set_audio_encoder(filter->replayOutput,
						     filter->aacTrack, 0);
	}

	filter->starting_replay_output = true;

	pthread_t thread;
	pthread_create(&thread, NULL, start_replay_thread, filter);
}

const char *get_encoder_id(obs_data_t *settings)
{
	const char *enc_id = obs_data_get_string(settings, "encoder");
	if (strlen(enc_id) == 0 || strcmp(enc_id, "x264") == 0 ||
	    strcmp(enc_id, "x264_lowcpu") == 0) {
		enc_id = "obs_x264";
	} else if (strcmp(enc_id, "qsv") == 0) {
		enc_id = "obs_qsv11_v2";
	} else if (strcmp(enc_id, "qsv_av1") == 0) {
		enc_id = "obs_qsv11_av1";
	} else if (strcmp(enc_id, "amd") == 0) {
		enc_id = "h264_texture_amf";
	} else if (strcmp(enc_id, "amd_hevc") == 0) {
		enc_id = "h265_texture_amf";
	} else if (strcmp(enc_id, "amd_av1") == 0) {
		enc_id = "av1_texture_amf";
	} else if (strcmp(enc_id, "nvenc") == 0) {
		enc_id = EncoderAvailable("jim_nvenc") ? "jim_nvenc"
						       : "ffmpeg_nvenc";
	} else if (strcmp(enc_id, "nvenc_hevc") == 0) {
		enc_id = EncoderAvailable("jim_hevc_nvenc")
				 ? "jim_hevc_nvenc"
				 : "ffmpeg_hevc_nvenc";
	} else if (strcmp(enc_id, "nvenc_av1") == 0) {
		enc_id = "jim_av1_nvenc";
	} else if (strcmp(enc_id, "apple_h264") == 0) {
		enc_id = "com.apple.videotoolbox.videoencoder.ave.avc";
	} else if (strcmp(enc_id, "apple_hevc") == 0) {
		enc_id = "com.apple.videotoolbox.videoencoder.ave.hevc";
	}
	return enc_id;
}

void source_record_filter_update(void *data, obs_data_t *settings)
{
	struct source_record_filter_context *filter = data;

	const long long record_mode = obs_data_get_int(settings, "record_mode");
	const long long stream_mode = obs_data_get_int(settings, "stream_mode");
	const bool replay_buffer = obs_data_get_bool(settings, "replay_buffer");
	if (record_mode != OUTPUT_MODE_NONE ||
	    stream_mode != OUTPUT_MODE_NONE || replay_buffer) {
		const char *enc_id = get_encoder_id(settings);
		if (!filter->encoder ||
		    strcmp(obs_encoder_get_id(filter->encoder), enc_id) != 0) {
			obs_encoder_release(filter->encoder);
			filter->encoder = obs_video_encoder_create(
				enc_id, obs_source_get_name(filter->source),
				settings, NULL);

			obs_encoder_set_scaled_size(filter->encoder, 0, 0);
			obs_encoder_set_video(filter->encoder,
					      filter->video_output);
			if (filter->fileOutput &&
			    obs_output_get_video_encoder(filter->fileOutput) !=
				    filter->encoder)
				obs_output_set_video_encoder(filter->fileOutput,
							     filter->encoder);
			if (filter->streamOutput &&
			    obs_output_get_video_encoder(
				    filter->streamOutput) != filter->encoder)
				obs_output_set_video_encoder(
					filter->streamOutput, filter->encoder);
			if (filter->replayOutput &&
			    obs_output_get_video_encoder(
				    filter->replayOutput) != filter->encoder)
				obs_output_set_video_encoder(
					filter->replayOutput, filter->encoder);
		} else if (!obs_encoder_active(filter->encoder)) {
			obs_encoder_update(filter->encoder, settings);
		}
		const int audio_track =
			obs_data_get_bool(settings, "different_audio")
				? (int)obs_data_get_int(settings, "audio_track")
				: 0;
		if (!filter->audio_output) {
			if (audio_track > 0) {
				filter->audio_output = obs_get_audio();
			} else {
				struct audio_output_info oi = {0};
				oi.name = obs_source_get_name(filter->source);
				oi.speakers = SPEAKERS_STEREO;
				oi.samples_per_sec =
					audio_output_get_sample_rate(
						obs_get_audio());
				oi.format = AUDIO_FORMAT_FLOAT_PLANAR;
				oi.input_param = filter;
				oi.input_callback = audio_input_callback;
				audio_output_open(&filter->audio_output, &oi);
			}
		} else if (audio_track > 0 && filter->audio_track <= 0) {
			audio_output_close(filter->audio_output);
			filter->audio_output = obs_get_audio();
		} else if (audio_track <= 0 && filter->audio_track > 0) {
			filter->audio_output = NULL;
			struct audio_output_info oi = {0};
			oi.name = obs_source_get_name(filter->source);
			oi.speakers = SPEAKERS_STEREO;
			oi.samples_per_sec =
				audio_output_get_sample_rate(obs_get_audio());
			oi.format = AUDIO_FORMAT_FLOAT_PLANAR;
			oi.input_param = filter;
			oi.input_callback = audio_input_callback;
			audio_output_open(&filter->audio_output, &oi);
		}

		if (!filter->aacTrack || filter->audio_track != audio_track) {
			if (filter->aacTrack) {
				obs_encoder_release(filter->aacTrack);
				filter->aacTrack = NULL;
			}
			if (audio_track > 0) {
				filter->aacTrack = obs_audio_encoder_create(
					"ffmpeg_aac",
					obs_source_get_name(filter->source),
					NULL, audio_track - 1, NULL);
			} else {
				filter->aacTrack = obs_audio_encoder_create(
					"ffmpeg_aac",
					obs_source_get_name(filter->source),
					NULL, 0, NULL);
			}
			if (filter->audio_output)
				obs_encoder_set_audio(filter->aacTrack,
						      filter->audio_output);

			if (filter->fileOutput)
				obs_output_set_audio_encoder(filter->fileOutput,
							     filter->aacTrack,
							     0);
			if (filter->replayOutput)
				obs_output_set_audio_encoder(
					filter->replayOutput, filter->aacTrack,
					0);
		}
		filter->audio_track = audio_track;
	}
	bool record = false;
	if (record_mode == OUTPUT_MODE_ALWAYS) {
		record = true;
	} else if (record_mode == OUTPUT_MODE_RECORDING) {
		record = obs_frontend_recording_active();
	} else if (record_mode == OUTPUT_MODE_STREAMING) {
		record = obs_frontend_streaming_active();
	} else if (record_mode == OUTPUT_MODE_STREAMING_OR_RECORDING) {
		record = obs_frontend_streaming_active() ||
			 obs_frontend_recording_active();
	} else if (record_mode == OUTPUT_MODE_VIRTUAL_CAMERA) {
		record = obs_frontend_virtualcam_active();
	}

	if (record != filter->record) {
		if (record) {
			if (obs_source_enabled(filter->source) &&
			    filter->video_output)
				start_file_output(filter, settings);
		} else {
			if (filter->fileOutput) {
				pthread_t thread;
				pthread_create(&thread, NULL,
					       force_stop_output_thread,
					       filter->fileOutput);
				filter->fileOutput = NULL;
			}
		}
		filter->record = record;
	}

	if (replay_buffer != filter->replayBuffer) {
		if (replay_buffer) {
			if (obs_source_enabled(filter->source) &&
			    filter->video_output)
				start_replay_output(filter, settings);
		} else if (filter->replayOutput) {
			obs_data_t *hotkeys =
				obs_hotkeys_save_output(filter->replayOutput);
			obs_data_set_obj(settings, "replay_hotkeys", hotkeys);
			obs_data_release(hotkeys);
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       filter->replayOutput);
			filter->replayOutput = NULL;
		}

		filter->replayBuffer = replay_buffer;
	} else if (replay_buffer && filter->replayOutput &&
		   obs_source_enabled(filter->source)) {
		if (filter->replay_buffer_duration !=
		    obs_data_get_int(settings, "replay_duration")) {
			obs_data_t *hotkeys =
				obs_hotkeys_save_output(filter->replayOutput);
			obs_data_set_obj(settings, "replay_hotkeys", hotkeys);
			obs_data_release(hotkeys);
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       filter->replayOutput);
			filter->replayOutput = NULL;
			start_replay_output(filter, settings);
		}
	}

	bool stream = false;
	if (stream_mode == OUTPUT_MODE_ALWAYS) {
		stream = true;
	} else if (stream_mode == OUTPUT_MODE_RECORDING) {
		stream = obs_frontend_recording_active();
	} else if (stream_mode == OUTPUT_MODE_STREAMING) {
		stream = obs_frontend_streaming_active();
	} else if (stream_mode == OUTPUT_MODE_STREAMING_OR_RECORDING) {
		stream = obs_frontend_streaming_active() ||
			 obs_frontend_recording_active();
	} else if (stream_mode == OUTPUT_MODE_VIRTUAL_CAMERA) {
		stream = obs_frontend_virtualcam_active();
	}

	if (stream != filter->stream) {
		if (stream) {
			if (obs_source_enabled(filter->source) &&
			    filter->video_output)
				start_stream_output(filter, settings);
		} else {
			if (filter->streamOutput) {
				pthread_t thread;
				pthread_create(&thread, NULL,
					       force_stop_output_thread,
					       filter->streamOutput);
				filter->streamOutput = NULL;
			}
		}
		filter->stream = stream;
	}

	if (!replay_buffer && !record && !stream) {
		if (filter->encoder) {
			obs_encoder_release(filter->encoder);
			filter->encoder = NULL;
		}
		if (filter->aacTrack) {
			obs_encoder_release(filter->aacTrack);
			filter->aacTrack = NULL;
		}
	}

	vec4_from_rgba(&filter->backgroundColor,
		       (uint32_t)obs_data_get_int(settings, "backgroundColor"));

	if (obs_data_get_bool(settings, "different_audio")) {
		const char *source_name =
			obs_data_get_string(settings, "audio_source");
		if (!strlen(source_name)) {
			if (filter->audio_source) {
				obs_weak_source_release(filter->audio_source);
				filter->audio_source = NULL;
			}
		} else {
			obs_source_t *source = obs_weak_source_get_source(
				filter->audio_source);
			if (source)
				obs_source_release(source);
			if (!source ||
			    strcmp(source_name, obs_source_get_name(source)) !=
				    0) {
				if (filter->audio_source) {
					obs_weak_source_release(
						filter->audio_source);
					filter->audio_source = NULL;
				}
				source = obs_get_source_by_name(source_name);
				if (source) {
					filter->audio_source =
						obs_source_get_weak_source(
							source);
					obs_source_release(source);
				}
			}
		}

	} else if (filter->audio_source) {
		obs_weak_source_release(filter->audio_source);
		filter->audio_source = NULL;
	}
}

void source_record_filter_save(void *data, obs_data_t *settings)
{
	struct source_record_filter_context *filter = data;
	if (filter->replayOutput) {
		obs_data_t *hotkeys =
			obs_hotkeys_save_output(filter->replayOutput);
		obs_data_set_obj(settings, "replay_hotkeys", hotkeys);
		obs_data_release(hotkeys);
	}
}

void source_record_filter_defaults(obs_data_t *settings)
{
	config_t *config = obs_frontend_get_profile_config();

	const char *mode = config_get_string(config, "Output", "Mode");
	const char *type = config_get_string(config, "AdvOut", "RecType");
	const char *adv_path =
		strcmp(type, "Standard") != 0 || strcmp(type, "standard") != 0
			? config_get_string(config, "AdvOut", "FFFilePath")
			: config_get_string(config, "AdvOut", "RecFilePath");
	bool adv_out = strcmp(mode, "Advanced") == 0 ||
		       strcmp(mode, "advanced") == 0;
	const char *rec_path =
		adv_out ? adv_path
			: config_get_string(config, "SimpleOutput", "FilePath");

	obs_data_set_default_string(settings, "path", rec_path);
	obs_data_set_default_string(settings, "filename_formatting",
				    config_get_string(config, "Output",
						      "FilenameFormatting"));
	obs_data_set_default_string(
		settings, "rec_format",
		config_get_string(config, adv_out ? "AdvOut" : "SimpleOutput",
				  "RecFormat"));

	obs_data_set_default_int(settings, "backgroundColor", 0);

	const char *enc_id;
	if (adv_out) {
		enc_id = config_get_string(config, "AdvOut", "RecEncoder");
		if (strcmp(enc_id, "none") == 0 || strcmp(enc_id, "None") == 0)
			enc_id = config_get_string(config, "AdvOut", "Encoder");
		else if (strcmp(enc_id, "jim_nvenc") == 0)
			enc_id = "nvenc";
		else
			obs_data_set_default_string(settings, "encoder",
						    enc_id);
	} else {
		const char *quality =
			config_get_string(config, "SimpleOutput", "RecQuality");
		if (strcmp(quality, "Stream") == 0 ||
		    strcmp(quality, "stream") == 0) {
			enc_id = config_get_string(config, "SimpleOutput",
						   "StreamEncoder");
		} else if (strcmp(quality, "Lossless") == 0 ||
			   strcmp(quality, "lossless") == 0) {
			enc_id = "ffmpeg_output";
		} else {
			enc_id = config_get_string(config, "SimpleOutput",
						   "RecEncoder");
		}
		obs_data_set_default_string(settings, "encoder", enc_id);
	}
	obs_data_set_default_int(settings, "replay_duration", 5);
}

void frontend_event(enum obs_frontend_event event, void *data)
{
	struct source_record_filter_context *context = data;
	if (event == OBS_FRONTEND_EVENT_STREAMING_STARTING ||
	    event == OBS_FRONTEND_EVENT_STREAMING_STARTED ||
	    event == OBS_FRONTEND_EVENT_STREAMING_STOPPING ||
	    event == OBS_FRONTEND_EVENT_STREAMING_STOPPED ||
	    event == OBS_FRONTEND_EVENT_RECORDING_STARTING ||
	    event == OBS_FRONTEND_EVENT_RECORDING_STARTED ||
	    event == OBS_FRONTEND_EVENT_RECORDING_STOPPING ||
	    event == OBS_FRONTEND_EVENT_RECORDING_STOPPED ||
	    event == OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED ||
	    event == OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED) {
		obs_source_update(context->source, NULL);
	} else if (event == OBS_FRONTEND_EVENT_EXIT ||
		   event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP) {
		context->closing = true;
	}
}

void *source_record_filter_create(obs_data_t *settings,
					 obs_source_t *source)
{
	struct source_record_filter_context *context =
		bzalloc(sizeof(struct source_record_filter_context));
	context->source = source;

	context->enableHotkey = OBS_INVALID_HOTKEY_PAIR_ID;
	source_record_filter_update(context, settings);
	obs_frontend_add_event_callback(frontend_event, context);
	return context;
}

void source_record_filter_destroy(void *data)
{
	struct source_record_filter_context *context = data;
	context->closing = true;
	if (context->output_active) {
		obs_source_dec_showing(obs_filter_get_parent(context->source));
		context->output_active = false;
	}
	obs_frontend_remove_event_callback(frontend_event, context);

	if (context->fileOutput) {
		obs_output_force_stop(context->fileOutput);
		obs_output_release(context->fileOutput);
		context->fileOutput = NULL;
	}
	if (context->streamOutput) {
		obs_output_force_stop(context->streamOutput);
		obs_output_release(context->streamOutput);
		context->streamOutput = NULL;
	}
	if (context->replayOutput) {
		obs_output_force_stop(context->replayOutput);
		obs_output_release(context->replayOutput);
		context->replayOutput = NULL;
	}

	if (context->video_output) {
		obs_view_remove(context->view);
		obs_view_set_source(context->view, 0, NULL);
		context->video_output = NULL;
	}

	if (context->enableHotkey != OBS_INVALID_HOTKEY_PAIR_ID)
		obs_hotkey_pair_unregister(context->enableHotkey);

	obs_encoder_release(context->aacTrack);
	obs_encoder_release(context->encoder);

	obs_weak_source_release(context->audio_source);
	context->audio_source = NULL;

	if (context->audio_track <= 0)
		audio_output_close(context->audio_output);

	obs_service_release(context->service);

	obs_view_destroy(context->view);

	bfree(context);
}

bool source_record_enable_hotkey(void *data, obs_hotkey_pair_id id,
					obs_hotkey_t *hotkey, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);
	struct source_record_filter_context *context = data;
	if (!pressed)
		return false;

	if (obs_source_enabled(context->source))
		return false;

	obs_source_set_enabled(context->source, true);
	return true;
}

bool source_record_disable_hotkey(void *data, obs_hotkey_pair_id id,
					 obs_hotkey_t *hotkey, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);
	struct source_record_filter_context *context = data;
	if (!pressed)
		return false;
	if (!obs_source_enabled(context->source))
		return false;
	obs_source_set_enabled(context->source, false);
	return true;
}

void source_record_filter_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	struct source_record_filter_context *context = data;
	if (context->closing)
		return;

	obs_source_t *parent = obs_filter_get_parent(context->source);
	if (!parent)
		return;

	if (context->enableHotkey == OBS_INVALID_HOTKEY_PAIR_ID)
		context->enableHotkey = obs_hotkey_pair_register_source(
			parent, "source_record.enable",
			obs_module_text("SourceRecordEnable"),
			"source_record.disable",
			obs_module_text("SourceRecordDisable"),
			source_record_enable_hotkey,
			source_record_disable_hotkey, context, context);

	uint32_t width = obs_source_get_width(parent);
	width += (width & 1);
	uint32_t height = obs_source_get_height(parent);
	height += (height & 1);
	if (context->width != width || context->height != height ||
	    (!context->video_output && width && height)) {
		struct obs_video_info ovi = {0};
		obs_get_video_info(&ovi);

		ovi.base_width = width;
		ovi.base_height = height;
		ovi.output_width = width;
		ovi.output_height = height;

		if (!context->view)
			context->view = obs_view_create();

		const bool restart = !!context->video_output;
		if (restart)
			obs_view_remove(context->view);

		obs_view_set_source(context->view, 0, parent);

		context->video_output = obs_view_add2(context->view, &ovi);
		if (context->video_output) {
			context->width = width;
			context->height = height;
			if (restart)
				context->restart = true;
		}
	}

	if (context->restart && context->output_active) {
		if (context->fileOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->fileOutput);
			context->fileOutput = NULL;
		}
		if (context->streamOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->streamOutput);
			context->streamOutput = NULL;
		}
		if (context->replayOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->replayOutput);
			context->replayOutput = NULL;
		}
		context->output_active = false;
		context->restart = false;
		obs_source_dec_showing(obs_filter_get_parent(context->source));
	} else if (!context->output_active &&
		   obs_source_enabled(context->source) &&
		   (context->replayBuffer || context->record ||
		    context->stream)) {
		if (context->starting_file_output ||
		    context->starting_stream_output ||
		    context->starting_replay_output || !context->video_output)
			return;
		obs_data_t *s = obs_source_get_settings(context->source);
		if (context->record)
			start_file_output(context, s);
		if (context->stream)
			start_stream_output(context, s);
		if (context->replayBuffer)
			start_replay_output(context, s);
		obs_data_release(s);
	} else if (context->output_active &&
		   !obs_source_enabled(context->source)) {
		if (context->fileOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->fileOutput);
			context->fileOutput = NULL;
		}
		if (context->streamOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->streamOutput);
			context->streamOutput = NULL;
		}
		if (context->replayOutput) {
			pthread_t thread;
			pthread_create(&thread, NULL, force_stop_output_thread,
				       context->replayOutput);
			context->replayOutput = NULL;
		}
		context->output_active = false;
		obs_source_dec_showing(obs_filter_get_parent(context->source));
	}
}

bool encoder_changed(void *data, obs_properties_t *props,
			    obs_property_t *property, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(property);
	UNUSED_PARAMETER(settings);
	obs_properties_remove_by_name(props, "encoder_group");
	obs_properties_remove_by_name(props, "plugin_info");
	const char *enc_id = get_encoder_id(settings);
	obs_properties_t *enc_props = obs_get_encoder_properties(enc_id);
	if (enc_props) {
		obs_properties_add_group(props, "encoder_group",
					 obs_encoder_get_display_name(enc_id),
					 OBS_GROUP_NORMAL, enc_props);
	}
	obs_properties_add_text(
		props, "plugin_info",
		"<a href=\"https://obsproject.com/forum/resources/source-record.1285/\">Source Record</a> (" PROJECT_VERSION
		") by <a href=\"https://www.exeldro.com\">Exeldro</a>",
		OBS_TEXT_INFO);
	return true;
}

bool list_add_audio_sources(void *data, obs_source_t *source)
{
	obs_property_t *p = data;
	const uint32_t flags = obs_source_get_output_flags(source);
	if ((flags & OBS_SOURCE_COMPOSITE) != 0) {
		obs_property_list_add_string(p, obs_source_get_name(source),
					     obs_source_get_name(source));
	} else if ((flags & OBS_SOURCE_AUDIO) != 0) {
		obs_property_list_add_string(p, obs_source_get_name(source),
					     obs_source_get_name(source));
	}
	return true;
}

obs_properties_t *source_record_filter_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_t *record = obs_properties_create();

	obs_property_t *p = obs_properties_add_list(
		record, "record_mode", obs_module_text("RecordMode"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(p, obs_module_text("None"), OUTPUT_MODE_NONE);
	obs_property_list_add_int(p, obs_module_text("Always"),
				  OUTPUT_MODE_ALWAYS);
	obs_property_list_add_int(p, obs_module_text("Streaming"),
				  OUTPUT_MODE_STREAMING);
	obs_property_list_add_int(p, obs_module_text("Recording"),
				  OUTPUT_MODE_RECORDING);
	obs_property_list_add_int(p, obs_module_text("StreamingOrRecording"),
				  OUTPUT_MODE_STREAMING_OR_RECORDING);
	obs_property_list_add_int(p, obs_module_text("VirtualCamera"),
				  OUTPUT_MODE_VIRTUAL_CAMERA);

	obs_properties_add_path(record, "path", obs_module_text("Path"),
				OBS_PATH_DIRECTORY, NULL, NULL);
	obs_properties_add_text(record, "filename_formatting",
				obs_module_text("FilenameFormatting"),
				OBS_TEXT_DEFAULT);
	p = obs_properties_add_list(record, "rec_format",
				    obs_module_text("RecFormat"),
				    OBS_COMBO_TYPE_EDITABLE,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, "flv", "flv");
	obs_property_list_add_string(p, "mp4", "mp4");
	obs_property_list_add_string(p, "mov", "mov");
	obs_property_list_add_string(p, "mkv", "mkv");
	obs_property_list_add_string(p, "ts", "ts");
	obs_property_list_add_string(p, "m3u8", "m3u8");

	obs_properties_add_group(props, "record", obs_module_text("Record"),
				 OBS_GROUP_NORMAL, record);

	obs_properties_t *replay = obs_properties_create();

	p = obs_properties_add_int(replay, "replay_duration",
				   obs_module_text("Duration"), 1, 1000, 1);
	obs_property_int_set_suffix(p, "s");

	obs_properties_add_group(props, "replay_buffer",
				 obs_module_text("ReplayBuffer"),
				 OBS_GROUP_CHECKABLE, replay);

	obs_properties_t *stream = obs_properties_create();

	p = obs_properties_add_list(stream, "stream_mode",
				    obs_module_text("StreamMode"),
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(p, obs_module_text("None"), OUTPUT_MODE_NONE);
	obs_property_list_add_int(p, obs_module_text("Always"),
				  OUTPUT_MODE_ALWAYS);
	obs_property_list_add_int(p, obs_module_text("Streaming"),
				  OUTPUT_MODE_STREAMING);
	obs_property_list_add_int(p, obs_module_text("Recording"),
				  OUTPUT_MODE_RECORDING);
	obs_property_list_add_int(p, obs_module_text("StreamingOrRecording"),
				  OUTPUT_MODE_STREAMING_OR_RECORDING);
	obs_property_list_add_int(p, obs_module_text("VirtualCamera"),
				  OUTPUT_MODE_VIRTUAL_CAMERA);

	obs_properties_add_text(stream, "server", obs_module_text("Server"),
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(stream, "key", obs_module_text("Key"),
				OBS_TEXT_PASSWORD);

	obs_properties_add_group(props, "stream", obs_module_text("Stream"),
				 OBS_GROUP_NORMAL, stream);

	obs_properties_t *background = obs_properties_create();

	obs_properties_add_color(background, "backgroundColor",
				 obs_module_text("BackgroundColor"));

	obs_properties_add_group(props, "background",
				 obs_module_text("Background"),
				 OBS_GROUP_NORMAL, background);

	obs_properties_t *audio = obs_properties_create();

	p = obs_properties_add_list(audio, "audio_track",
				    obs_module_text("AudioTrack"),
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text("None"), 0);
	const char *track = obs_module_text("Track");
	for (int i = 1; i <= MAX_AUDIO_MIXES; i++) {
		char buffer[64];
		snprintf(buffer, 64, "%s %i", track, i);
		obs_property_list_add_int(p, buffer, i);
	}

	p = obs_properties_add_list(audio, "audio_source",
				    obs_module_text("Source"),
				    OBS_COMBO_TYPE_EDITABLE,
				    OBS_COMBO_FORMAT_STRING);
	obs_enum_sources(list_add_audio_sources, p);
	obs_enum_scenes(list_add_audio_sources, p);

	obs_properties_add_group(props, "different_audio",
				 obs_module_text("DifferentAudio"),
				 OBS_GROUP_CHECKABLE, audio);

	p = obs_properties_add_list(props, "encoder",
				    obs_module_text("Encoder"),
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);

	obs_property_list_add_string(p, obs_module_text("Software"), "x264");
	if (EncoderAvailable("obs_qsv11"))
		obs_property_list_add_string(p, obs_module_text("QSV.H264"),
					     "qsv");
	if (EncoderAvailable("obs_qsv11_av1"))
		obs_property_list_add_string(p, obs_module_text("QSV.AV1"),
					     "qsv_av1");
	if (EncoderAvailable("ffmpeg_nvenc"))
		obs_property_list_add_string(p, obs_module_text("NVENC.H264"),
					     "nvenc");
	if (EncoderAvailable("jim_av1_nvenc"))
		obs_property_list_add_string(p, obs_module_text("NVENC.AV1"),
					     "nvenc_av1");
	if (EncoderAvailable("h265_texture_amf"))
		obs_property_list_add_string(p, obs_module_text("AMD.HEVC"),
					     "amd_hevc");
	if (EncoderAvailable("ffmpeg_hevc_nvenc"))
		obs_property_list_add_string(p, obs_module_text("NVENC.HEVC"),
					     "nvenc_hevc");
	if (EncoderAvailable("h264_texture_amf"))
		obs_property_list_add_string(p, obs_module_text("AMD.H264"),
					     "amd");
	if (EncoderAvailable("av1_texture_amf"))
		obs_property_list_add_string(p, obs_module_text("AMD.AV1"),
					     "amd_av1");
	if (EncoderAvailable("com.apple.videotoolbox.videoencoder.ave.avc"))
		obs_property_list_add_string(p, obs_module_text("Apple.H264"),
					     "apple_h264");
	if (EncoderAvailable("com.apple.videotoolbox.videoencoder.ave.hevc"))
		obs_property_list_add_string(p, obs_module_text("Apple.HEVC"),
					     "apple_hevc");

	const char *enc_id = NULL;
	size_t i = 0;
	while (obs_enum_encoder_types(i++, &enc_id)) {
		if (obs_get_encoder_type(enc_id) != OBS_ENCODER_VIDEO)
			continue;
		const uint32_t caps = obs_get_encoder_caps(enc_id);
		if ((caps & (OBS_ENCODER_CAP_DEPRECATED |
			     OBS_ENCODER_CAP_INTERNAL)) != 0)
			continue;
		const char *name = obs_encoder_get_display_name(enc_id);
		obs_property_list_add_string(p, name, enc_id);
	}
	obs_property_set_modified_callback2(p, encoder_changed, data);

	obs_properties_t *group = obs_properties_create();
	obs_properties_add_group(props, "encoder_group",
				 obs_module_text("Encoder"), OBS_GROUP_NORMAL,
				 group);

	return props;
}

void source_record_filter_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct source_record_filter_context *context = data;
	obs_source_skip_video_filter(context->source);
}

void source_record_filter_filter_remove(void *data, obs_source_t *parent)
{
	UNUSED_PARAMETER(parent);
	struct source_record_filter_context *context = data;
	context->closing = true;
	if (context->fileOutput) {
		obs_output_force_stop(context->fileOutput);
		obs_output_release(context->fileOutput);
		context->fileOutput = NULL;
	}
	if (context->streamOutput) {
		obs_output_force_stop(context->streamOutput);
		obs_output_release(context->streamOutput);
		context->streamOutput = NULL;
	}
	if (context->replayOutput) {
		obs_output_force_stop(context->replayOutput);
		obs_output_release(context->replayOutput);
		context->replayOutput = NULL;
	}
	obs_frontend_remove_event_callback(frontend_event, context);
}

struct obs_source_info* get_source_record_filter_info() 
{
  static struct obs_source_info source_record_filter_info = 
  {
    .id = "source_record_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = source_record_filter_get_name,
    .create = source_record_filter_create,
    .destroy = source_record_filter_destroy,
    .update = source_record_filter_update,
    .load = source_record_filter_update,
    .save = source_record_filter_save,
    .get_defaults = source_record_filter_defaults,
    .video_render = source_record_filter_render,
    .video_tick = source_record_filter_tick,
    .get_properties = source_record_filter_properties,
    .filter_remove = source_record_filter_filter_remove,
  };

  return &source_record_filter_info;
}
