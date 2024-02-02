#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "message-box.hpp"

MessageBox::MessageBox(const QString& message, QWidget* parent)
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

void MessageBox::dInstance(const QString& message, QWidget* parent)
{
  std::unique_ptr<MessageBox> messageBox = std::make_unique<MessageBox>(message, parent);
  messageBox->exec();
}