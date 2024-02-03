#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "ui_ok-cancel-dialog.h"
#include "message-dialog.hpp"

OkDialog::OkDialog(const QString& message, QWidget* parent)
  : QDialog(parent)
{
  setWindowFlag(Qt::FramelessWindowHint);
  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* label = new QLabel(message);
  layout->addWidget(label);

  QPushButton* okButton = new QPushButton("OK");
  connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
  okButton->setMaximumWidth(60);
  layout->addWidget(okButton, 0, Qt::AlignHCenter);

  setLayout(layout);
  show();

  setFocus();
}

void OkDialog::instance(const QString& message, QWidget* parent)
{
  std::unique_ptr<OkDialog> messageBox = std::make_unique<OkDialog>(message, parent);
  messageBox->exec();
}

OkCancelDialog::OkCancelDialog(const QString& message, bool& out, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkCancelDialog), 
    decision(out)
{
  setWindowFlag(Qt::FramelessWindowHint);
  ui->setupUi(this);
  findChild<QLabel*>("messageLabel")->setText(message);
}

void OkCancelDialog::instance(const QString& message, bool& out, QWidget* parent)
{
  std::unique_ptr<OkCancelDialog> messageBox = std::make_unique<OkCancelDialog>(message, out, parent);
  messageBox->exec();
}

void OkCancelDialog::on_okButton_pressed()
{
  decision = true;
  hide();
}

void OkCancelDialog::on_cancelButton_pressed()
{
  decision = false;
  hide();
}