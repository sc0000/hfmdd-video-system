/* Pan Tilt Zoom camera controls
 *
 * Copyright 2020 Grant Likely (2020) <grant.likely@secretlab.ca>
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include <QTimer>
#include <QList>
#include <obs.hpp>
#include <QDockWidget>

#include "text-handler.hpp"
#include "time-observer.hpp"
#include "booking-handler.hpp"
#include "imported/qjoysticks/QJoysticks.h"
#include "touch-control.hpp"
#include "ptz-device.hpp"
#include "ui_ptz-controls.h"

class PTZControls : public QDockWidget, public Translatable {
	Q_OBJECT

private:
	static void OBSFrontendEventWrapper(enum obs_frontend_event event,
					    void *ptr);
	void OBSFrontendEvent(enum obs_frontend_event event);

	std::unique_ptr<Ui::PTZControls> ui;
	TouchControl *pantilt_widget;

  PTZPresetListModel m_presetsModel;
  PTZPresetListModel m_userPresetsModel;

  TimeObserver* m_timeObserver;

  bool showOverview = true;

  bool hasRecorded = false;
	bool live_moves_disabled = false;

	// Current status
	double pan_speed = 0.0;
	double pan_accel = 0.0;
	double tilt_speed = 0.0;
	double tilt_accel = 0.0;
	double zoom_speed = 0.0;
	double zoom_accel = 0.0;
	double focus_speed = 0.0;
	double focus_accel = 0.0;
	QTimer accel_timer;

	bool pantiltingFlag = false;
	bool zoomingFlag = false;
	bool focusingFlag = false;
  
  QDialog* savePresetDialog;
  QString newPresetName;

	void copyActionsDynamicProperties();
	void SaveConfig();
	void LoadConfig();

	void setZoom(double speed);
	void setFocus(double speed);

	void setCurrent(unsigned int index);
	int presetIndexToId(PTZPresetListModel* model, QModelIndex index);

  Booking* booking;

// TODO: Tidy up!
public:
  void setNewPresetName(const QString& text);
  void savePreset();
  
private:
	void presetSet(int id);
  void presetSetAll(int preset_id);

	void presetRecall(int id);
  void presetRecallAll(int preset_id);
  int presetNameToId(QAbstractListModel* model, const QString& name);
  
  // TODO: Remove!
  void onNewSelectedItem(obs_scene_t* scene, obs_sceneitem_t* item);

	void setAutofocusEnabled(bool autofocus_on);

  void savePresets();

private:
	PTZDevice *currCamera();
  QList<PTZDevice*> allCameras();

	QList<obs_hotkey_id> hotkeys;
	QMap<obs_hotkey_id, int> preset_hotkey_map;

private slots:
	void setPanTilt(double pan, double tilt, double pan_accel = 0,
			double tilt_accel = 0);
	void keypressPanTilt(double pan, double tilt);
	void on_panTiltButton_up_pressed();
	void on_panTiltButton_up_released();
	void on_panTiltButton_upleft_pressed();
	void on_panTiltButton_upleft_released();
	void on_panTiltButton_upright_pressed();
	void on_panTiltButton_upright_released();
	void on_panTiltButton_left_pressed();
	void on_panTiltButton_left_released();
	void on_panTiltButton_right_pressed();
	void on_panTiltButton_right_released();
	void on_panTiltButton_down_pressed();
	void on_panTiltButton_down_released();
	void on_panTiltButton_downleft_pressed();
	void on_panTiltButton_downleft_released();
	void on_panTiltButton_downright_pressed();
	void on_panTiltButton_downright_released();
	void on_panTiltButton_home_released();

	void on_zoomButton_tele_pressed();
	void on_zoomButton_tele_released();
	void on_zoomButton_wide_pressed();
	void on_zoomButton_wide_released();

	void on_focusButton_auto_clicked(bool checked);
	void on_focusButton_near_pressed();
	void on_focusButton_near_released();
	void on_focusButton_far_pressed();
	void on_focusButton_far_released();
	void on_focusButton_onetouch_clicked();

  void on_previousCamButton_pressed();
  void on_nextCamButton_pressed();
  void on_overviewButton_clicked();

  void on_infoButton_pressed();
  void on_savePresetButton_clicked();
  void on_loadPresetButton_clicked();
  void on_deletePresetButton_clicked();
  
  void on_recordButton_pressed();
  void on_toBookingManagerButton_clicked();
  void on_logoutButton_clicked();

	void on_actionFollowPreview_toggled(bool checked);
	void on_actionFollowProgram_toggled(bool checked);

	void currentChanged(QModelIndex current, QModelIndex previous);
	void settingsChanged(OBSData settings);
	void updateMoveControls();

	// void presetUpdateActions();
	void on_presetListView_activated(QModelIndex index);
	void on_presetListView_customContextMenuRequested(const QPoint &pos);
	void on_cameraList_doubleClicked(const QModelIndex &index);
	void on_cameraList_customContextMenuRequested(const QPoint &pos);
	void on_actionPTZProperties_triggered();
	void on_actionDisableLiveMoves_toggled(bool checked);
	void on_actionTouchControl_toggled(bool checked);
	void on_actionPresetAdd_triggered();
	void on_actionPresetRemove_triggered();
	void on_actionPresetMoveUp_triggered();
	void on_actionPresetMoveDown_triggered();

	void accelTimerHandler();

	/* Joystick support */
protected:
	bool m_joystick_enable = false;
	int m_joystick_id = -1;
	double m_joystick_deadzone = 0.0;
	double m_joystick_speed = 1.0;
#if defined(ENABLE_JOYSTICK)
public:
	void joystickSetup();
	bool joystickEnabled() { return m_joystick_enable; };
	double joystickDeadzone() { return m_joystick_deadzone; };
	double joystickSpeed() { return m_joystick_speed; };
	void setJoystickEnabled(bool enable);
	void setJoystickSpeed(double speed);
	void setJoystickDeadzone(double deadzone);
	int joystickId() { return m_joystick_id; };
	void setJoystickId(int id) { m_joystick_id = id; };
protected slots:
	void joystickAxesChanged(const QJoystickDevice *jd, uint32_t updated);
	void joystickAxisEvent(const QJoystickAxisEvent evt);
	void joystickButtonEvent(const QJoystickButtonEvent evt);
	void joystickPOVEvent(const QJoystickPOVEvent evt);
#else
	void joystickSetup(){};
#endif /* ENABLE_JOYSTICK */

public:
	PTZControls(QWidget* parent = nullptr);
	~PTZControls();
	void setDisableLiveMoves(bool enable);
	bool liveMovesDisabled() { return live_moves_disabled; };
  virtual PTZPresetListModel* presetModel() { return &m_presetsModel; }
  PTZPresetListModel* userPresetModel() { return &m_userPresetsModel; } 

  void reload();

  // TODO: Which of these actually need to be public?

  void setSelectedBooking();
  void setViewportMode();
  void selectCamera();
  void loadUserPresets();
  void connectSignalItemSelect();
  void startRecording();
  void stopRecording(bool manual);
  void logout();
  // void loginTimeOut();

  QString currCameraName;

  virtual void updateTexts() override; 
};
