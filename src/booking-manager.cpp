#include <QLabel>
#include <QScreen>

#include "backend.hpp"
#include "widgets.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "settings-manager.hpp"
#include "mode-select.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"


BookingManager* BookingManager::instance = nullptr;

BookingManager::BookingManager(QWidget* parent)
  : FullScreenDialog(parent), 
    ui(new Ui::BookingManager),
    bookings(Backend::loadedBookings)
{
  setWindowFlags(
    windowFlags() | 
    Qt::MSWindowsFixedSizeDialogHint | 
    Qt::FramelessWindowHint
  );
  
  setWindowTitle("Booking Manager");
  instance = this;
  ui->setupUi(this);
  center(ui->masterWidget);

  ui->infoButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(31, 30, 31); }"
  );

  ui->infoLabel->setStyleSheet(
    "QLabel { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
  );

  ui->infoLabel->setText(
    QString("<html><head/><body>") + 
      "From this screen, you can book video recording sessions, or edit existing ones.<br/><br/>" +
      "Please note the following:<br/><br/>--You have to select a booking to continue to camera controls. " +
      "In general, you can book and work with time slots that are conflicting with others, but please be aware " +
      "that these might be subject to adjustments by the admin.<br/><br/>" +
      "--You are allowed to start a recording 15 minutes before the designated time at the earliest, however, " +
      "you can already set up your cameras and save presets before.<br/><br/>" +
      "--You can stop the recording whenever you want, but:<br/><br/>" +
      "<span style=\"font-weight: bold;\">" +
      "--The recording will stop automatically 10 minutes after the designated time!</span><br/><br/>" +
      "--No matter how the recording was stopped, you will receive a download link via email afterwards." +
      "</body></html>"
  );

  ui->bookingsList->setStyleSheet("QListWidget { border: 1px solid rgb(31, 30, 31); }");

  setModal(false);
  hide();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::reload()
{
  if (!Backend::mailAddressIsValid) {
    Widgets::loginDialog->reload();
    return;
  }

  ui->infoLabel->hide();
  
  raise();
  center(ui->masterWidget);

  Backend::reevaluateConflicts();
  loadBookings();
}

void BookingManager::toLoginDialog()
{ 
  fade(Widgets::loginDialog);
}

void BookingManager::loadBookings()
{   
  Backend::loadBookings();

  if (!ui->bookingsList) return;

  ui->bookingsList->clear();

  for (qsizetype i = 0; i < bookings.size(); ++i) {
    QString entryText = Backend::makeEntry(bookings[i]);
    QListWidgetItem* item = new QListWidgetItem(entryText);
    
    if (bookings[i].isConflicting) {
      item->setBackground(QBrush(QColor(31, 30, 31)));
      item->setForeground(QBrush(QColor(254, 253, 254)));
    }

    ui->bookingsList->addItem(item);
  }
}

void BookingManager::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    ui->newBookingButton->setText("Neue Buchung");
    ui->editBookingButton->setText("Buchung bearbeiten");
    ui->deleteBookingButton->setText("Buchung löschen");
    ui->toPTZControlsButton->setText("Zur Kamerasteuerung");
    ui->toModeSelectButton->setText("Zurück");

    ui->infoLabel->setFont(QFont("DaxOT", 11));
    ui->infoLabel->setText(
      QString("<html><head/><body>") + 
        "Auf diesem Bildschirm können Sie Videosessions buchen oder vorhandene Buchungen bearbeiten.<br/><br/>" +
        "Bitte beachten Sie folgendes:<br/><br/>--Sie müssen eine Buchung auswählen, um zur Kamerasteuerung fortzufahren. " +
        "Grundsätzlich können Sie auch Sessions buchen und nutzen, die mit anderen kollidieren. Beachten Sie jedoch, " +
        "dass diese überprüft und ggf. angepasst werden.<br/><br/>" +
        "--Sie können eine Aufnahme frühestens 15 Minuten vor der angegeben Zeit starten. Allerdings " +
        "können Sie jederzeit die Kameras konfigurieren und diese Einstellung als Preset speichern.<br/><br/>" +
        "--Sie können die Aufnahme jederzeit stoppen, aber:<br/><br/>" +
        "<span style=\"font-weight: bold;\">" +
        "--10 Minuten nach der angegeben Zeit wird die Aufnahme automatisch gestoppt!</span><br/><br/>" +
        "--Nach Ende der Aufnahme (egal ob manuell oder automatisch ausgelöst) erhalten Sie einen Downloadlink per E-Mail." +
        "</body></html>"
    );

    break;

    case ELanguage::English:
    ui->newBookingButton->setText("New Booking");
    ui->editBookingButton->setText("Edit Booking");
    ui->deleteBookingButton->setText("Delete Booking");
    ui->toPTZControlsButton->setText("Go to Camera Controls");
    ui->toModeSelectButton->setText("Go back to Mode Selection");

    ui->infoLabel->setFont(QFont("DaxOT", 12));
    ui->infoLabel->setText(
      QString("<html><head/><body>") + 
        "From this screen, you can book video recording sessions, or edit existing bookings.<br/><br/>" +
        "Please note the following:<br/><br/>--You have to select a booking to continue to camera controls. " +
        "In general, you can book and work with time slots that are conflicting with others, but please be aware " +
        "that these might be subject to adjustments by the admin.<br/><br/>" +
        "--You are allowed to start a recording 15 minutes before the designated time at the earliest, however, " +
        "you can set up your cameras and save presets anytime.<br/><br/>" +
        "--You can stop the recording whenever you want, but:<br/><br/>" +
        "<span style=\"font-weight: bold;\">" +
        "--The recording will stop automatically 10 minutes after the designated time!</span><br/><br/>" +
        "--No matter how the recording was stopped, you will receive a download link via email afterwards." +
        "</body></html>"
    );

    break;

    case ELanguage::Default:
    break;
  }
}

