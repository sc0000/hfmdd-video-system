#include <QLabel>

#include "booking-handler.hpp"
#include "mail-handler.hpp"
#include "widgets.hpp"
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "text-handler.hpp"
#include "booking-manager.hpp"
#include "ui_booking-editor.h"
#include "booking-editor.hpp"

BookingEditor::BookingEditor(QWidget* parent)
  : AnimatedDialog(parent),
    ui(new Ui::BookingEditor),
    handlebar(nullptr),
    booking(BookingHandler::currentBooking),
    timeToSet(ETimeToSet::StartTime)
{
  ui->setupUi(this);

  handlebar = new Handlebar(this, EHandlebarStyle::Black, "Booking Editor");
 
  ui->masterWidget->setFixedWidth(width() - 16);
  ui->masterWidget->setFixedHeight(height() - 56);
  ui->masterWidget->move(QPoint(8, 48));

  ui->calendarWidget->setStyleSheet(
    "QCalendarWidget { border: 1px solid rgb(31, 30, 31); }"
    "QPushButton:disabled { }"
    "QToolButton:disabled { }"
  );

  ui->scrollArea->takeWidget();

  ui->bookingsOnSelectedDateLabel->setLayout(ui->scrollAreaLayout);
  ui->bookingsOnSelectedDateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->bookingsOnSelectedDateLabel->setMinimumSize(595, 64);

  ui->scrollArea->setWidgetResizable(false);
  ui->scrollArea->setWidget(ui->bookingsOnSelectedDateLabel);

  updateTexts();
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setModal(true);
}

BookingEditor::~BookingEditor()
{
  delete ui;
}

void BookingEditor::reload(Booking* bookingToEdit)
{
  if (bookingToEdit) {
    isEditing = true;
    originalBooking = *bookingToEdit;

    booking->date = bookingToEdit->date;
    booking->email = bookingToEdit->email;
    booking->index = bookingToEdit->index;
    booking->startTime = bookingToEdit->startTime;
    booking->stopTime = bookingToEdit->stopTime;
    booking->event = bookingToEdit->event;
    booking->isConflicting = bookingToEdit->isConflicting;
    booking->index = bookingToEdit->index;

    ui->eventTypeLineEdit->setText(booking->event);
  }

  else {
    isEditing = false;
    QTime currentTime = QTime::currentTime();

    booking->date = QDate::currentDate();
    booking->email = MailHandler::currentEmail;
    booking->event = "";
    booking->isConflicting = false;
    booking->index = JsonParser::availableIndex();

    booking->startTime = currentTime < QTime(23, 0) && currentTime > QTime(7, 0) ?
      currentTime :
      QTime(8, 0);

    BookingHandler::roundTime(booking->startTime);
    
    booking->stopTime = 
      booking->startTime.addSecs(60 * 60) < QTime(23, 0) && booking->startTime.addSecs(60 * 60) > QTime(7, 0) ? 
      booking->startTime.addSecs(60 * 60) : 
      QTime(23, 0);

    ui->eventTypeLineEdit->setText("");
    ui->eventTypeLineEdit->setStyleSheet("QLineEdit { font-style: italic; }");
  }

  timeToSet = ETimeToSet::StartTime;

  ui->setStartTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px; border: 1px solid rgb(31, 30, 31); }");
  ui->setStopTimeButton->setStyleSheet(
    "QPushButton { background-color: rgb(229, 230, 230); font-size: 16px; border: 1px solid rgb(31, 30, 31); }"
    "QPushButton:hover { background-color: #ffbf00; }"  
  );

  ui->calendarWidget->setMinimumDate(QDate::currentDate());
  ui->calendarWidget->setSelectedDate(booking->date);
  updateExistingBookingsLabel(booking->date);

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);

  drawTimespan();

  ui->saveButton->setAttribute(Qt::WA_UnderMouse, false);
  ui->cancelButton->setAttribute(Qt::WA_UnderMouse, false);

  QPushButton* closeButton = handlebar->getCloseButton();
  
  if (closeButton)
    closeButton->setAttribute(Qt::WA_UnderMouse, false);

  fade();
  exec();
}

