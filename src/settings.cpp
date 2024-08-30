#include <QPlainTextEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QPushButton>
#include <QFontDatabase>
#include <QFont>
#include <QDialogButtonBox>
#include <QResizeEvent>
#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <QUrl>
#include <QDesktopServices>
#include <QStringList>
#include <QJsonDocument>

#include <string>

#include <obs.hpp>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <obs-properties.h>
#include "imported/qjoysticks/QJoysticks.h"

#include "source-record.h"
#include "mail-handler.hpp"
#include "storage-handler.hpp"
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "widgets.hpp"
#include "ptz.h"
#include "ptz-device.hpp"
#include "ptz-controls.hpp"
#include "settings.hpp"
#include "ui_settings.h"

/* ----------------------------------------------------------------- */

static PTZSettings *ptzSettingsWindow = nullptr;

QString PTZSettings::filenameFormatting = "";
QString PTZSettings::recFormat = "";
QString PTZSettings::qualityPreset = "";
QVector<obs_source_t*> PTZSettings::sources;

/* ----------------------------------------------------------------- */

QString SourceNameDelegate::displayText(const QVariant &value,
					const QLocale &locale) const
{
	auto string = QStyledItemDelegate::displayText(value, locale);
	auto ptz = ptzDeviceList.getDeviceByName(string);
	if (ptz)
		return ptz->description() + " - " + string;
	return string;
}

obs_properties_t *PTZSettings::getProperties(void)
{
	auto applycb = [](obs_properties_t *, obs_property_t *, void *data_) {
		auto s = static_cast<PTZSettings *>(data_);
		PTZDevice *ptz = ptzDeviceList.getDevice(
			s->ui->deviceList->currentIndex());
		if (ptz)
			ptz->set_config(s->propertiesView->GetSettings());
		return true;
	};
	auto cb = [](obs_properties_t *, obs_property_t *, void *data_) {
		auto data = static_cast<obs_data_t *>(data_);
		blog(LOG_INFO, "%s", obs_data_get_string(data, "debug_info"));
		return true;
	};

	PTZDevice *ptz =
		ptzDeviceList.getDevice(ui->deviceList->currentIndex());
	if (!ptz)
		return obs_properties_create();

	auto props = ptz->get_obs_properties();
	auto p = obs_properties_get(props, "interface");
	if (p) {
		auto iface = obs_property_group_content(p);
		if (iface)
			obs_properties_add_button2(iface, "iface_apply",
						   "Apply", applycb, this);
	}
	if (ptz_debug_level <= LOG_INFO) {
		auto debug = obs_properties_create();
		obs_properties_add_text(debug, "debug_info", NULL,
					OBS_TEXT_INFO);
		obs_properties_add_button2(debug, "dbgdump", "Write to OBS log",
					   cb, settings);
		obs_properties_add_group(props, "debug", "Debug",
					 OBS_GROUP_NORMAL, debug);
	}
	return props;
}

void PTZSettings::updateProperties(OBSData old_settings, OBSData new_settings)
{
	PTZDevice *ptz =
		ptzDeviceList.getDevice(ui->deviceList->currentIndex());
	if (ptz)
		ptz->set_settings(new_settings);
	Q_UNUSED(old_settings);
}