void BookingManager::on_bookingsList_currentRowChanged()
{
  // Backend::currentBooking = bookings[ui->bookingsList->currentRow()];
}

void BookingManager::on_infoButton_pressed()
{
  if (ui->infoLabel->isHidden()) {
    ui->infoButton->setStyleSheet(
      "QPushButton { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(42,130,218); }"
      "QPushButton:pressed { background-color: rgb(42,130,218); }"
    );

    ui->infoLabel->show();
  }
    
  else {
    ui->infoButton->setStyleSheet(
      "QPushButton { background-color: rgb(42,130,218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(31, 30, 31); }"
      "QPushButton:pressed { background-color: rgb(31, 30, 31); }"
    );

    ui->infoLabel->hide();
  }
}

void BookingManager::on_newBookingButton_clicked()
{
  Widgets::bookingEditor->reload(nullptr);
}

void BookingManager::on_editBookingButton_clicked()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ? 
      "Please select the booking you want to edit." :
      "Bitte wählen Sie eine Buchung aus, die Sie bearbeiten wollen."  
    );

    return;
  }

  else if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please select only one booking to edit." :
      "Bitte wählen Sie nur eine Buchung zur Bearbeitung aus."
    );

    return;
  }

  Widgets::bookingEditor->reload(&bookings[ui->bookingsList->currentRow()]);
}

void BookingManager::on_deleteBookingButton_clicked()
{
  if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please select the booking you want to delete." :
      "Bitte wählen Sie eine Buchung aus, die Sie löschen möchten."
    );

    return;
  }

  // TODO: Select more than one booking for deletion?

  int result = Widgets::okCancelDialog->display(
    Backend::language != ELanguage::German ?
    "Do you really want to delete the selected booking? This cannot be undone." :
    "Wollen Sie diese Buchung wirklich unwiderruflich löschen?"
  );

  if (result == QDialog::Rejected) 
    return;
  
  int rowIndex = ui->bookingsList->currentRow();
  Booking& selectedBooking = bookings[rowIndex];
  Backend::updateConflictingBookings(selectedBooking, false);
  JsonParser::removeBooking(selectedBooking);
  // bookings.removeAt(rowIndex);
  // QListWidgetItem* item = ui->bookingsList->takeItem(rowIndex);
  // if (item) delete item;

  loadBookings();
}

void BookingManager::on_toPTZControlsButton_clicked()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please select a booking to continue." :
      "Bitte wählen Sie eine Buchung aus, um fortzufahren."
    );

    return;
  }

  else if (ui->bookingsList->selectedItems().size() > 1) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please select only one booking to continue." :
      "Bitte wählen Sie nur eine Buchung aus, um fortzufahren.");
    return;
  }

  Backend::currentBooking = bookings[ui->bookingsList->currentRow()];

  // TODO: Check: always reset or update, and setup option to reset manually?
  SettingsManager::resetFilterSettings();

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

