#include <QLabel>
#include <QScreen>

#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"


BookingManager* BookingManager::instance = nullptr;

BookingManager::BookingManager(QWidget* parent)
  : QDialog(parent), 
    ui(new Ui::BookingManager)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  
  setWindowTitle("Booking Manager");
  instance = this;
  ui->setupUi(this);
  repositionMasterWidget();

  showFullScreen();
  
  loadBookings();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::reload()
{
  repositionMasterWidget();
  show();
}

void BookingManager::repositionMasterWidget()
{
  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return;

  QRect screenGeometry = mainWindow->screen()->geometry();

  int screenWidth = screenGeometry.width();
  int screenHeight = screenGeometry.height();

  QPoint centerPoint(screenWidth / 2, screenHeight / 2);

  int masterWidgetX = centerPoint.x() - (ui->masterWidget->width() / 2);
  int masterWidgetY = centerPoint.y() - (ui->masterWidget->height() / 2);

  move(mainWindow->pos());
  ui->masterWidget->move(masterWidgetX, masterWidgetY);
}

void BookingManager::loadBookings()
{   
  QVector<Booking> tmp;

  if (Globals::currentEmail == Globals::oliversEmail)
    JsonParser::getAllBookings(tmp);

  else   
    JsonParser::getBookingsForEmail(Globals::currentEmail, tmp);

  bookings.clear();  

  for (const Booking& b : tmp) {
   bookings.append(b);
  }

  sortBookings();

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

void BookingManager::sortBookings()
{
  qsizetype size = bookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = bookings[j].date.toString(Qt::ISODate) + "_" +
        bookings[j].startTime.toString("HH:mm");

      QString dateTime1 = bookings[j + 1].date.toString(Qt::ISODate) + "_" +
        bookings[j + 1].startTime.toString("HH:mm");

      if (dateTime0 > dateTime1) {
        Booking tmp = bookings[j];
        bookings[j] = bookings[j + 1];
        bookings[j + 1] = tmp;

        flag = true;
      }
    }

    if (flag == 0) break;
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
  BookingEditor::instance(nullptr, this);
}

void BookingManager::on_editBookingButton_pressed()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    OkDialog::instance("Please select the booking you want to edit.", this);
    return;
  }

  else if (ui->bookingsList->selectedItems().size() > 1) {
    OkDialog::instance("Please select only one booking to edit.", this);
    return;
  }

  BookingEditor::instance(&bookings[ui->bookingsList->currentRow()], this);
}

void BookingManager::on_deleteBookingButton_pressed()
{
  if (ui->bookingsList->selectedItems().isEmpty()) {
    OkDialog::instance("Please select the booking you want to delete.", this);
    return;
  }

  // TODO: Select more than one booking for deletion?

  bool confirmed;
  OkCancelDialog::instance(
    "Do you really want to delete the selected booking? This cannot be undone.", 
    confirmed, this
  );

  if (!confirmed) return;

  int rowIndex = ui->bookingsList->currentRow();
  const Booking& selectedBooking = bookings[rowIndex];
  JsonParser::removeBooking(selectedBooking);
  bookings.removeAt(rowIndex);
  QListWidgetItem* item = ui->bookingsList->takeItem(rowIndex);
  if (item) delete item;
}

void BookingManager::on_logoutButton_pressed()
{
  LoginDialog* loginDialog = LoginDialog::getInstance();
  
  if (!loginDialog) return;

  loginDialog->reload();
  hide();
}

void BookingManager::on_toPTZControlsButton_pressed()
{
   if (ui->bookingsList->selectedItems().isEmpty()) {
    OkDialog::instance("Please select a booking to continue.", this);
    return;
  }

  else if (ui->bookingsList->selectedItems().size() > 1) {
    OkDialog::instance("Please select only one booking to continue.", this);
    return;
  }

  selectedBooking = &bookings[ui->bookingsList->currentRow()];
  PathManager::outerDirectory = selectedBooking->date.toString(Qt::ISODate) + "/";

  // TODO: Check: always reset or update, and setup option to reset manually?
  PathManager::resetFilterSettings();

  hide();

  Login::getInstance()->hide();
  LoginDialog::getInstance()->hide();

  PTZControls* ptzControls = PTZControls::getInstance();

  if (!ptzControls) return;

  ptzControls->connectSignalItemSelect();
  ptzControls->loadUserPresets();
  ptzControls->setViewportMode();
  ptzControls->selectCamera();
  ptzControls->setFloating(false);
  ptzControls->show();
}