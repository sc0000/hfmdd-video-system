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
  OVERVIEW_NEW,
  OVERVIEW_EDIT,
  OVERVIEW_EDIT_NONE_SELECTED,
  OVERVIEW_EDIT_TOO_MANY_SELECTED,
  OVERVIEW_DELETE,
  OVERVIEW_DELETE_NONE_SELECTED,
  OVERVIEW_DELETE_TOO_MANY_SELECTED,
  OVERVIEW_DELETE_CONFIRM,
  OVERVIEW_TO_PTZ,
  OVERVIEW_TO_PTZ_NONE_SELECTED,
  OVERVIEW_TO_PTZ_TOO_MANY_SELECTED,
  OVERVIEW_BACK,
  OVERVIEW_INFO,
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
  CONTROLS_CAMERA_OVERVIEW,
  CONTROLS_CAMERA_SINGLE,
  CONTROLS_PRESET_SAVE,
  CONTROLS_PRESET_LOAD,
  CONTROLS_PRESET_LOAD_NONE_SELECTED,
  CONTROLS_PRESET_DELETE,
  CONTROLS_PRESET_DELETE_NONE_SELECTED,
  CONTROLS_PRESET_DELETE_DENIED,
  CONTROLS_PRESET_DELETE_CONFIRM,
  CONTROLS_RECORD,
  CONTROLS_BACK_OVERVIEW,
  CONTROLS_BACK_QUICK,
  CONTROLS_SETTINGS_DENIED,
  CONTROLS_INFO,
  CONFLICT,
  MAIL_SUBJECT
};

using ID = ETextId;

class TextManager
{
public:
  static void translate(const QString language);
  static const QString getText(ETextId id);

  static QLocale locale;

private:
  static QMap<ETextId, QString>* currentTexts;
  static QMap<ETextId, QString> english;
  static QMap<ETextId, QString> german;
};