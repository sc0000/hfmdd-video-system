#pragma once

#include <QWidget>
#include <QDockWidget>
#include <QDialog>

// #ifdef __cplusplus
// extern "C" {
// #endif

// extern void booking_manager_load(void);

// #ifdef __cplusplus
// }
// #endif

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

private slots:
  void on_newBookingButton_pressed();
  void on_editSelectedBookingButton_pressed();
    
public:
  BookingManager(QWidget *parent = nullptr);
  ~BookingManager();
  static inline BookingManager* getInstance() { return instance; }

};