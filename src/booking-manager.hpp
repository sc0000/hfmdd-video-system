#pragma once

#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include "booking.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BookingManager;
}
QT_END_NAMESPACE

class BookingManager : public QDialog
{
  Q_OBJECT

private:
  static BookingManager* instance;
  Ui::BookingManager* ui;

  QVector<Booking> bookings;

private:
  void sortBookings();
  QString makeEntry(const Booking& booking);
  
private slots:
  void on_newBookingButton_pressed();
  void on_editBookingButton_pressed();
  void on_deleteBookingButton_pressed();
  void on_toPTZControlsButton_pressed();  

public:
  BookingManager(QWidget* parent = nullptr);
  ~BookingManager();
  static inline BookingManager* getInstance() { return instance; }
  
  void loadBookings();

  Booking* selectedBooking;
};