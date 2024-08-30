#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>

#include "booking-handler.hpp"
#include "mail-handler.hpp"
#include "widgets.hpp"
#include "login-dialog.hpp"
#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "quick-record.hpp"
#include "json-parser.hpp"
#include "ui_mode-select.h"
#include "mode-select.hpp"

ModeSelect::ModeSelect(QWidget *parent)
  : FullScreenDialog(parent),
    ui(new Ui::ModeSelect)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Mode Select");

  ui->setupUi(this);
  
  center(ui->masterWidget);

  ui->bookingModeButton->setStyleSheet("QPushButton:hover { background-color: rgb(42, 130, 218); }"
                                       "QPushButton:pressed  { background-color: rgb(31, 30, 31); }");

  updateTexts();
  setModal(false);
  hide();
}

ModeSelect::~ModeSelect()
{
    delete ui;
}

void ModeSelect::reload()
{
  if (!MailHandler::mailAddressIsValid) {
    Widgets::loginDialog->reload();
    return;
  }
  
  raise();
  center(ui->masterWidget);
}

void ModeSelect::updateTexts()
{
  ui->quickModeButton->setText(TextHandler::getText(ID::MODE_QUICK));
  ui->bookingModeButton->setText(TextHandler::getText(ID::MODE_BOOKING));
}

void ModeSelect::on_quickModeButton_clicked()
{
  BookingHandler::mode = EMode::QuickMode;
  fade(Widgets::quickRecord);
}

void ModeSelect::on_bookingModeButton_clicked()
{
  BookingHandler::mode = EMode::BookMode;
  fade(Widgets::bookingManager);
}  