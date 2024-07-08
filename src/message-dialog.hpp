#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLineEdit>

#include "language.hpp"
#include "booking.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class OkDialog;
}
QT_END_NAMESPACE

class OkDialog : public QDialog
{
  Q_OBJECT

private:
  Ui::OkDialog* ui;
  bool m_triggerLogout = false;

public:
  explicit OkDialog(QWidget* parent = nullptr);
  void display(const QString& message, bool triggerLogout = false);

private slots:
  void on_okButton_pressed();
};

QT_BEGIN_NAMESPACE
namespace Ui {
class OkCancelDialog;
}
QT_END_NAMESPACE

class OkCancelDialog : public QDialog
{
  Q_OBJECT

private:
  Ui::OkCancelDialog* ui;
  bool decision;  

public:
  explicit OkCancelDialog(QWidget* parent = nullptr);
  void display(const QString& message, bool& out);

private slots:
  void on_okButton_pressed();
  void on_cancelButton_pressed();
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PasswordDialog;
}
QT_END_NAMESPACE

class PasswordDialog : public QDialog
{
  Q_OBJECT

private:
  Ui::PasswordDialog* ui;
  bool valid;

public:
  explicit PasswordDialog(QWidget* parent = nullptr);
  void display();
  inline bool isValid() { return valid; } 

private slots:
  void on_okButton_pressed();
  void on_cancelButton_pressed();  
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PresetDialog;
}
QT_END_NAMESPACE

class PresetDialog : public QDialog
{
  Q_OBJECT

private:
  Ui::PresetDialog* ui;

public:
  explicit PresetDialog(QWidget* parent = nullptr);
  void display(Booking* booking = nullptr); 

private slots:
  void on_okButton_pressed();
  void on_cancelButton_pressed();  
};