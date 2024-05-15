#include <QDialog>
#include "language.hpp"
#include "booking.h"

enum class ETimeToSet
{
  StartTime,
  StopTime,
  Default
};

QT_BEGIN_NAMESPACE
namespace Ui {
class BookingEditor;
}
QT_END_NAMESPACE

class BookingEditor : public QDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::BookingEditor* ui;

  Booking& booking;
  bool isEditing;

  ETimeToSet timeToSet;

private:
  void updateExistingBookingsLabel(const QDate& date);
  void drawTimespan();
 
private slots:
  void on_calendarWidget_clicked(QDate date);

  void on_setStartTimeButton_pressed();
  void on_setStopTimeButton_pressed();

  void on_decreaseTimeBy60Button_pressed();
  void on_decreaseTimeBy05Button_pressed();
  void on_increaseTimeBy05Button_pressed();
  void on_increaseTimeBy60Button_pressed();

  void on_eventTypeLineEdit_textChanged(const QString& text);
  void on_saveButton_pressed();
  void on_cancelButton_pressed();
  
public:
  BookingEditor(QWidget* parent = nullptr);
  ~BookingEditor();
  static void instance(QWidget* parent = nullptr);

  void reload(Booking* bookingToEdit = nullptr);
  virtual void translate(ELanguage language) override;  

};