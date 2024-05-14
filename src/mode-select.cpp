#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>

#include "backend.hpp"
#include "widgets.hpp"
#include "login-dialog.hpp"
#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "quick-record.hpp"
#include "settings-manager.hpp"
#include "ui_mode-select.h"
#include "mode-select.hpp"

ModeSelect* ModeSelect::instance = NULL;

ModeSelect::ModeSelect(QWidget *parent)
  : FullScreenDialog(parent),
    ui(new Ui::ModeSelect)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Mode Select");

  instance = this;
  ui->setupUi(this);
  
  center(ui->masterWidget);

  // ui->quickModeButton->setStyleSheet("QPushButton { border-radius: none; }");
  ui->bookModeButton->setStyleSheet("QPushButton:hover { background-color: rgb(42,130,218); }");

  setModal(false);
  hide();
}

ModeSelect::~ModeSelect()
{
    delete ui;
}

void ModeSelect::reload()
{
  raise();
  center(ui->masterWidget);
}

void ModeSelect::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    ui->quickModeButton->setText("Direkt ein Video aufnehmen");
    ui->bookModeButton->setText("Eine Aufnahmesession buchen");
    break;

    case ELanguage::English:
    ui->quickModeButton->setText("Record a video right now");
    ui->bookModeButton->setText("Book a video recording session in the future");
    break;

    case ELanguage::Default:
    break;
  }
}

void ModeSelect::on_quickModeButton_pressed()
{
  Backend::mode = EMode::QuickMode;
  fade(Widgets::quickRecord);
}

void ModeSelect::on_bookModeButton_pressed()
{
  Backend::mode = EMode::BookMode;
  fade(Widgets::bookingManager);
}  