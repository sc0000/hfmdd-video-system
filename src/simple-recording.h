#pragma once

#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QLineEdit>
#include <QCalendarWidget>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QDate>
#include <QTimeEdit>
#include <QVector>

#include <obs-frontend-api.h>

#include "booking.h"

class SimpleRecordingWidget : public QDockWidget
{
  Q_OBJECT

public:

  explicit SimpleRecordingWidget(QWidget* parent = nullptr);
  ~SimpleRecordingWidget();

  static QVector<obs_source_t*> sources;

private:

  void buttonClicked();

  QWidget* m_parent = nullptr;

  QLineEdit* m_overrideBaseDirLineEdit; // only for development
  QLineEdit* m_mailAddressLineEdit;

  // TODO: Replace with QTimeEdit?
  QLineEdit* m_setStartTimeLineEdit;
  QLineEdit* m_setStopTimeLineEdit;

  QTimeEdit* m_startTimeEdit;
  QTimeEdit* m_stopTimeEdit;
  
  QPushButton* m_testButton; // only for development

  QPushButton* m_confirmMailAddressButton;
  QPushButton* m_setDateButton;
  QPushButton* m_confirmDateTimeButton;
  QPushButton* m_recordButton;

  QLabel* m_bookedTimesLabel;

  QCalendarWidget* m_calendar;

  QDate m_selectedDate;
  QVector<Booking> m_selectedDateBookings;

  bool m_mailAddressIsValid;
  bool m_dateTimeIsValid;

  QString m_outerDir;
  QString m_innerDir;
  QString m_baseDir;

  QString m_email;

  // TODO: Move everything re: presets to the ptz controls
  QString m_currentPresetName;

private:

  // ? Should link/mail logic be in its own class ?
  QString sendDownloadLink(const Booking& booking, const QString& path);

private slots:

  void onTestButtonClicked(); // only for development

  void onConfirmMailAddressButtonClicked();
  void onSetDateButtonClicked();
  void onConfirmDateTimeButtonClicked();
  void onRecordButtonClicked();
  
  void onCalendarCellClicked(const QDate& date);

  void onOverrideBaseDirLineEdited(const QString& text);
  void onMailAddressLineEdited(const QString& text);

  void updateFilterSettings(const char* path);
  void resetFilterSettings();
};