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

  ui->infoLabel->setText(
    QString("<html><head/><body>") +
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" + 
      "From this screen, you can book video recording sessions, or edit existing ones. " + 
      "Please note that you have to select a booking to continue to camera controls.<br/><br/>" +
      "<span style=\"font-weight: bold;\">I got a conflict warning when I made my booking..?</span><br/>"
      "In general, you can book and work with time slots that are conflicting with others, but please be aware " +
      "that these might be subject to adjustments by the admin.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Anything else I need to know?</span><br/>" +
      "You are allowed to start a recording 15 minutes before the designated time at the earliest, however, " +
      "you can set up your cameras and save presets anytime. " +
      "You can stop the recording whenever you want, but " +
      "<span style=\"font-weight: bold;\">(IMPORTANT) </span>" +
      "the recording will stop automatically 10 minutes after the designated time!<br/><br/>" +
      "<span style=\"font-weight: bold;\">And then what?</span><br/>" +
      "No matter how the recording was stopped, you will receive a download link via email afterwards." +
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
  
  raise();
  center(ui->masterWidget);

  Backend::reevaluateConflicts();
  loadBookings();

  ui->infoLabel->setMaximumWidth(0);
  infoLabelVisible = false;
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

      // ui->infoLabel->setFont(QFont("DaxOT", 11));
      ui->infoLabel->setText(
      QString("<html><head/><body>") +
        "<span style=\"font-weight: bold;\">Was kann ich hier machen?</span><br/>" + 
        "Hier können Sie Videosessions buchen oder vorhandene Buchungen bearbeiten.<br/><br/>" +
        "<span style=\"font-weight: bold;\">Mir wurde eine Warnung angezeigt, als ich meine letzte Buchung abschließen wollte...?</span><br/>"
        "Grundsätzlich können Sie auch Zeitfenster buchen und nutzen, die mit anderen kollidieren. " +
        "Beachten Sie jedoch, dass diese möglicherweise vom Administrator etwas angepasst werden. " +
        "Damit hier eine sinnvolle Entscheidung getroffen werden kann, müssen Sie die Art der Veranstaltung vermerken.<br/><br/>" +
        "<span style=\"font-weight: bold;\">Was muss ich noch wissen?</span><br/>" +
        "Sie können eine Aufnahme frühestens 15 Minuten vor der angegeben Zeit starten. Allerdings " +
        "können Sie jederzeit die Kameras konfigurieren und diese Einstellung als Preset speichern. " +
        "Sie können die Aufnahme jederzeit stoppen, aber " +
        "<span style=\"font-weight: bold;\">(WICHTIG) </span>" +
        "10 Minuten nach der angegeben Zeit wird die Aufnahme automatisch gestoppt!<br/><br/>" +
        "<span style=\"font-weight: bold;\">Was passiert danach?</span><br/>" +
        "Nach Ende der Aufnahme (egal ob manuell oder automatisch ausgelöst) erhalten Sie einen Downloadlink per E-Mail." +
        "</body></html>"
      );

      break;

    case ELanguage::English:
      ui->newBookingButton->setText("New Booking");
      ui->editBookingButton->setText("Edit Booking");
      ui->deleteBookingButton->setText("Delete Booking");
      ui->toPTZControlsButton->setText("Go to Camera Controls");
      ui->toModeSelectButton->setText("Go back to Mode Selection");

      // ui->infoLabel->setFont(QFont("DaxOT", 12));
      ui->infoLabel->setText(
        QString("<html><head/><body>") +
        "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" + 
        "From this screen, you can book video recording sessions, or edit existing ones. " + 
        "Please note that you have to select a booking to continue to camera controls.<br/><br/>" +
        "<span style=\"font-weight: bold;\">I got a conflict warning when I made my booking..?</span><br/>"
        "In general, you can book and work with time slots that are conflicting with others, but please be aware " +
        "that these might be subject to adjustments by the admin.<br/><br/>" +
        "<span style=\"font-weight: bold;\">Anything else I need to know?</span><br/>" +
        "You are allowed to start a recording 15 minutes before the designated time at the earliest, however, " +
        "you can set up your cameras and save presets anytime. " +
        "You can stop the recording whenever you want, but " +
        "<span style=\"font-weight: bold;\">(IMPORTANT) </span>" +
        "the recording will stop automatically 10 minutes after the designated time!<br/><br/>" +
        "<span style=\"font-weight: bold;\">And then what?</span><br/>" +
        "No matter how the recording was stopped, you will receive a download link via email afterwards." +
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

