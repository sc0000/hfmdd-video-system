/* Full screen widget for booking instant recordings
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <QLabel>
#include <QScreen>

#include "booking-handler.hpp"
#include "mail-handler.hpp"
#include "settings.hpp"
#include "widgets.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "booking-manager.hpp"
#include "text-handler.hpp"
#include "mode-select.hpp"
#include "styles.hpp"
#include "ui_quick-record.h"
#include "quick-record.hpp"

QuickRecord::QuickRecord(QWidget* parent)
  : FullScreenDialog(parent), 
    ui(new Ui::QuickRecord),
    infoLabelVisible(false),
    booking(BookingHandler::currentBooking),
    numLaterBookings(0)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  
  setWindowTitle("Quick Record");
  ui->setupUi(this);
  center(ui->masterWidget);

  ui->infoButton->setStyleSheet(Styles::infoButtonOff);

  ui->infoLabel->setStyleSheet(Styles::infoLabel);

  infoLabelAnimation = new QPropertyAnimation(ui->infoLabel, "maximumWidth");
  infoLabelAnimation->setDuration(100);

  ui->scrollArea->takeWidget();

  ui->bookingsOnSelectedDateLabel->setLayout(ui->scrollAreaLayout);
  ui->bookingsOnSelectedDateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->bookingsOnSelectedDateLabel->setMinimumSize(527, 64);

  ui->scrollArea->setWidgetResizable(false);
  ui->scrollArea->setWidget(ui->bookingsOnSelectedDateLabel);


  updateTexts();

  setModal(false);
  hide();
  
  BookingHandler::loadBookings();
}

QuickRecord::~QuickRecord()
{
  delete ui;
}

void QuickRecord::reload()
{
  if (!MailHandler::mailAddressIsValid) {
    Widgets::loginDialog->reload();
    return;
  }

  raise();
  center(ui->masterWidget);

  BookingHandler::loadBookings();
  BookingHandler::reevaluateConflicts();

  booking->date = QDate::currentDate();
  booking->startTime = QTime::currentTime();
  booking->stopTime = booking->startTime.addSecs(60 * 60);
  booking->email = MailHandler::currentEmail;
  booking->index = JsonParser::availableIndex();
  booking->event = "Quick Record";

  BookingHandler::roundTime(booking->stopTime);
  BookingHandler::getBookingsOnSelectedDate(booking->date);
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking->date);
  updateDurationLabel();

  ui->infoLabel->setMaximumWidth(0);
  infoLabelVisible = false;
}

void QuickRecord::updateTexts()
{
  ui->recordUntilLabel->setText(TextHandler::getText(ID::QUICK_UNTIL));
  ui->toPTZControlsButton->setText(TextHandler::getText(ID::QUICK_TO_PTZ));
  ui->toModeSelectButton->setText(TextHandler::getText(ID::QUICK_BACK));

  // ui->infoLabel->setFont(QFont("DaxOT", 11));
  ui->infoLabel->setText(TextHandler::getText(ID::QUICK_INFO));
}

void QuickRecord::updateExistingBookingsLabel(QDate date)
{
  BookingHandler::getBookingsOnSelectedDate(date);

  const QVector<Booking*>& bosd = BookingHandler::bookingsOnSelectedDate;

  booking->isConflicting = false;
  numLaterBookings = 0;

  QString str = TextHandler::getText(ID::QUICK_PREV_BOOKINGS);

  str += "<html><head/><body>";

  for (const Booking* b : bosd) {
    if (b->index == booking->index ||
        b->stopTime < booking->startTime) continue;

    ++numLaterBookings;
    bool isConflicting = false;
    
    if (BookingHandler::bookingsAreConflicting(booking, b)) {
      isConflicting = true;
      booking->isConflicting = true;
      str += QString(
        "<span style=\"background-color: %1; color: %2;\">"
      ).arg(Color::black).arg(Color::white);
    }

    str += b->startTime.toString("HH:mm") + "-" + 
      b->stopTime.toString("HH:mm") + ": " + 
      b->event + " (" + b->email + ")";

    if (isConflicting)
      str += TextHandler::getText(ID::CONFLICT) + "</span>";

    str += "<br/>";
  }

  str += "</body></html>";

  if (numLaterBookings == 0 ||
     (bosd.size() == 1 && bosd[0]->index == booking->index)) {
        ui->bookingsOnSelectedDateLabel->setText(TextHandler::getText(ID::QUICK_PREV_BOOKINGS_NONE));
        resizeBookingsOnExistingDateLabel();
        return;
  }

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  ui->bookingsOnSelectedDateLabel->setText(str);

  resizeBookingsOnExistingDateLabel();
}

void QuickRecord::resizeBookingsOnExistingDateLabel()
{
  QFontMetrics fontMetrics(ui->bookingsOnSelectedDateLabel->font());
  int lineHeight = fontMetrics.lineSpacing();
  int numLines = numLaterBookings + 2;
  int labelHeight = lineHeight * numLines;

  ui->bookingsOnSelectedDateLabel->setFixedHeight(labelHeight);
}

void QuickRecord::updateStopTimeLabel()
{ 
  ui->stopTimeLabel->setText(booking->stopTime.toString("HH:mm"));
}

void QuickRecord::updateDurationLabel()
{
  int secs = QTime::currentTime().secsTo(booking->stopTime);
  int totalMin = secs / 60;
  QString hours = QString::number(totalMin / 60);
  QString minutes = QString::number(totalMin % 60);

  ui->durationLabel->setText(
    "( " + hours + "h " + minutes + TextHandler::getText(ID::QUICK_FROM_NOW) + " )"  
  );
}

void QuickRecord::on_infoButton_pressed()
{
   infoLabelAnimation->stop();

  if (infoLabelVisible) {
      infoLabelAnimation->setStartValue(420);
      infoLabelAnimation->setEndValue(0);
      infoLabelVisible = false;

      ui->infoButton->setStyleSheet(Styles::infoButtonOff);
  } 
  
  else {
      infoLabelAnimation->setStartValue(0);
      infoLabelAnimation->setEndValue(420);
      infoLabelVisible = true;

      ui->infoButton->setStyleSheet(Styles::infoButtonOn);
  }

  infoLabelAnimation->start();
}

void QuickRecord::on_decreaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking->stopTime.addSecs(60 * -20);

  if (newStopTime <= booking->startTime) 
    return;

  if (newStopTime > QTime(23, 0))  
    return;

  booking->stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking->date);
  updateDurationLabel();
}

void QuickRecord::on_decreaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking->stopTime.addSecs(60 * -5);

  if (newStopTime <= booking->startTime) 
    return;
  
  if (newStopTime > QTime(23, 0)) 
    return;

  booking->stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking->date);
  updateDurationLabel();
}

void QuickRecord::on_increaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking->stopTime.addSecs(60 * 5);

  if (newStopTime > QTime(23, 0)) 
    return;

  booking->stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking->date);
  updateDurationLabel();
}

void QuickRecord::on_increaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking->stopTime.addSecs(60 * 20);

  if (newStopTime > QTime(23, 0))
    return;

  booking->stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking->date);
  updateDurationLabel();
}

void QuickRecord::on_toPTZControlsButton_clicked()
{ 
  if (booking->isConflicting) {
    int result = Widgets::okCancelDialog->display(
      TextHandler::getText(ID::EDITOR_CONFLICTING_BOOKINGS), true
    );
    
    if (result == QDialog::Rejected)
      return;

    MailHandler::sendMail(EMailType::BookingConflictWarning, booking);
  }

  JsonParser::addBooking(booking);
  
  // TODO: Check: always reset or update, and setup option to reset manually?
  PTZSettings::resetFilterSettings();

  Widgets::ptzControls->reload();
  Widgets::showFullScreenDialogs(false);
}

void QuickRecord::on_toModeSelectButton_clicked()
{
  fade(Widgets::modeSelect);
}

void QuickRecord::on_logoutButton_clicked()
{
  fade(Widgets::loginDialog);
}
