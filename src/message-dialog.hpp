#include <QDialog>
#include <QString>
#include <QWidget>


class OkDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OkDialog(const QString& message, QWidget* parent = nullptr);
  static void instance(const QString& message, QWidget* parent = nullptr);
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

public:
  explicit OkCancelDialog(const QString& message, bool& out, QWidget* parent = nullptr);
  static void instance(const QString& message, bool& out, QWidget* parent = nullptr);

private:
  bool& decision;  

private slots:
  void on_okButton_pressed();
  void on_cancelButton_pressed();
};