void BookingEditor::updateTexts()
{
  ui->bookingsOnSelectedDateLabel->setText(TextHandler::getText(ID::EDITOR_DATE_NONE_SELECTED));
  ui->setStartTimeButton->setText(TextHandler::getText(ID::EDITOR_TIME_START));
  ui->setStopTimeButton->setText(TextHandler::getText(ID::EDITOR_TIME_STOP));
  ui->eventTypeLabel->setText(TextHandler::getText(ID::EDITOR_EVENT));
  ui->eventTypeLineEdit->setPlaceholderText(TextHandler::getText(ID::EDITOR_EVENT_PLACEHOLDER));
  ui->saveButton->setText(TextHandler::getText(ID::EDITOR_SAVE));
  ui->cancelButton->setText(TextHandler::getText(ID::EDITOR_CANCEL));
  ui->calendarWidget->setLocale(TextHandler::locale);
}

void BookingEditor::updateExistingBookingsLabel(const QDate& date)
{ 
  BookingHandler::getBookingsOnSelectedDate(date);

  const QVector<Booking*>& bosd = BookingHandler::bookingsOnSelectedDate;

  QString dateStr = TextHandler::locale.toString(date);

  if (bosd.isEmpty() ||
     (bosd.size() == 1 && bosd[0]->index == booking->index)) {
        ui->bookingsOnSelectedDateLabel->setText(
          TextHandler::getText(ID::EDITOR_PREV_BOOKINGS_NONE) + dateStr + "."
        );

        booking->isConflicting = false;
        resizeBookingsOnExistingDateLabel();
        return;
  }

  QString str = TextHandler::getText(ID::EDITOR_PREV_BOOKINGS) + dateStr + ":\n";

  str += "<html><head/><body>";

  booking->isConflicting = false;

  for (const Booking* b : bosd) {
    if (b->index == booking->index) continue;

    bool isConflicting = false;
    
    if (BookingHandler::bookingsAreConflicting(booking, b)) {
      isConflicting = true;
      booking->isConflicting = true;
      str += "<span style=\"background-color: rgb(31, 30, 31); color: rgb(254, 253, 254);\">";
    }
      
    str += b->startTime.toString("HH:mm") + "-" + b->stopTime.toString("HH:mm") + ": " + 
      b->event + " (" + b->email + ")";

    if (isConflicting) 
      str += TextHandler::getText(ID::CONFLICT) + "</span>";

    str += "<br/>";
  }

  str += "</body></html>";

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  ui->bookingsOnSelectedDateLabel->setText(str);
  
  resizeBookingsOnExistingDateLabel();
}

void BookingEditor::drawTimespan()
{
  ui->timeLabel->setText(booking->startTime.toString("HH:mm") + " – " + booking->stopTime.toString("HH:mm"));
}

void BookingEditor::resizeBookingsOnExistingDateLabel()
{
  QFontMetrics fontMetrics(ui->bookingsOnSelectedDateLabel->font());
  int lineHeight = fontMetrics.lineSpacing();
  int numLines = 2 + BookingHandler::bookingsOnSelectedDate.size();
  int labelHeight = lineHeight * numLines;

  ui->bookingsOnSelectedDateLabel->setFixedHeight(labelHeight);
}

void BookingEditor::on_calendarWidget_clicked(QDate date)
{
  booking->date = date;
  updateExistingBookingsLabel(date);
}

void BookingEditor::on_setStartTimeButton_pressed()
{
  ui->setStartTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px; border: 1px solid rgb(31, 30, 31); }");
  ui->setStopTimeButton->setStyleSheet(
    "QPushButton { background-color: rgb(229, 230, 230); font-size: 16px; border: 1px solid rgb(31, 30, 31); }"
    "QPushButton:hover { background-color: #ffbf00; }"  
  );

  timeToSet = ETimeToSet::StartTime;
}
  
void BookingEditor::on_setStopTimeButton_pressed()
{
  ui->setStopTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px; border: 1px solid rgb(31, 30, 31); }");
  ui->setStartTimeButton->setStyleSheet(
    "QPushButton { background-color: rgb(229, 230, 230); font-size: 16px; border: 1px solid rgb(31, 30, 31); }"
    "QPushButton:hover { background-color: #ffbf00; }"  
  );

  timeToSet = ETimeToSet::StopTime;
}

