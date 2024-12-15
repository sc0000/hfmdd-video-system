/* Full screen login widget 
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPaintEvent>

#include "fullscreen-dialog.hpp"
#include "text-handler.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public FullScreenDialog, public Translatable
{
  Q_OBJECT
private:
  Ui::LoginDialog* ui;

private:
  bool verifyMailAddress();

  QString reminderLabelText;
  QString infoDialogText;

private slots:
  void on_mailAddressLineEdit_textChanged(const QString& text);
  void on_manageBookingsButton_clicked();
  void on_languageComboBox_currentTextChanged(const QString& text);
  void on_infoButton_pressed();
    
public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  virtual void reload() override;
  virtual void updateTexts() override; 
};
