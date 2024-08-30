#include <QVector>

#include "fullscreen-dialog.hpp"
#include "text-handler.hpp"

class Widgets
{
public:
  static void load();
  
  static class Login* login; // TODO: Rename!
  static class LoginDialog* loginDialog;
  static class ModeSelect* modeSelect;
  static class PTZControls* ptzControls;
  static class BookingEditor* bookingEditor;
  static class BookingManager* bookingManager;
  static class QuickRecord* quickRecord;

  static class OkDialog* okDialog;
  static class OkCancelDialog* okCancelDialog;
  static class PasswordDialog* passwordDialog;
  static class PresetDialog* presetDialog;
  static class InfoDialog* infoDialog;

  static QVector<FullScreenDialog*> fullScreenDialogs;
  static QVector<Translatable*> translatables;

  static void showFullScreenDialogs(bool show);
};