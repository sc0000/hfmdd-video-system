/* Custom handlebar, fade animations for animated dialog boxes
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <QLabel>

#include "styles.hpp"
#include "animated-dialog.hpp"

Handlebar::Handlebar(QWidget* parent, EHandlebarStyle HandlebarStyle, const QString& title) 
  : QFrame(parent),
    animatedDialogParent(static_cast<AnimatedDialog*>(parent)) 
{
  setFixedHeight(40);
  
  if (parent)
    setFixedWidth(parent->width());

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  
  move(QPoint(0, 0));

  if (title != "") {
    QLabel* titleLabel = new QLabel(this);

    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLabel->setFixedHeight(24);
    titleLabel->move(12, 8);
    titleLabel->setText(title);

    titleLabel->setStyleSheet(
        QString("QLabel { color: %1; border: 0px; font: 12pt DaxOT; }").arg(Color::white)
      );
  }

  closeButton = new QPushButton(this);

  closeButton->setFixedSize(24, 24);
  closeButton->move(width() - 34, 8);
  closeButton->setText("x");

  if (HandlebarStyle == EHandlebarStyle::Black) {
    setStyleSheet(
      QString("QWidget { background-color: %1; }").arg(Color::black)
    );

    closeButton->setStyleSheet(Styles::blackButtonYellowHover);
  }

  else if (HandlebarStyle == EHandlebarStyle::Blue) {
    setStyleSheet(
      QString("QWidget { background-color: %1; }").arg(Color::blue)
    );

    closeButton->setStyleSheet(Styles::blueButton);
  }

  QObject::connect(closeButton, &QPushButton::clicked, this, &Handlebar::onCloseButtonClicked);

  show();
}

void Handlebar::mousePressEvent(QMouseEvent *event) 
{
  if (event->button() == Qt::LeftButton) {
      dragStartPosition = event->globalPosition().toPoint();
      dragStartParentPosition = parentWidget()->pos();
  }
}

void Handlebar::mouseMoveEvent(QMouseEvent *event) 
{
  if (event->buttons() & Qt::LeftButton) {
      const QPoint globalMousePos = event->globalPosition().toPoint();
      const QPoint diff = globalMousePos - dragStartPosition;
      parentWidget()->move(dragStartParentPosition + diff);
  }
}

void Handlebar::onCloseButtonClicked()
{
  if (animatedDialogParent)
    animatedDialogParent->fade();
}

//----------------------------------------------------

AnimatedDialog::AnimatedDialog(QWidget* parent)
  : QDialog(parent),
    visible(false),
    fadeAnimation(new QPropertyAnimation(this, "windowOpacity")),
    sendResultCode(nullptr)
{
  fadeAnimation->setDuration(100);
  connect(fadeAnimation, &QPropertyAnimation::finished, this, &AnimatedDialog::onFinished);
}

void AnimatedDialog::onFinished()
{
  if (!visible) {
    hide();
    
    if (sendResultCode)
      sendResultCode();

    else QDialog::reject();
  }
}

void AnimatedDialog::fade(void (*result)(void))
{
  fadeAnimation->stop();

  if (visible) {
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
  }

  else {
    show();
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
  }

  visible = !visible;
  sendResultCode = result;

  fadeAnimation->start();
}

void AnimatedDialog::setFadeDuration(int fadeDuration)
{
  if (!fadeAnimation) return;

  fadeAnimation->setDuration(fadeDuration);
}