PTZSettings::PTZSettings(QWidget* parent) 
  : AnimatedDialog(parent), 
    ui(new Ui_PTZSettings),
    sourcesDockIsOpen(false)
{
	settings = obs_data_create();
	obs_data_release(settings);

	ui->setupUi(this);

  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  //setModal(true);

  ui->tabWidget->move(QPoint(0, 41));
  ui->tabWidget->setFixedWidth(width());
  ui->tabWidget->setFixedHeight(height() - 41);

  Handlebar* handlebar = new Handlebar(this);

	ui->livemoveCheckBox->setChecked(
		Widgets::ptzControls->liveMovesDisabled());
	ui->enableDebugLogCheckBox->setChecked(ptz_debug_level <= LOG_INFO);

	auto snd = new SourceNameDelegate(this);
	ui->deviceList->setModel(&ptzDeviceList);
	ui->deviceList->setItemDelegateForColumn(0, snd);

	QItemSelectionModel *selectionModel = ui->deviceList->selectionModel();
	connect(selectionModel,
		SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
		SLOT(currentChanged(QModelIndex, QModelIndex)));

	auto reload_cb = [](void *obj) {
		return static_cast<PTZSettings *>(obj)->getProperties();
	};
	auto update_cb = [](void *obj, obs_data_t *oldset, obs_data_t *newset) {
		static_cast<PTZSettings *>(obj)->updateProperties(oldset,
								  newset);
	};
	propertiesView =
		new OBSPropertiesView(settings, this, reload_cb, update_cb);
	propertiesView->setSizePolicy(QSizePolicy::Expanding,
				      QSizePolicy::Expanding);
	ui->propertiesLayout->insertWidget(0, propertiesView, 0);

	joystickSetup();

	// ui->versionLabel->setText(description_text);

  //----------------------------------------------------
  for (const QString& preset : qualityPresets) 
    ui->qualityComboBox->addItem(preset);

  for (const QString& recFormat : recFormats)
    ui->recFormatComboBox->addItem(recFormat);

  getAdditionalProperties();
  getCredentials();
  //----------------------------------------------------
}

PTZSettings::~PTZSettings()
{
	delete ui;
}

#ifdef ENABLE_JOYSTICK
void PTZSettings::joystickSetup()
{
	auto joysticks = QJoysticks::getInstance();
	auto controls = Widgets::ptzControls;
	ui->joystickNamesListView->setModel(&m_joystickNamesModel);
	ui->joystickGroupBox->setChecked(controls->joystickEnabled());
	ui->joystickSpeedSlider->setDoubleConstraints(
		0.25, 1.75, 0.05, controls->joystickSpeed());
	ui->joystickDeadzoneSlider->setDoubleConstraints(
		0.01, 0.15, 0.01, controls->joystickDeadzone());

	connect(joysticks, SIGNAL(countChanged()), this,
		SLOT(joystickUpdate()));
	connect(joysticks, SIGNAL(axisEvent(const QJoystickAxisEvent)), this,
		SLOT(joystickAxisEvent(const QJoystickAxisEvent)));

	auto selectionModel = ui->joystickNamesListView->selectionModel();
	connect(selectionModel,
		SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
		SLOT(joystickCurrentChanged(QModelIndex, QModelIndex)));
	joystickUpdate();
}

void PTZSettings::on_joystickSpeedSlider_doubleValChanged(double val)
{
	Widgets::ptzControls->setJoystickSpeed(val);
}

void PTZSettings::on_joystickDeadzoneSlider_doubleValChanged(double val)
{
	Widgets::ptzControls->setJoystickDeadzone(val);
}

void PTZSettings::on_joystickGroupBox_toggled(bool checked)
{
	Widgets::ptzControls->setJoystickEnabled(checked);
	ui->joystickNamesListView->setEnabled(checked);
}

void PTZSettings::joystickUpdate()
{
	auto joysticks = QJoysticks::getInstance();
	auto controls = Widgets::ptzControls;
	m_joystickNamesModel.setStringList(joysticks->deviceNames());
	auto idx = m_joystickNamesModel.index(controls->joystickId(), 0);
	if (idx.isValid())
		ui->joystickNamesListView->setCurrentIndex(idx);
}

void PTZSettings::joystickCurrentChanged(QModelIndex current,
					 QModelIndex previous)
{
	Q_UNUSED(previous);
	Widgets::ptzControls->setJoystickId(current.row());
}
#else
void PTZSettings::joystickSetup()
{
	ui->joystickGroupBox->setVisible(false);
}
#endif /* ENABLE_JOYSTICK */

void PTZSettings::set_selected(uint32_t device_id)
{
	ui->deviceList->setCurrentIndex(
		ptzDeviceList.indexFromDeviceId(device_id));
}

