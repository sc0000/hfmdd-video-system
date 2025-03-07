/* Full screen widgets to choose between recording immediately
 * or employing the booking system
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>

#include "text-handler.hpp"
#include "fullscreen-dialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class ModeSelect;
}
QT_END_NAMESPACE

class ModeSelect : public FullScreenDialog, public Translatable
{
  Q_OBJECT
private:
  Ui::ModeSelect* ui;

private slots:
  void on_quickModeButton_clicked();
  void on_bookingModeButton_clicked();
    
public:
  ModeSelect(QWidget *parent = nullptr);
  ~ModeSelect();

  virtual void reload() override;
  virtual void updateTexts() override; 
};
