#include "animated-dialog.hpp"

AnimatedDialog::AnimatedDialog(QWidget* parent)
  : QDialog(parent),
    visible(false),
    fadeAnimation(new QPropertyAnimation(this, "windowOpacity")),
    m_sendResultCode(nullptr)
{
  fadeAnimation->setDuration(100);
  connect(fadeAnimation, &QPropertyAnimation::finished, this, &AnimatedDialog::onFinished);
}

void AnimatedDialog::onFinished()
{
  if (!visible) {
    hide();
    
    if (m_sendResultCode)
      m_sendResultCode();
  }
}

void AnimatedDialog::fade(void (*sendResultCode)(void))
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
  m_sendResultCode = sendResultCode;

  fadeAnimation->start();
}

void AnimatedDialog::setFadeDuration(int fadeDuration)
{
  if (!fadeAnimation) return;

  fadeAnimation->setDuration(fadeDuration);
}