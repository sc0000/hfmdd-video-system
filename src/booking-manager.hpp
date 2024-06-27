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
  
private slots:
  void on_newBookingButton_clicked();
  void on_editBookingButton_clicked();
  void on_deleteBookingButton_clicked();
  void on_toPTZControlsButton_clicked();
  void on_toModeSelectButton_clicked();
  void on_logoutButton_clicked();  

public:
  BookingManager(QWidget* parent = nullptr);
  ~BookingManager();
  static inline BookingManager* getInstance() { return instance; }
  virtual void reload() override;
  void toLoginDialog();

  void loadBookings();

  virtual void translate(ELanguage language) override;

};