/* Some utility code for all full screen widgets, incl. (minimal) animations
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include <QDialog>
#include <QWidget>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QPropertyAnimation>

class FullScreenDialog : public QDialog
{
public:
  FullScreenDialog(QWidget* parent);

  static void loadAnims();
  
protected:
  void center(QWidget* masterWidget, int xOffset = 0, int yOffset = 0);
  void fade(FullScreenDialog* next);
  virtual void reload() = 0;

  virtual void keyPressEvent(QKeyEvent* e) override;
  virtual void closeEvent(QCloseEvent* e) override;

private:
  static QPropertyAnimation* fadeInAnim;
  static QPropertyAnimation* fadeOutAnim;
};
