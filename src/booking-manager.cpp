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
#include "ui_booking-manager.h"
#include "booking-manager.hpp"

BookingManager::BookingManager(QWidget* parent)
  : FullScreenDialog(parent), 
    ui(new Ui::BookingManager),
    bookings(BookingHandler::loadedBookings),
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

  ui->infoButton->setStyleSheet(
    "QPushButton { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(42, 130, 218); }"
    "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
  );

  ui->infoLabel->setStyleSheet(
    "QLabel { background-color: rgb(42, 130, 218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
  );

  infoLabelAnimation = new QPropertyAnimation(ui->infoLabel, "maximumWidth");
  infoLabelAnimation->setDuration(100);

  ui->infoLabel->setFont(QFont("DaxOT", 11));

  ui->bookingsList->setStyleSheet("QListWidget { border: 1px solid rgb(31, 30, 31); }");

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

void BookingManager::loadBookings()
{   
  BookingHandler::loadBookings();

  if (!ui->bookingsList) return;

  ui->bookingsList->clear();

  for (qsizetype i = 0; i < bookings.size(); ++i) {
    QString entryText = BookingHandler::makeEntry(bookings[i]);
    QListWidgetItem* item = new QListWidgetItem(entryText);
    
    if (bookings[i].isConflicting) {
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

  BookingHandler::reevaluateConflicts();
  loadBookings();

  ui->infoLabel->setMaximumWidth(0);
  infoLabelVisible = false;
}

void BookingManager::updateTexts()
{
  ui->newBookingButton->setText(TextHandler::getText(ID::MANAGER_NEW));
  ui->editBookingButton->setText(TextHandler::getText(ID::MANAGER_EDIT));
  ui->deleteBookingButton->setText(TextHandler::getText(ID::MANAGER_DELETE));
  ui->toPTZControlsButton->setText(TextHandler::getText(ID::MANAGER_TO_PTZ));
  ui->toModeSelectButton->setText(TextHandler::getText(ID::MANAGER_BACK));

  // ui->infoLabel->setFont(QFont("DaxOT", 11));
  ui->infoLabel->setText(TextHandler::getText(ID::MANAGER_INFO));
}

void BookingManager::on_bookingsList_currentRowChanged()
{
  
}

void BookingManager::on_infoButton_pressed()
{
  infoLabelAnimation->stop();

  if (infoLabelVisible) {
      infoLabelAnimation->setStartValue(420);
      infoLabelAnimation->setEndValue(0);
      infoLabelVisible = false;

      ui->infoButton->setStyleSheet(
        "QPushButton { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
        "QPushButton:hover { background-color: rgb(42, 130, 218); }"
        "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
      );
  } 
  
  else {
      // ui->infoLabel->setMaximumWidth(420);
      infoLabelAnimation->setStartValue(0);
      infoLabelAnimation->setEndValue(420);
      infoLabelVisible = true;

      ui->infoButton->setStyleSheet(
        "QPushButton { background-color: rgb(42, 130, 218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
        "QPushButton:hover { background-color: rgb(42, 130, 218); }"
        "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
      );
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

  else if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_EDIT_TOO_MANY_SELECTED)
    );

    return;
  }

  currentRow = ui->bookingsList->currentRow();
  Widgets::bookingEditor->reload(&bookings[currentRow]);
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
  
  int rowIndex = ui->bookingsList->currentRow();
  Booking& selectedBooking = bookings[rowIndex];
  BookingHandler::updateConflictingBookings(selectedBooking, false);
  JsonParser::removeBooking(selectedBooking);

  loadBookings();
}

void BookingManager::on_toPTZControlsButton_clicked()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_TO_PTZ_NONE_SELECTED)
    );

    return;
  }

  else if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::MANAGER_TO_PTZ_TOO_MANY_SELECTED)
    );

    return;
  }

  BookingHandler::currentBooking = bookings[currentRow];

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

