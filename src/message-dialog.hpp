#include <QDialog>
#include <QString>
#include <QWidget>

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