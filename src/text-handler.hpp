#pragma once

#include <QMap>
#include <QLocale>

enum class ETextId {
  LOGIN_INFO,
  LOGIN_REMINDER,
  MODE_QUICK,
  MODE_BOOKING,
  QUICK_UNTIL,
  QUICK_FROM_NOW,
  QUICK_TO_PTZ,
  QUICK_BACK,
  QUICK_INFO,
  QUICK_PREV_BOOKINGS_NONE,
  QUICK_PREV_BOOKINGS,
  MANAGER_NEW,
  MANAGER_EDIT,
  MANAGER_EDIT_NONE_SELECTED,
  MANAGER_EDIT_TOO_MANY_SELECTED,
  MANAGER_DELETE,
  MANAGER_DELETE_NONE_SELECTED,
  MANAGER_DELETE_TOO_MANY_SELECTED,
  MANAGER_DELETE_CONFIRM,
  MANAGER_TO_PTZ,
  MANAGER_TO_PTZ_NONE_SELECTED,
  MANAGER_TO_PTZ_TOO_MANY_SELECTED,
  MANAGER_BACK,
  MANAGER_INFO,
  EDITOR_DATE_NONE_SELECTED,
  EDITOR_TIME_START,
  EDITOR_TIME_STOP,
  EDITOR_EVENT,
  EDITOR_EVENT_PLACEHOLDER,
  EDITOR_SAVE,
  EDITOR_CANCEL,
  EDITOR_PREV_BOOKINGS,
  EDITOR_PREV_BOOKINGS_NONE,
  EDITOR_EVENT_MISSING,
  EDITOR_IDENTICAL_TIMES,
  EDITOR_CONFLICTING_BOOKINGS,
  CONTROLS_TITLE,
  CONTROLS_CAMERA_PREV,
  CONTROLS_CAMERA_NEXT,
  CONTROLS_CAMERA_MANAGER,
  CONTROLS_CAMERA_SINGLE,
  CONTROLS_PRESET_SAVE,
  CONTROLS_PRESET_LOAD,
  CONTROLS_PRESET_LOAD_NONE_SELECTED,
  CONTROLS_PRESET_DELETE,
  CONTROLS_PRESET_DELETE_NONE_SELECTED,
  CONTROLS_PRESET_DELETE_DENIED,
  CONTROLS_PRESET_DELETE_CONFIRM,
  CONTROLS_RECORD,
  CONTROLS_RECORD_WRONG_DATE,
  CONTROLS_RECORD_WRONG_TIME,
  CONTROLS_BACK_MANAGER,
  CONTROLS_BACK_QUICK,
  CONTROLS_SETTINGS_DENIED,
  CONTROLS_INFO,
  CONFLICT,
  MAIL_FILES_SUBJECT,
  MAIL_FILES_BODY,
  MAIL_CONFLICT_SUBJECT,
  MAIL_CONFLICT_BODY,
  MAIL_ADMIN_ADJUSTMENT_SUBJECT,
  MAIL_ADMIN_ADJUSTMENT_BODY,
  MAIL_ADMIN_DELETION_SUBJECT,
  MAIL_ADMIN_DELETION_BODY,
  MAIL_ADMIN_SEND,
  MAIL_ADMIN_NO_SEND,
  MAIL_NO_ANSWER,
  INPUT_CONFIRM,
  INPUT_CANCEL,
  INPUT_PASSWORD,
  INPUT_PRESET,
  DATE_FORMAT
};

using ID = ETextId;

class TextHandler
{
public:
  static void translate(const QString language);
  static const QString getText(ETextId id);
  static const QString getTextEnglish(ETextId id);
  static const QString getTextGerman(ETextId id);

  static QLocale locale;

private:
  static const QMap<ETextId, QString>* currentTexts;
  static const QMap<ETextId, QString> english;
  static const QMap<ETextId, QString> german;
};

class Translatable
{
public:
  virtual void updateTexts() = 0;
};