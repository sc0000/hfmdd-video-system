#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "quick-record.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "ui_mode-select.h"
#include "mode-select.hpp"

ModeSelect* ModeSelect::instance = NULL;

ModeSelect::ModeSelect(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::ModeSelect)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Mode Select");

  instance = this;
  ui->setupUi(this);

  repositionMasterWidget();

  showFullScreen();
}

ModeSelect::~ModeSelect()
{
    delete ui;
}

void ModeSelect::reload()
{
  repositionMasterWidget();
  show();
}

void ModeSelect::repositionMasterWidget()
{
  Globals::centerFullScreenWidget(ui->masterWidget);
}

void ModeSelect::on_quickModeButton_pressed()
{
  // if (!BookingManager::getInstance()) {
  //   BookingManager* bookingManager = new BookingManager(this);

  //   if (!bookingManager) return;

  //   bookingManager->exec();
  //   bookingManager->hide();
  // }

  Globals::mode = EMode::QuickMode;

  QuickRecord* quickRecord = QuickRecord::getInstance();

  if (quickRecord) {
    quickRecord->loadBookings();
    quickRecord->show();
  }


  else {
    quickRecord = new QuickRecord(this);

    if (!quickRecord) return;

    quickRecord->exec();
  }

  hide();
}

void ModeSelect::on_bookModeButton_pressed()
{
  Globals::mode = EMode::BookMode;

  BookingManager* bookingManager = BookingManager::getInstance();

  if (bookingManager) {
    bookingManager->loadBookings();
    bookingManager->show();
  }


  else {
    bookingManager = new BookingManager(this);

    if (!bookingManager) return;

    bookingManager->exec();
  }

  hide();
}  