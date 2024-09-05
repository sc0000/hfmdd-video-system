#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLineEdit>
#include <QPropertyAnimation>

#include "animated-dialog.hpp"
#include "booking-handler.hpp"
#include "text-handler.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class OkDialog;
}
QT_END_NAMESPACE

class OkDialog : public AnimatedDialog
{
  Q_OBJECT

private:
  Ui::OkDialog* ui;
  bool m_triggerLogout = false;

public:
  explicit OkDialog(QWidget* parent = nullptr);
  void display(const QString& message, bool triggerLogout = false);

private slots:
  void on_okButton_clicked();
};

QT_BEGIN_NAMESPACE
namespace Ui {
class OkCancelDialog;
}
QT_END_NAMESPACE

class OkCancelDialog : public AnimatedDialog
{
  Q_OBJECT

private:
  Ui::OkCancelDialog* ui;

public:
  explicit OkCancelDialog(QWidget* parent = nullptr);
  int display(const QString& message, const bool colorBlack = false);

private slots:
  void on_okButton_clicked();
  void on_cancelButton_clicked();
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PasswordDialog;
}
QT_END_NAMESPACE

class PasswordDialog : public AnimatedDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::PasswordDialog* ui;
  bool valid;

public:
  explicit PasswordDialog(QWidget* parent = nullptr);
  int display();
  virtual void updateTexts() override;
  inline bool isValid() { return valid; } 

private slots:
  void on_okButton_clicked();
  void on_cancelButton_clicked();  
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PresetDialog;
}
QT_END_NAMESPACE

class PresetDialog : public AnimatedDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::PresetDialog* ui;

public:
  explicit PresetDialog(QWidget* parent = nullptr);
  void display(Booking* booking = nullptr);
  virtual void updateTexts() override;
  
private slots:
  void on_okButton_clicked();
  void on_cancelButton_clicked();  
};

QT_BEGIN_NAMESPACE
namespace Ui {
class InfoDialog;
}
QT_END_NAMESPACE

class InfoDialog : public AnimatedDialog
{
  Q_OBJECT

private:
  Ui::InfoDialog* ui;
  QPushButton* button;
  Handlebar* handlebar;

public:
  explicit InfoDialog(QWidget* parent = nullptr);
  void display(const QString& message, QPushButton* activatingButton = nullptr, const int offsetX = 0, const int offsetY = 0);
  virtual void fade(void (*result)(void) = nullptr) override;
};

enum class EAdminMailType 
{
  Adjustment,
  Deletion,
  Default
};

QT_BEGIN_NAMESPACE
namespace Ui {
class AdminMailDialog;
}
QT_END_NAMESPACE

class AdminMailDialog : public AnimatedDialog, public Translatable
{
  Q_OBJECT

private:
  Ui::AdminMailDialog* ui;
  Handlebar* handlebar;
  const Booking* m_booking;
  const Booking* m_originalEditedBooking;
  QLocale locale;
  EAdminMailType m_type;

  void updateTemplate(const QString& language);

public:
  explicit AdminMailDialog(QWidget* parent = nullptr);
  void display(EAdminMailType type, const Booking* booking, const Booking* originalEditedBooking = nullptr);
  virtual void updateTexts() override; 

private slots:
  void on_okButton_clicked();
  void on_cancelButton_clicked();
  void on_languageButton_clicked();  
};