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