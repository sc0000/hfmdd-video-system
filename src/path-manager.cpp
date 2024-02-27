#include <util/config-file.h>
#include "source-record.h"
#include "message-dialog.hpp"
#include "path-manager.hpp"

QString PathManager::baseDirectory = "V:/sb-terminal-test/";
QString PathManager::outerDirectory = "";
QString PathManager::innerDirectory = "";

QVector<obs_source_t*> PathManager::sources;

static bool scene_enum_callback(obs_scene_t* scene, obs_sceneitem_t* item, void*) 
{
    obs_source_t* source = obs_sceneitem_get_source(item);

    if (source) 
      PathManager::sources.append(source);

    return true; // Returning true continues the enumeration
}

static void filter_enum_callback(obs_source_t* source, obs_source_t* filter, void*)
{
  if (!source || !filter) return;

  obs_source_filter_remove(source, filter);
}

void PathManager::updateFilterSettings(const char* path)
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

void PathManager::resetFilterSettings()
{
  if (baseDirectory == "" || outerDirectory == "" || innerDirectory == "") {
    OkDialog::instance("ERROR: Unspecified directory");
    return;
  }

  obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentScene) return;
  
  obs_scene_t* scene = obs_scene_from_source(currentScene);

  if (!scene) return;

  void* unused = nullptr; 
  
  obs_source_info* sourceRecordFilterInfo = get_source_record_filter_info();
  const char* id = sourceRecordFilterInfo->id;
  QString sourceRecordFilterName = sourceRecordFilterInfo->get_name(unused);

  // pass path as input param?
  QString path = baseDirectory + outerDirectory + innerDirectory;
  QByteArray pathByteArray = path.toLocal8Bit();
  const char* c_path = pathByteArray.data();

  config_t* currentProfile = obs_frontend_get_profile_config();
  QString filenameFormatting = 
    config_get_string(currentProfile, "Output", "FilenameFormatting");

  sources.clear();
  obs_scene_enum_items(scene, &scene_enum_callback, NULL);
  
  for (obs_source_t* source : sources)
  {
    if (!source) continue;

    obs_source_enum_filters(source, filter_enum_callback, nullptr);

    obs_data_t* settings = obs_data_create();
    sourceRecordFilterInfo->get_defaults(settings);

    QString sourceName = obs_source_get_name(source);
    QString newFormatting = filenameFormatting + "-" + sourceName;

    QString filterName = sourceRecordFilterName + "-" + sourceName;

    obs_source_t* filter = obs_source_create(id, filterName.toUtf8().constData(), settings, NULL);
    obs_source_filter_add(source, filter);
    
    obs_data_set_int(settings, "record_mode", OUTPUT_MODE_RECORDING);
    obs_data_set_string(settings, "path", c_path);
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "rec_format", "mkv");
    obs_data_set_string(settings, "encoder", "nvenc_hevc");
    obs_data_set_string(settings, "preset2", "p7");
     
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