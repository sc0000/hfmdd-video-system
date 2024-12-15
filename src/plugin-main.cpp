/* Entry point
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 * 
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QWidget>
#include <QFontDatabase>
#include "simple-recording.h"
#include "source-record.h"
#include <plugin-support.h>
#include <util/config-file.h>
#include <util/platform.h>
#include "login.hpp"
#include "login-dialog.hpp"
#include "widgets.hpp"
#include "settings.hpp"
#include "mail-handler.hpp"
#include "ptz.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Sebastian Cyliax <sebastiancyliax@gmx.net>");
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return false;

  BookingHandler::initCurrentBooking();
  
  ptz_load_devices();
	ptz_load_action_source();
	ptz_load_settings();

  Widgets::load();
  FullScreenDialog::loadAnims();

  // mainWindow->setStyleSheet("QWidget { font: DaxOT-Medium; }");

  obs_register_source(get_source_record_filter_info());
  
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);

  config_t* globalConfig = obs_frontend_get_global_config();

  if (!globalConfig) return false;

  config_set_string(globalConfig, "General", "CurrentTheme3", "Mondrian");
  config_set_string(globalConfig, "BasicWindow", "DockState", 
    "AAAA/wAAAAD9AAAAAgAAAAEAAADQAAAD4/wCAAAABPsAAAAKAEwAbwBnAGkAbgEAAAAWAAAD4wAAAaYAB///+wAAABQAcwBjAGUAbgBlAHMARABvAGMAawAAAAAWAAAAjAAAAG8A////+wAAABYAcwBvAHUAcgBjAGUAcwBEAG8AYwBrAAAAABYAAADGAAAAbwD////7AAAAFgBQAFQAWgBDAG8AbgB0AHIAbwBsAHMAAAAAFgAAA+MAAAIXAAf//wAAAAMAAAeAAAAA0PwBAAAAA/wAAAAAAAACSgAAAAAA////+v////8CAAAAAvsAAAASAG0AaQB4AGUAcgBEAG8AYwBrAAAAAAD/////AAAAcQD////7AAAAHgB0AHIAYQBuAHMAaQB0AGkAbwBuAHMARABvAGMAawAAAAKiAAAAegAAAGsA////+wAAABgAYwBvAG4AdAByAG8AbABzAEQAbwBjAGsAAAAAAAAAB4AAAAE2AP////sAAAASAHMAdABhAHQAcwBEAG8AYwBrAgAAAcD///0iAAACuQAAAN4AAAasAAAD4wAAAAQAAAAEAAAACAAAAAj8AAAAAA==");
  config_set_string(globalConfig, "BasicWindow", "DocksLocked", "true");

  PTZSettings::loadSettings();
  MailHandler::loadCredentials();

	return true;
}

void obs_module_unload(void)
{
  ptz_unload_devices();
	obs_log(LOG_INFO, "plugin unloaded");
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return obs_module_text("Pan, Tilt & Zoom control plugin");
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return obs_module_text("PTZ Controls");
}

struct source_active_cb_data {
	obs_source_t *source;
	bool active;
};

static void source_active_cb(obs_source_t *parent, obs_source_t *child,
			     void *data)
{
	UNUSED_PARAMETER(parent);
	struct source_active_cb_data *cb_data = (source_active_cb_data *)data;
	if (child == cb_data->source)
		cb_data->active = true;
}

bool ptz_scene_is_source_active(obs_source_t *scene, obs_source_t *source)
{
	struct source_active_cb_data cb_data;
  cb_data.source = source;
  cb_data.active = false;

	if (scene == source)
		return true;
	obs_source_enum_active_sources(scene, source_active_cb, &cb_data);
	return cb_data.active;
}
