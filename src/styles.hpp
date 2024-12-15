/* Style sheets to be used in tandem with the included OBS theme
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 * 
 * SPDX-License-Identifier: GPLv2
*/

#pragma once

#include <QString>

namespace Color {
  inline const QString white = "rgb(254, 253, 254)";
  inline const QString lightGrey = "rgb(229, 230, 230)";
  inline const QString black = "rgb(31, 30, 31)";
  inline const QString yellow = "rgb(255, 191, 0)";
  inline const QString blue = "rgb(42, 130, 218)";
  inline const QString red = "rgb(242, 26, 26)";
}

namespace Styles {
  inline const QString regularButtonLF = QString(
    "QPushButton { background-color: %1; font-size: 16px; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %3; }"
  ).arg(Color::lightGrey).arg(Color::black).arg(Color::yellow);
  
  inline const QString selectedButtonLF = QString(
    "QPushButton { background-color: %1; font-size: 16px; border: 1px solid %2; }"
  ).arg(Color::yellow).arg(Color::black);

  inline const QString blueButton = QString(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %3; }"
    "QPushButton:pressed { background-color: %2; color: %3; border: 1px solid %3; }"
  ).arg(Color::blue).arg(Color::white).arg(Color::black);

  inline const QString blackButtonYellowHover = QString(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %3; color: %1; border: 1px solid %1; }"
    "QPushButton:pressed { background-color: %1; color: %2; border: 1px solid %2; }"
  ).arg(Color::black).arg(Color::white).arg(Color::yellow);

  inline const QString blackButtonBlueHover = QString(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %3; color: %1; border: 1px solid %1; }"
    "QPushButton:pressed { background-color: %1; color: %2; border: 1px solid %2; }"
  ).arg(Color::black).arg(Color::white).arg(Color::blue);

  inline const QString infoButtonOff = QString(
    "QPushButton { background-color: %3; color: %2; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %1; }"
    "QPushButton:pressed { background-color: %1; }"
  ).arg(Color::blue).arg(Color::white).arg(Color::black);

  inline const QString infoButtonOn = QString(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; }"
    "QPushButton:hover { background-color: %1; }"
    "QPushButton:pressed { background-color: %1; }"
  ).arg(Color::blue).arg(Color::white);

  inline const QString invalidLineEdit = QString(
    "QLineEdit { border: 2px solid %1; border-radius: 0px; background-color: %2}"
  ).arg(Color::red).arg(Color::white);

  inline const QString validLineEdit = QString(
    "QLineEdit { border: 2px solid %1; border-radius: 0px; background-color: %2}"
  ).arg(Color::black).arg(Color::white);

  inline const QString blueLineEdit = QString(
    "QLineEdit { background-color: %1; color: %2; border: 1px solid %2; }"
  ).arg(Color::blue).arg(Color::white);

  inline const QString blueTextEdit = QString(
    "QTextEdit { background-color: %1; color: %2; border: 1px solid %2; }"
    "QScrollBar::handle::vertical { background-color: %2; min-height: 20px; margin: 2px; border-radius: 0px; border: none}"
    "QScrollBar::handle::horizontal { background-color: %2; min-height: 20px; margin: 2px; border-radius: 0px; border: none}"
  ).arg(Color::blue).arg(Color::white);

  inline const QString messageBox = QString(
    "QWidget { background-color: %1; color: %2; font: 12pt 'DaxOT'; border: 1px %3; }"
  ).arg(Color::blue).arg(Color::white).arg(Color::black);

  inline const QString unframedMessageBox = QString(
    "QWidget { background-color: %1; color: %2; font: 12pt 'DaxOT'; }"
  ).arg(Color::blue).arg(Color::white);

  inline const QString framedWidget = QString(
    "QWidget { border: 1px solid %1; }"
  ).arg(Color::white);

  inline const QString framedLabel = QString(
    "QLabel { border: 1px solid %1; }"
  ).arg(Color::white);

  inline const QString unframedLabel = QString(
    "QLabel { border: none; }"
  );

  inline const QString infoLabel = QString(
    "QLabel { background-color: %1; color: %2; border: 1px solid %2; }"
  ).arg(Color::blue).arg(Color::white);
}
