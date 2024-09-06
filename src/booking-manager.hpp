#pragma once

#include <QWidget>
#include <QDialog>
#include <QListWidget>

#include "fullscreen-dialog.hpp"
#include "booking-handler.hpp"
#include "text-handler.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class BookingManager;
}
QT_END_NAMESPACE

class BookingManager : public FullScreenDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::BookingManager* ui;
  QPropertyAnimation* infoLabelAnimation;
  bool infoLabelVisible;
  int currentRow;

  QString makeEntry(const Booking& booking);
  void deleteBooking(const Booking* booking);
  
private slots:
  void on_infoButton_pressed();
  void on_newBookingButton_clicked();
  void on_editBookingButton_clicked();
  void on_deleteBookingButton_clicked();
  void on_toPTZControlsButton_clicked();
  void on_toModeSelectButton_clicked();
  void on_logoutButton_clicked();  

public:
  BookingManager(QWidget* parent = nullptr);
  ~BookingManager();

  void toLoginDialog();

  void constructList();

  virtual void reload() override;
  virtual void updateTexts() override;
};