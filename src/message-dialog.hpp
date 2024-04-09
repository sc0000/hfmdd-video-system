#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLineEdit>

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

public:
  explicit OkDialog(const QString& message, QWidget* parent = nullptr);
  static void instance(const QString& message, QWidget* parent = nullptr);
  
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
  bool& decision;  

public:
  explicit OkCancelDialog(const QString& message, bool& out, QWidget* parent = nullptr);
  static void instance(const QString& message, bool& out, QWidget* parent = nullptr);

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
  bool& valid;

public:
  explicit PasswordDialog(bool& out, QWidget* parent = nullptr);
  static void instance(bool& out, QWidget* parent = nullptr);

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
  explicit PresetDialog(struct Booking* booking = nullptr, QWidget* parent = nullptr);
  static void instance(Booking* booking = nullptr, QWidget* parent = nullptr);

private slots:
  void on_okButton_pressed();
  void on_cancelButton_pressed();  
};