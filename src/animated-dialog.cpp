#include "animated-dialog.hpp"

AnimatedDialog::AnimatedDialog(QWidget* parent)
  : QDialog(parent),
    visible(false),
    fadeAnimation(new QPropertyAnimation(this, "windowOpacity"))
{
  fadeAnimation->setDuration(100);
}

void AnimatedDialog::fade()
{
  fadeAnimation->stop();

  if (visible) {
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
  }

  else {
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
  }

  visible = !visible;

  fadeAnimation->start();
}

void AnimatedDialog::setFadeDuration(int fadeDuration)
{
  if (!fadeAnimation) return;

  fadeAnimation->setDuration(fadeDuration);
}