void PTZSettings::on_addPTZ_clicked()
{
	QMenu addPTZContext;
#if defined(ENABLE_SERIALPORT)
	QAction *addViscaSerial = addPTZContext.addAction("VISCA (Serial)");
#endif
	QAction *addViscaUDP = addPTZContext.addAction("VISCA (UDP)");
	QAction *addViscaTCP = addPTZContext.addAction("VISCA (TCP)");
#if defined(ENABLE_SERIALPORT)
	QAction *addPelcoD = addPTZContext.addAction("Pelco D");
	QAction *addPelcoP = addPTZContext.addAction("Pelco P");
#endif
#if defined(ENABLE_ONVIF) // ONVIF disabled until code is reworked
	QAction *addOnvif = addPTZContext.addAction("ONVIF (experimental)");
#endif
	QAction *action = addPTZContext.exec(QCursor::pos());

#if defined(ENABLE_SERIALPORT)
	if (action == addViscaSerial) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "visca");
		ptzDeviceList.make_device(cfg);
	}
#endif
	if (action == addViscaUDP) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "visca-over-ip");
		obs_data_set_int(cfg, "port", 52381);
		ptzDeviceList.make_device(cfg);
	}
	if (action == addViscaTCP) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "visca-over-tcp");
		obs_data_set_int(cfg, "port", 5678);
		ptzDeviceList.make_device(cfg);
	}
#if defined(ENABLE_SERIALPORT)
	if (action == addPelcoD) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "pelco");
		obs_data_set_bool(cfg, "use_pelco_d", true);
		ptzDeviceList.make_device(cfg);
	}
	if (action == addPelcoP) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "pelco");
		obs_data_set_bool(cfg, "use_pelco_d", false);
		ptzDeviceList.make_device(cfg);
	}
#endif
#if defined(ENABLE_ONVIF)
	if (action == addOnvif) {
		OBSData cfg = obs_data_create();
		obs_data_release(cfg);
		obs_data_set_string(cfg, "type", "onvif");
		ptzDeviceList.make_device(cfg);
	}
#endif
}

static bool scene_enum_callback(obs_scene_t* scene, obs_sceneitem_t* item, void*) 
{
    obs_source_t* source = obs_sceneitem_get_source(item);

    if (source) 
      PTZSettings::sources.append(source);

    return true; // Returning true continues the enumeration
}

static void filter_enum_callback(obs_source_t* source, obs_source_t* filter, void*)
{
  if (!source || !filter) return;

  obs_source_filter_remove(source, filter);
}

void PTZSettings::updateFilterSettings(const char* path)
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

void PTZSettings::resetFilterSettings()
{ 
  if (StorageHandler::baseDirectory == "") {
    Widgets::okDialog->display("ERROR: Unspecified base directory");
    return;
  }

  StorageHandler::setOuterDirectory();


  if (StorageHandler::outerDirectory == "") {
    Widgets::okDialog->display("ERROR: Unspecified outer directory");
    return;
  }

  StorageHandler::setInnerDirectory();

  if (StorageHandler::innerDirectory == "") {
    Widgets::okDialog->display("ERROR: Unspecified inner directory");
    return;
  }

  StorageHandler::setTempPath();

  config_t* currentProfile = obs_frontend_get_profile_config();

  obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentProfile || !currentScene) return;
  
  obs_scene_t* scene = obs_scene_from_source(currentScene);

  if (!scene) return;

  void* unused = nullptr; 
  
  obs_source_info* sourceRecordFilterInfo = get_source_record_filter_info();
  const char* id = sourceRecordFilterInfo->id;
  QString sourceRecordFilterName = sourceRecordFilterInfo->get_name(unused);

  const QString path = StorageHandler::getPath();

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
    QString newFormatting = filenameFormatting + " " + sourceName;

    QString filterName = sourceRecordFilterName + "-" + sourceName;

    obs_source_t* filter = obs_source_create(id, filterName.toUtf8().constData(), settings, NULL);
    obs_source_filter_add(source, filter);
    
    obs_data_set_int(settings, "record_mode", OUTPUT_MODE_RECORDING);
    obs_data_set_string(settings, "path", c_path);
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "rec_format", recFormat.toUtf8().constData());
    obs_data_set_string(settings, "encoder", "nvenc_hevc");
    obs_data_set_string(settings, "preset2", qualityPreset.toUtf8().constData());
     
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

