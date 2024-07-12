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
  void on_okButton_clicked();
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
  void on_okButton_clicked();
  void on_cancelButton_clicked();  
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
  void on_okButton_clicked();
  void on_cancelButton_clicked();  
};

QT_BEGIN_NAMESPACE
namespace Ui {
class InfoDialog;
}
QT_END_NAMESPACE

class InfoDialog : public QDialog
{
  Q_OBJECT

private:
  Ui::InfoDialog* ui;
  QPushButton* button;

public:
  explicit InfoDialog(QWidget* parent = nullptr);
  void display(const QString& message, QPushButton* activatingButton = nullptr, const int offsetX = 0, const int offsetY = 0);

private slots:
  void on_closeButton_clicked();
};