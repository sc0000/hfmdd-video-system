#include <QDir>

#include <obs.hpp>
#include <util/config-file.h>
#include "source-record.h"
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "settings-manager.hpp"

QString SettingsManager::baseDirectory = "V:/sb-terminal-test/";
QString SettingsManager::outerDirectory = "";
QString SettingsManager::innerDirectory = "";

QString SettingsManager::filenameFormatting = "";
QString SettingsManager::recFormat = "";
QString SettingsManager::qualityPreset = "";

QVector<obs_source_t*> SettingsManager::sources;

static bool scene_enum_callback(obs_scene_t* scene, obs_sceneitem_t* item, void*) 
{
    obs_source_t* source = obs_sceneitem_get_source(item);

    if (source) 
      SettingsManager::sources.append(source);

    return true; // Returning true continues the enumeration
}

static void filter_enum_callback(obs_source_t* source, obs_source_t* filter, void*)
{
  if (!source || !filter) return;

  obs_source_filter_remove(source, filter);
}

void SettingsManager::updateFilterSettings(const char* path)
{
   obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentScene) return;

  obs_scene_t* scene = obs_scene_from_source(currentScene);
  
  if (!scene) return;

  config_t* currentProfile = obs_frontend_get_profile_config();
  QString filenameFormatting = 
    config_get_string(currentProfile, "Output", "FilenameFormatting");

  sources.clear();
  obs_scene_enum_items(scene, &scene_enum_callback, NULL);
   
  for (obs_source_t* source : sources)
  {
    if (!source) continue;

    QString sourceName = obs_source_get_name(source);
    QString newFormatting = filenameFormatting + "-" + sourceName;
    obs_source_t* f = obs_source_get_filter_by_name(source, "Source Record");
    obs_data_t* settings = obs_source_get_settings(f);

    // file_formatting could be set manually...
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "path", path);

    obs_data_release(settings);
  }
}

void SettingsManager::resetFilterSettings()
{
  if (baseDirectory == "" || outerDirectory == "" || innerDirectory == "") {
    OkDialog::instance("ERROR: Unspecified directory");
    return;
  }

  setTempPath();

  config_t* currentProfile = obs_frontend_get_profile_config();

  obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentProfile || !currentScene) return;
  
  obs_scene_t* scene = obs_scene_from_source(currentScene);

  if (!scene) return;

  void* unused = nullptr; 
  
  obs_source_info* sourceRecordFilterInfo = get_source_record_filter_info();
  const char* id = sourceRecordFilterInfo->id;
  QString sourceRecordFilterName = sourceRecordFilterInfo->get_name(unused);

  QString path = baseDirectory + outerDirectory + innerDirectory;
  QByteArray pathByteArray = path.toLocal8Bit();
  const char* c_path = pathByteArray.data();

  // QString filenameFormatting = 
  //   config_get_string(currentProfile, "Output", "FilenameFormatting");

  sources.clear();
  obs_scene_enum_items(scene, &scene_enum_callback, NULL);
  
  for (obs_source_t* source : sources)
  {
    if (!source) continue;

    obs_source_enum_filters(source, filter_enum_callback, nullptr);

    obs_data_t* settings = obs_data_create();
    sourceRecordFilterInfo->get_defaults(settings);

    QString sourceName = obs_source_get_name(source);
    QString newFormatting = SettingsManager::filenameFormatting + "-" + sourceName;

    QString filterName = sourceRecordFilterName + "-" + sourceName;

    obs_source_t* filter = obs_source_create(id, filterName.toUtf8().constData(), settings, NULL);
    obs_source_filter_add(source, filter);
    
    obs_data_set_int(settings, "record_mode", OUTPUT_MODE_RECORDING);
    obs_data_set_string(settings, "path", c_path);
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "rec_format", SettingsManager::recFormat.toUtf8().constData());
    obs_data_set_string(settings, "encoder", "nvenc_hevc");
    obs_data_set_string(settings, "preset2", SettingsManager::qualityPreset.toUtf8().constData());
     
    // ! For finding the available encoder props !
    //
    // const char* encId = get_encoder_id(settings);
    // obs_properties_t* encProps = obs_get_encoder_properties(encId);
    // obs_property_t* p = obs_properties_first(encProps);
    
    // QString propsInfo = "";

    // while (p)
    // {
    //   QString name = obs_property_name(p);
    //   propsInfo.append(name + "\n");
    //   obs_property_next(&p);
    // }
    
    // QMessageBox::information(this, "available encoder props", propsInfo);
  
    obs_data_release(settings);
  }
}

void SettingsManager::save()
{
  char* file = obs_module_config_path("config2.json");

	if (!file) return;

  OBSData savedata = obs_data_create();
	obs_data_release(savedata);

  obs_data_set_string(savedata, "base_directory", SettingsManager::baseDirectory.toUtf8().constData());
  obs_data_set_string(savedata, "filename_formatting", SettingsManager::filenameFormatting.toUtf8().constData());
  obs_data_set_string(savedata, "rec_format", SettingsManager::recFormat.toUtf8().constData());
  obs_data_set_string(savedata, "quality_preset", SettingsManager::qualityPreset.toUtf8().constData());

  if (!obs_data_save_json_safe(savedata, file, "tmp", "bak")) {
		char *path = obs_module_config_path("");

		if (path) {
			os_mkdirs(path);
			bfree(path);
		}

		obs_data_save_json_safe(savedata, file, "tmp", "bak");
	}

	bfree(file);
}

void SettingsManager::load()
{
  char *file = obs_module_config_path("config2.json");

	if (!file) return;

	OBSData loaddata = obs_data_create_from_json_file_safe(file, "bak");

  bfree(file);

	if (!loaddata) return;
  
	obs_data_release(loaddata);

  SettingsManager::baseDirectory = obs_data_get_string(loaddata, "base_directory");
  SettingsManager::filenameFormatting = obs_data_get_string(loaddata, "filename_formatting");
  SettingsManager::recFormat = obs_data_get_string(loaddata, "rec_format");
  SettingsManager::qualityPreset = obs_data_get_string(loaddata, "quality_preset");

  JsonParser::bookingsPath = baseDirectory + "bookings.json";
  JsonParser::presetsPath = baseDirectory + "presets.json";

  setTempPath();
}

void SettingsManager::setTempPath()
{
  config_t* currentProfile = obs_frontend_get_profile_config();

  if (!currentProfile) return;

  QString sceneFilePath = SettingsManager::baseDirectory + "temp/";
  QDir tempDir = QDir(sceneFilePath);

  if (!tempDir.exists())
    tempDir.mkpath(sceneFilePath);

  const char* c_sceneFilePath = sceneFilePath.toUtf8().constData();

  config_set_string(currentProfile, "SimpleOutput", "FilePath", c_sceneFilePath);
}

void SettingsManager::deleteTempFiles()
{
  QDir tempDir(SettingsManager::baseDirectory + "temp/");
  tempDir.setNameFilters(QStringList() << "*.*");
  tempDir.setFilter(QDir::Files);

  for (const QString& file : tempDir.entryList())
    tempDir.remove(file);
}