void BookingEditor::on_decreaseTimeBy60Button_pressed()
{
  switch(timeToSet) {
    case ETimeToSet::StartTime: {
      QTime newStartTime = booking->startTime.addSecs(60 * -60);

      if (newStartTime > QTime(23, 0) || newStartTime < QTime( 7, 0))
        return;

      booking->startTime = newStartTime;
      break;
    }

    case ETimeToSet::StopTime: {
      QTime newStopTime = booking->stopTime.addSecs(60 * -60);

      if (newStopTime > QTime(23, 0) || newStopTime < QTime( 7, 0))
        return;

      if (newStopTime <= booking->startTime) {
        QTime newStartTime = newStopTime.addSecs(60 * -60);
        booking->startTime = newStartTime > QTime(7, 0) ? newStartTime : QTime(7, 0);
      }

      booking->stopTime = newStopTime;
      break;
    }

    case ETimeToSet::Default:
      break;
  }

  updateExistingBookingsLabel(booking->date);
  drawTimespan();
}

void BookingEditor::on_decreaseTimeBy05Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking->startTime.addSecs(60 * -5);

   if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    booking->startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking->stopTime.addSecs(60 * -5);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    if (newStopTime <= booking->startTime) {
      QTime newStartTime = newStopTime.addSecs(60 * -5);
      booking->startTime = newStartTime > QTime(7, 0) ? newStartTime : QTime(7, 0);
    }

    booking->stopTime = newStopTime;
  }

  updateExistingBookingsLabel(booking->date);
  drawTimespan();
}

void BookingEditor::on_increaseTimeBy05Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking->startTime.addSecs(60 * 5);

    if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    if (newStartTime >= booking->stopTime) {
      QTime newStopTime = newStartTime.addSecs(60 * 5);
      booking->stopTime = newStopTime < QTime(23, 0) ? newStopTime : QTime(23, 0);
    }

    booking->startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking->stopTime.addSecs(60 * 5);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    booking->stopTime = newStopTime;
  }

  updateExistingBookingsLabel(booking->date);
  drawTimespan();
}

void BookingEditor::on_increaseTimeBy60Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking->startTime.addSecs(60 * 60);

    if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    if (newStartTime >= booking->stopTime) {
      QTime newStopTime = newStartTime.addSecs(60 * 60);
      booking->stopTime = newStopTime < QTime(23, 0) ? newStopTime : QTime(23, 0);
    }

    booking->startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking->stopTime.addSecs(60 * 60);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    booking->stopTime = newStopTime;
  }

  updateExistingBookingsLabel(booking->date);
  drawTimespan();
}

void BookingEditor::on_eventTypeLineEdit_textChanged(const QString& text)
{
  booking->event = text;

  if (text == "")
    ui->eventTypeLineEdit->setStyleSheet("QLineEdit { font-style: italic; }");

  else
    ui->eventTypeLineEdit->setStyleSheet("QLineEdit { font-style: normal; }");
}

void bookingEditorAccept()
{
  if (Widgets::bookingEditor)
    Widgets::bookingEditor->accept();
}

void bookingEditorReject()
{
  if (Widgets::bookingEditor)
    Widgets::bookingEditor->reject();
}

void BookingEditor::on_saveButton_clicked()
{
  if (booking->event == "") {
    Widgets::okDialog->display(
      TextHandler::getText(ID::EDITOR_EVENT_MISSING)
    );

    return;
  }

  if (booking->startTime.toString("HH:mm") == booking->stopTime.toString("HH:mm")) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::EDITOR_IDENTICAL_TIMES)
    );

    return;
  }

  if (booking->isConflicting) {
    int result = Widgets::okCancelDialog->display(
      TextHandler::getText(ID::EDITOR_CONFLICTING_BOOKINGS), true
    );
    
    if (result == QDialog::Rejected)
      return;

    const QString& sendWarningMsg = MailHandler::sendMail(EMailType::BookingConflictWarning, booking);

    // Widgets::okDialog->display(sendWarningMsg);
  }
    
  if (isEditing) {
    JsonParser::updateBooking(booking);

    if (MailHandler::isAdmin && booking->email != MailHandler::adminEmail) {
      Widgets::adminMailDialog->display(EAdminMailType::Adjustment, booking, &originalBooking);
    }
  }
    
  else JsonParser::addBooking(booking);
  
  Widgets::bookingManager->constructList();

  isEditing = false;
 
  fade(&bookingEditorAccept);
}

void BookingEditor::on_cancelButton_clicked()
{
  fade(&bookingEditorReject);
}