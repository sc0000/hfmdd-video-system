#pragma once

#include <QWidget>
#include <QDialog>
#include <QListWidget>

#include "language.hpp"
#include "fullscreen-dialog.hpp"
#include "booking.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BookingManager;
}
QT_END_NAMESPACE

class BookingManager : public FullScreenDialog, public Translatable
{
  Q_OBJECT

private:
  static BookingManager* instance;
  Ui::BookingManager* ui;

  QVector<Booking>& bookings;

private:
  QString makeEntry(const Booking& booking);
  
private slots:
  void on_newBookingButton_pressed();
  void on_editBookingButton_pressed();
  void on_deleteBookingButton_pressed();
  void on_toPTZControlsButton_pressed();
  void on_toModeSelectButton_pressed();
  void on_logoutButton_pressed();  

public:
  BookingManager(QWidget* parent = nullptr);
  ~BookingManager();
  static inline BookingManager* getInstance() { return instance; }
  virtual void reload() override;
  void toLoginDialog();

  void loadBookings();

  virtual void translate(ELanguage language) override;

};