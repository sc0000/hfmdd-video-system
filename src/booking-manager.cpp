/* Full screen widget for managing one's bookings
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <QLabel>
#include <QScreen>

#include "mail-handler.hpp"
#include "settings.hpp"
#include "widgets.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "text-handler.hpp"
#include "mode-select.hpp"
#include "styles.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"

BookingManager::BookingManager(QWidget* parent)
  : FullScreenDialog(parent), 
    ui(new Ui::BookingManager),
    currentRow(0)
{
  setWindowFlags(
    windowFlags() | 
    Qt::MSWindowsFixedSizeDialogHint | 
    Qt::FramelessWindowHint
  );
  
  setWindowTitle("Booking Manager");
  ui->setupUi(this);
  center(ui->masterWidget);

  ui->infoButton->setStyleSheet(Styles::infoButtonOff);

  ui->infoLabel->setStyleSheet(Styles::infoLabel);

  infoLabelAnimation = new QPropertyAnimation(ui->infoLabel, "maximumWidth");
  infoLabelAnimation->setDuration(100);

  ui->infoLabel->setFont(QFont("DaxOT", 12));

  ui->bookingsList->setStyleSheet(
    QString("QListWidget { border: 1px solid %1; }").arg(Color::black)
  );

  updateTexts();
  setModal(false);
  hide();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::toLoginDialog()
{ 
  fade(Widgets::loginDialog);
}

void BookingManager::constructList()
{   
  BookingHandler::loadBookings();
  BookingHandler::reevaluateConflicts();

  if (!ui->bookingsList) return;

  ui->bookingsList->clear();

  qsizetype size = BookingHandler::loadedBookings.size();

  for (const Booking* booking : BookingHandler::loadedBookings) {
    QString entryText = makeEntry(*booking);
    QListWidgetItem* item = new QListWidgetItem(entryText);

    if (booking->isConflicting) {
      item->setBackground(QBrush(QColor(31, 30, 31)));
      item->setForeground(QBrush(QColor(254, 253, 254)));
    }

    ui->bookingsList->addItem(item);
  }

  QListWidgetItem* currentItem = ui->bookingsList->item(currentRow);

  if (currentItem) {
    currentItem->setSelected(true);
    ui->bookingsList->setCurrentRow(currentRow);
  }

  ui->bookingsList->setFocus();
}

void BookingManager::reload()
{
  if (!MailHandler::mailAddressIsValid) {
    Widgets::loginDialog->reload();
    return;
  }
  
  raise();
  center(ui->masterWidget);

  BookingHandler::loadBookings();
  BookingHandler::reevaluateConflicts();
  constructList();

  ui->infoLabel->setMaximumWidth(0);
  infoLabelVisible = false;
}

QString BookingManager::makeEntry(const Booking& booking)
{
  const QString& format = TextHandler::getText(ID::DATE_FORMAT);

  QString entry = 
    TextHandler::locale.toString(booking.date, format) + "\t" +
    booking.startTime.toString("HH:mm") + " - " +
    booking.stopTime.toString("HH:mm") + "\t" +
    booking.event.leftJustified(20, ' ') + "\t" +
    booking.email.leftJustified(20, ' ') +
    (booking.isConflicting ? TextHandler::getText(ID::CONFLICT) : "");

  return entry;
}

void BookingManager::deleteBooking(const Booking* booking)
{
  if (!booking) return;

  if (MailHandler::isAdmin && 
      booking->email != MailHandler::adminEmail &&
      !BookingHandler::inThePast(booking)) 
    Widgets::adminMailDialog->display(EAdminMailType::Deletion, booking);
  
  JsonParser::removeBooking(booking);
  constructList();
}

void BookingManager::updateTexts()
{
  ui->newBookingButton->setText(TextHandler::getText(ID::MANAGER_NEW));
  ui->editBookingButton->setText(TextHandler::getText(ID::MANAGER_EDIT));
  ui->deleteBookingButton->setText(TextHandler::getText(ID::MANAGER_DELETE));
  ui->toPTZControlsButton->setText(TextHandler::getText(ID::MANAGER_TO_PTZ));
  ui->toModeSelectButton->setText(TextHandler::getText(ID::MANAGER_BACK));
  ui->infoLabel->setText(TextHandler::getText(ID::MANAGER_INFO));

  if (TextHandler::locale == QLocale::German)
    ui->infoLabel->setFont(QFont("DaxOT", 11));

  else
    ui->infoLabel->setFont(QFont("DaxOT", 12));
}

void BookingManager::on_infoButton_pressed()
{
  infoLabelAnimation->stop();

  if (infoLabelVisible) {
      infoLabelAnimation->setStartValue(420);
      infoLabelAnimation->setEndValue(0);
      infoLabelVisible = false;

      ui->infoButton->setStyleSheet(Styles::infoButtonOff);
  } 
  
  else {
      // ui->infoLabel->setMaximumWidth(420);
      infoLabelAnimation->setStartValue(0);
      infoLabelAnimation->setEndValue(420);
      infoLabelVisible = true;

      ui->infoButton->setStyleSheet(Styles::infoButtonOn);
  }

  infoLabelAnimation->start();
}

void BookingManager::on_newBookingButton_clicked()
{
  Widgets::bookingEditor->reload(nullptr);
}

void BookingManager::on_editBookingButton_clicked()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_EDIT_NONE_SELECTED)
    );

    return;
  }

  if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_EDIT_TOO_MANY_SELECTED)
    );

    return;
  }

  currentRow = ui->bookingsList->currentRow();
  Booking* selectedBooking = BookingHandler::loadedBookings[currentRow];

  if (!selectedBooking) return;

  if (!MailHandler::isAdmin && selectedBooking->date < QDate::currentDate()) {
    int result = Widgets::okCancelDialog->display(
      TextHandler::getText(ID::MANAGER_EDIT_OBSOLETE)
    );

    if (result == QDialog::Rejected) 
      return;

    deleteBooking(selectedBooking);
    return;
  }

  Widgets::bookingEditor->reload(selectedBooking);
}

void BookingManager::on_deleteBookingButton_clicked()
{
  if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_DELETE_NONE_SELECTED)
    );

    return;
  }

  // TODO: Select more than one booking for deletion?

  int result = Widgets::okCancelDialog->display(
    TextHandler::getText(ID::MANAGER_DELETE_CONFIRM)
  );

  if (result == QDialog::Rejected) 
    return;
  
  currentRow = ui->bookingsList->currentRow();
  Booking* selectedBooking = BookingHandler::loadedBookings[currentRow];

  if (!selectedBooking) return;

  deleteBooking(selectedBooking);
}

void BookingManager::on_toPTZControlsButton_clicked()
{
  if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_TO_PTZ_NONE_SELECTED)
    );

    return;
  }

  if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_TO_PTZ_TOO_MANY_SELECTED)
    );

    return;
  }

  currentRow = ui->bookingsList->currentRow();
  Booking* selectedBooking = BookingHandler::loadedBookings[currentRow];

  if (!MailHandler::isAdmin && selectedBooking->date < QDate::currentDate()) {
    int result = Widgets::okCancelDialog->display(
      TextHandler::getText(ID::MANAGER_TO_PTZ_OBSOLETE)
    );

    if (result == QDialog::Rejected) 
      return;

    deleteBooking(selectedBooking);
    return;
  }

  BookingHandler::currentBooking->date = selectedBooking->date;
  BookingHandler::currentBooking->email = selectedBooking->email;
  BookingHandler::currentBooking->index = selectedBooking->index;
  BookingHandler::currentBooking->startTime = selectedBooking->startTime;
  BookingHandler::currentBooking->stopTime = selectedBooking->stopTime;
  BookingHandler::currentBooking->event = selectedBooking->event;
  BookingHandler::currentBooking->isConflicting = selectedBooking->isConflicting;
  BookingHandler::currentBooking->index = selectedBooking->index;

  // TODO: Check: always reset or update, and setup option to reset manually?
  PTZSettings::resetFilterSettings();

  Widgets::ptzControls->reload();
  Widgets::showFullScreenDialogs(false);
}

void BookingManager::on_toModeSelectButton_clicked()
{
  Widgets::showFullScreenDialogs(true);
  fade(Widgets::modeSelect);
}

void BookingManager::on_logoutButton_clicked()
{
  Widgets::showFullScreenDialogs(true);
  fade(Widgets::loginDialog);
}
