#include "language.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "mode-select.hpp"
#include "booking-editor.hpp"
#include "booking-manager.hpp"
#include "quick-record.hpp"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "widgets.hpp"

Login* Widgets::login; // TODO: Rename!
LoginDialog* Widgets::loginDialog;
ModeSelect* Widgets::modeSelect;
BookingEditor* Widgets::bookingEditor;
BookingManager*Widgets::bookingManager;
QuickRecord* Widgets::quickRecord;

QVector<FullScreenDialog*> Widgets::fullScreenDialogs;
QVector<Translatable*> Widgets::translatables;

PTZControls* Widgets::ptzControls;
OkDialog* Widgets::okDialog;
OkCancelDialog* Widgets::okCancelDialog;
PasswordDialog* Widgets::passwordDialog;
PresetDialog* Widgets::presetDialog;

void Widgets::load()
{
  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  login = new Login(mainWindow); // TODO: Rename
  loginDialog = new LoginDialog(mainWindow);
  modeSelect = new ModeSelect(mainWindow);
  bookingEditor = new BookingEditor(mainWindow);
  bookingManager = new BookingManager(mainWindow);
  quickRecord = new QuickRecord(mainWindow);
  ptzControls = new PTZControls(mainWindow);

  okDialog = new OkDialog(mainWindow);
  okCancelDialog = new OkCancelDialog(mainWindow);
  passwordDialog = new PasswordDialog(mainWindow);
  presetDialog = new PresetDialog(mainWindow);

  assert(
    login &&
    loginDialog &&
    modeSelect &&
    bookingEditor &&
    bookingManager &&
    quickRecord &&
    ptzControls &&
    okDialog &&
    okCancelDialog &&
    passwordDialog &&
    presetDialog
  );

  fullScreenDialogs = {
    modeSelect, loginDialog, quickRecord, bookingManager
  };

  translatables = {
    login, loginDialog, modeSelect, bookingEditor, bookingManager, quickRecord, 
    ptzControls
  };
}

void Widgets::showFullScreenDialogs(bool show)
{
  for (FullScreenDialog* fsd : fullScreenDialogs) {
    if (show) 
      fsd->showFullScreen();
    else 
      fsd->hide();
  }
}