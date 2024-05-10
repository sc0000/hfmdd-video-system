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

  setModal(false);
  hide();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::reload()
{
  raise();
  center(ui->masterWidget);

  Backend::reevaluateConflicts();
  loadBookings();
  
}

void BookingManager::loadBookings()
{   
  Backend::loadBookings();

  if (!ui->bookingsList) return;

  ui->bookingsList->clear();

  for (qsizetype i = 0; i < bookings.size(); ++i) {
    QString entryText = makeEntry(bookings[i]);
    QListWidgetItem* item = new QListWidgetItem(entryText);
    
    if (bookings[i].isConflicting)
      item->setBackground(Qt::red);

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
    break;

    case ELanguage::English:
    ui->newBookingButton->setText("New Booking");
    ui->editBookingButton->setText("Edit Booking");
    ui->deleteBookingButton->setText("Delete Booking");
    ui->toPTZControlsButton->setText("Go to Camera Controls");
    ui->toModeSelectButton->setText("Go back to Mode Selection");
    break;

    case ELanguage::Default:
    break;
  }
}

QString BookingManager::makeEntry(const Booking& booking)
{
  QString entry = 
      booking.date.toString("ddd MMM dd yyyy") + "\t" +
      booking.startTime.toString("HH:mm") + " - " +
      booking.stopTime.toString("HH:mm") + "\t" +
      booking.event.leftJustified(20, ' ') + "\t" +
      booking.email.leftJustified(20, ' ') +
      (booking.isConflicting ? "\t--CONFLICTING!" : "");

  return entry;
}

void BookingManager::on_newBookingButton_pressed()
{
  // BookingEditor::instance(nullptr, this);
  Widgets::bookingEditor->reload();
}

void BookingManager::on_editBookingButton_pressed()
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

void BookingManager::on_deleteBookingButton_pressed()
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

  bool confirmed;
  Widgets::okCancelDialog->display(
    Backend::language != ELanguage::German ?
    "Do you really want to delete the selected booking? This cannot be undone." :
    "Wollen Sie diese Buchung wirklich löschen? Das kann nicht rückgängig gemacht werden.", 
    confirmed
  );

  if (!confirmed) return;

  int rowIndex = ui->bookingsList->currentRow();
  const Booking& selectedBooking = bookings[rowIndex];
  JsonParser::removeBooking(selectedBooking);
  bookings.removeAt(rowIndex);
  QListWidgetItem* item = ui->bookingsList->takeItem(rowIndex);
  if (item) delete item;
}

void BookingManager::on_toPTZControlsButton_pressed()
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
  SettingsManager::outerDirectory = Backend::currentBooking.date.toString(Qt::ISODate) + "/";

  // TODO: Check: always reset or update, and setup option to reset manually?
  SettingsManager::resetFilterSettings();

  Widgets::ptzControls->reload();

  Widgets::showFullScreenDialogs(false);
}

void BookingManager::on_toModeSelectButton_pressed()
{
  Widgets::showFullScreenDialogs(true);
  fade(Widgets::modeSelect);
}

void BookingManager::on_logoutButton_pressed()
{
  Widgets::showFullScreenDialogs(true);
  fade(Widgets::loginDialog);
}

