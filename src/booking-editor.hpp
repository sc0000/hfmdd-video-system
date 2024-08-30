#include <QMouseEvent>

#include "animated-dialog.hpp"
#include "booking-handler.hpp"
#include "text-handler.hpp"

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

class BookingEditor : public AnimatedDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::BookingEditor* ui;

  Booking& booking;
  bool isEditing;

  ETimeToSet timeToSet;

  bool mousePressed = false;
  QPoint dragPosition;

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
  void on_saveButton_clicked();
  void on_cancelButton_clicked();
  
public:
  BookingEditor(QWidget* parent = nullptr);
  ~BookingEditor();

  void reload(Booking* bookingToEdit = nullptr);
  virtual void updateTexts() override;  

};