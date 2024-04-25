#include <QDialog>
#include "booking.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BookingEditor;
}
QT_END_NAMESPACE

class BookingEditor : public QDialog
{
  Q_OBJECT

private:
  Ui::BookingEditor* ui;

  Booking& booking;
  bool isEditing;

private:
  void updateExistingBookingsLabel(const QDate& date);
 
private slots:
  void on_calendarWidget_clicked(QDate date);
  void on_startTimeEdit_timeChanged(QTime time);
  void on_stopTimeEdit_timeChanged(QTime time);
  void on_eventTypeLineEdit_textChanged(const QString& text);
  void on_saveButton_pressed();
  void on_cancelButton_pressed();
  
public:
  BookingEditor(Booking* bookingToEdit = nullptr, QWidget* parent = nullptr);
  ~BookingEditor();
  static void instance(Booking* bookingToEdit = nullptr, QWidget* parent = nullptr);

};