#pragma once

#include <QWidget>
// #include <QDockWidget>
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

  // children
  QListWidget* bookingsList;

  // props
  QString currentMailAddress;
  QVector<Booking> bookings;

  // methods
  void sortBookings();
  QString makeEntry(const Booking& booking);
  
private slots:
  void on_newBookingButton_pressed();
  void on_editBookingButton_pressed();
  void on_deleteBookingButton_pressed();  

public:
  BookingManager(QWidget* parent = nullptr);
  ~BookingManager();
  static inline BookingManager* getInstance() { return instance; }
  inline QListWidget* getBookingsList() { return bookingsList; }
  inline const QString& getCurrentMailAddress() { return currentMailAddress; }

  void loadBookings();
  void addBooking(const Booking& booking);
  void updateBooking(const Booking& booking);
};