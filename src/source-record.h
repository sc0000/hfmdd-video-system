#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/platform.h>
#include <util/threading.h>

#include "version.h"
#include "util/dstr.h"

#define OUTPUT_MODE_NONE 0
#define OUTPUT_MODE_ALWAYS 1
#define OUTPUT_MODE_STREAMING 2
#define OUTPUT_MODE_RECORDING 3
#define OUTPUT_MODE_STREAMING_OR_RECORDING 4
#define OUTPUT_MODE_VIRTUAL_CAMERA 5

#ifdef __cplusplus
extern "C" {
#endif

struct source_record_filter_context {
	obs_source_t* source;
	uint8_t* video_data;
	uint32_t video_linesize;
	video_t* video_output;
	audio_t* audio_output;
	bool output_active;
	uint32_t width;
	uint32_t height;
	uint64_t last_frame_time_ns;
	obs_view_t* view;
	bool starting_file_output;
	bool starting_stream_output;
	bool starting_replay_output;
	bool restart;
	obs_output_t* fileOutput;
	obs_output_t* streamOutput;
	obs_output_t* replayOutput;
	obs_encoder_t* encoder;
	obs_encoder_t* aacTrack;
	obs_service_t* service;
	bool record;
	bool stream;
	bool replayBuffer;
	obs_hotkey_pair_id enableHotkey;
	int audio_track;
	obs_weak_source_t *audio_source;
	bool closing;
	long long replay_buffer_duration;
	struct vec4 backgroundColor;
};

struct video_frame {
	uint8_t *data[MAX_AV_PLANES];
	uint32_t linesize[MAX_AV_PLANES];
};

extern const char *source_record_filter_get_name(void *unused);
bool EncoderAvailable(const char *encoder);
void calc_min_ts(obs_source_t *parent, obs_source_t *child, void *param);
void mix_audio(obs_source_t *parent, obs_source_t *child, void *param);
bool audio_input_callback(void *param, uint64_t start_ts_in, uint64_t end_ts_in, uint64_t *out_ts,
  uint32_t mixers, struct audio_output_data *mixes);
void *start_file_output_thread(void *data);
void *start_stream_output_thread(void *data);
void *force_stop_output_thread(void *data);
void *start_replay_thread(void *data);
void ensure_directory(char *path);
void start_file_output(struct source_record_filter_context *filter, obs_data_t *settings);
void start_stream_output(struct source_record_filter_context *filter, obs_data_t *settings);
void start_replay_output(struct source_record_filter_context *filter, obs_data_t *settings);
const char *get_encoder_id(obs_data_t *settings);
void source_record_filter_update(void *data, obs_data_t *settings);
void source_record_filter_save(void *data, obs_data_t *settings);
void source_record_filter_defaults(obs_data_t *settings);
void frontend_event(enum obs_frontend_event event, void *data);
void *source_record_filter_create(obs_data_t *settings, obs_source_t *source);
void source_record_filter_destroy(void *data);
bool source_record_enable_hotkey(void *data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed);
bool source_record_disable_hotkey(void *data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed);
void source_record_filter_tick(void *data, float seconds);
bool encoder_changed(void *data, obs_properties_t *props, obs_property_t *property, obs_data_t *settings);
bool list_add_audio_sources(void *data, obs_source_t *source);
obs_properties_t *source_record_filter_properties(void *data);
void source_record_filter_render(void *data, gs_effect_t *effect);
void source_record_filter_filter_remove(void *data, obs_source_t *parent);

struct obs_source_info* get_source_record_filter_info();

#ifdef __cplusplus
}
#endif