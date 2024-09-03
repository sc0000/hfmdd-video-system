#pragma once

#include <QWidget>
#include <QDialog>
#include <QListWidget>

#include "text-handler.hpp"
#include "fullscreen-dialog.hpp"
#include "booking-handler.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class QuickRecord;
}
QT_END_NAMESPACE

class QuickRecord : public FullScreenDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::QuickRecord* ui;
  QPropertyAnimation* infoLabelAnimation;
  bool infoLabelVisible;

  Booking* booking;

private:
  void updateExistingBookingsLabel(QDate date);
  void resizeBookingsOnExistingDateLabel();
  void updateStopTimeLabel();
  void updateDurationLabel();
  
private slots:
  void on_infoButton_pressed();
  void on_decreaseTimeBy20Button_pressed();
  void on_decreaseTimeBy05Button_pressed();
  void on_increaseTimeBy05Button_pressed();
  void on_increaseTimeBy20Button_pressed();

  void on_toPTZControlsButton_clicked();
  void on_toModeSelectButton_clicked();
  void on_logoutButton_clicked();  

public:
  QuickRecord(QWidget* parent = nullptr);
  ~QuickRecord();
  
  virtual void reload() override;
  virtual void updateTexts() override; 
};