// TODO: Use custom JSON interface
void PTZSettings::saveSettings()
{
  char* file = obs_module_config_path("config2.json");

	if (!file) return;

  OBSData savedata = obs_data_create();
	obs_data_release(savedata);

  obs_data_set_string(savedata, "base_directory", StorageHandler::baseDirectory.toUtf8().constData());
  obs_data_set_string(savedata, "filename_formatting", filenameFormatting.toUtf8().constData());
  obs_data_set_string(savedata, "rec_format", recFormat.toUtf8().constData());
  obs_data_set_string(savedata, "quality_preset", qualityPreset.toUtf8().constData());

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

void PTZSettings::loadSettings()
{
  char *file = obs_module_config_path("config2.json");

	if (!file) return;

	OBSData loaddata = obs_data_create_from_json_file_safe(file, "bak");

  bfree(file);

	if (!loaddata) return;
  
	obs_data_release(loaddata);

  StorageHandler::baseDirectory = obs_data_get_string(loaddata, "base_directory");
  filenameFormatting = obs_data_get_string(loaddata, "filename_formatting");
  recFormat = obs_data_get_string(loaddata, "rec_format");
  qualityPreset = obs_data_get_string(loaddata, "quality_preset");

  JsonParser::updatePaths();
  StorageHandler::setTempPath();
}

void PTZSettings::on_removePTZ_clicked()
{
	PTZDevice *ptz =
		ptzDeviceList.getDevice(ui->deviceList->currentIndex());
	if (!ptz)
		return;
	delete ptz;
}

void PTZSettings::on_livemoveCheckBox_stateChanged(int state)
{
	Widgets::ptzControls->setDisableLiveMoves(state != Qt::Unchecked);
}

void PTZSettings::on_enableDebugLogCheckBox_stateChanged(int state)
{
	ptz_debug_level = (state == Qt::Unchecked) ? LOG_DEBUG : LOG_INFO;
}

void PTZSettings::on_saveButton0_pressed()
{
  updateAdditionalProperties();
}

void PTZSettings::on_saveButton1_pressed()
{
  updateCredentials();
}

void PTZSettings::on_sourcesButton_pressed()
{
  (sourcesDockIsOpen = !sourcesDockIsOpen);

  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return;

  QDockWidget* sourcesDock = mainWindow->findChild<QDockWidget*>("sourcesDock");

  if (!sourcesDock) return;

  if (sourcesDockIsOpen) 
    sourcesDock->show();
    
  else 
    sourcesDock->hide();
}

void PTZSettings::on_sourcesButton_released()
{
  if (sourcesDockIsOpen)
    ui->sourcesButton->setText("Close Sources Dock");

  else
    ui->sourcesButton->setText("Edit Sources");
}

void PTZSettings::currentChanged(const QModelIndex &current,
				 const QModelIndex &previous)
{
	auto ptz = ptzDeviceList.getDevice(previous);

	if (ptz) ptz->disconnect(this);

	obs_data_clear(settings);

	ptz = ptzDeviceList.getDevice(current);

	if (ptz) {
		obs_data_apply(settings, ptz->get_settings());

		/* For debug, display all data in JSON format */
		if (ptz_debug_level <= LOG_INFO) {
			auto rawjson = obs_data_get_json(settings);
			/* Use QJsonDocument for nice formatting */
			auto json = QJsonDocument::fromJson(rawjson).toJson();
			obs_data_set_string(settings, "debug_info",
					    json.constData());
		}

		/* The settings dialog doesn't touch presets, so remove them */
		obs_data_erase(settings, "presets");

		ptz->connect(ptz, SIGNAL(settingsChanged(OBSData)), this,
			     SLOT(settingsChanged(OBSData)));
	}

	propertiesView->ReloadProperties();
}

void PTZSettings::settingsChanged(OBSData changed)
{
	obs_data_apply(settings, changed);
	obs_data_erase(settings, "debug_info");
	auto json =
		QJsonDocument::fromJson(obs_data_get_json(settings)).toJson();
	obs_data_set_string(settings, "debug_info", json.constData());
	propertiesView->RefreshProperties();
}

// TODO: Rename: reload()
void PTZSettings::showDevice(uint32_t device_id)
{
	if (device_id) {
		set_selected(device_id);
		ui->tabWidget->setCurrentWidget(ui->camerasTab);
	} 
  
  else {
		ui->tabWidget->setCurrentWidget(ui->generalTab);
	}

	// show();
	// raise();
  fade();
}

void PTZSettings::getAdditionalProperties()
{
  loadSettings();

  ui->baseDirectoryLineEdit->setText(StorageHandler::baseDirectory);
  ui->filenameFormattingLineEdit->setText(filenameFormatting);
  ui->recFormatComboBox->setCurrentText(recFormat);

  for (const QString& preset : qualityPresets) {
    if (preset.contains(qualityPreset.toUpper())) {
      ui->qualityComboBox->setCurrentText(preset);
      break;
    }
  }  
}

void PTZSettings::getCredentials()
{
  MailHandler::loadCredentials();

  ui->adminEmailLineEdit->setText(MailHandler::adminEmail);
  ui->adminPasswordLineEdit->setText(MailHandler::adminPassword);

  ui->nasIpLineEdit->setText(MailHandler::nasIP);
  ui->nasPortLineEdit->setText(MailHandler::nasPort);
  ui->nasUserNameLineEdit->setText(MailHandler::nasUser);
  ui->nasPasswordLineEdit->setText(MailHandler::nasPassword);
  ui->mailHostLineEdit->setText(MailHandler::mailHost);
  ui->mailUserNameLineEdit->setText(MailHandler::mailUser);
  ui->mailPasswordLineEdit->setText(MailHandler::mailPassword);
  ui->mailSenderAddressLineEdit->setText(MailHandler::mailSenderAddress);
}

void PTZSettings::updateAdditionalProperties()
{
	StorageHandler::baseDirectory = ui->baseDirectoryLineEdit->text();
  StorageHandler::baseDirectory.replace("\\", "/");

  if (StorageHandler::baseDirectory.last(1) != "/")
    StorageHandler::baseDirectory.append("/");

  filenameFormatting = ui->filenameFormattingLineEdit->text();
  qualityPreset = ui->qualityComboBox->currentText().first(2).toLower();
  recFormat = ui->recFormatComboBox->currentText();

  resetFilterSettings();
  saveSettings();
}

void PTZSettings::updateCredentials()
{
  MailHandler::adminEmail = ui->adminEmailLineEdit->text();
  MailHandler::adminPassword = ui->adminPasswordLineEdit->text();

  MailHandler::nasIP = ui->nasIpLineEdit->text();
  MailHandler::nasPort = ui->nasPortLineEdit->text();
  MailHandler::nasUser = ui->nasUserNameLineEdit->text();
  MailHandler::nasPassword = ui->nasPasswordLineEdit->text();
  MailHandler::mailHost = ui->mailHostLineEdit->text();
  MailHandler::mailUser = ui->mailUserNameLineEdit->text();
  MailHandler::mailPassword = ui->mailPasswordLineEdit->text();
  MailHandler::mailSenderAddress = ui->mailSenderAddressLineEdit->text();

  MailHandler::saveCredentials();
}

/* ----------------------------------------------------------------- */

static void obs_event(enum obs_frontend_event event, void *)
{
	if (event == OBS_FRONTEND_EVENT_EXIT)
		delete ptzSettingsWindow;
}

void ptz_settings_show(uint32_t device_id)
{
	obs_frontend_push_ui_translation(obs_module_get_string);

  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return;

	if (!ptzSettingsWindow)
		ptzSettingsWindow = new PTZSettings(mainWindow);
	ptzSettingsWindow->showDevice(device_id);

	obs_frontend_pop_ui_translation();
}

extern "C" void ptz_load_settings()
{
	QAction *action = (QAction *)obs_frontend_add_tools_menu_qaction(
		obs_module_text("PTZ Devices"));

	auto cb = []() { ptz_settings_show(0); };

	obs_frontend_add_event_callback(obs_event, nullptr);

	action->connect(action, &QAction::triggered, cb);
}
