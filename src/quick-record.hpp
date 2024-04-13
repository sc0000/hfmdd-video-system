#pragma once

#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include "booking.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class QuickRecord;
}
QT_END_NAMESPACE

class QuickRecord : public QDialog
{
  Q_OBJECT

private:
  static QuickRecord* instance;
  Ui::QuickRecord* ui;

  QVector<Booking> bookings;
  QVector<Booking> bookingsOnSelectedDate;

private:
  void repositionMasterWidget();
  void sortBookings();
  QString makeEntry(const Booking& booking);

  void updateExistingBookingsLabel(QDate date);
  void updateConflictingBookings(const QDate& date);
  bool bookingsAreConflicting(const Booking& booking0, const Booking& booking1);

  void roundTime(QTime& time);
  void updateStopTimeLabel();
  
private slots:
  void on_decreaseTimeBy20Button_pressed();
  void on_decreaseTimeBy05Button_pressed();
  void on_increaseTimeBy05Button_pressed();
  void on_increaseTimeBy20Button_pressed();

  void on_toPTZControlsButton_pressed();
  void on_toModeSelectButton_pressed();
  void on_logoutButton_pressed();  

public:
  QuickRecord(QWidget* parent = nullptr);
  ~QuickRecord();
  static inline QuickRecord* getInstance() { return instance; }
  
  void reload();

  void loadBookings();
  
  Booking booking;
};