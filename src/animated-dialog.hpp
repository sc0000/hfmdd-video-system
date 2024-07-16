#pragma once

#include <QDialog>
#include <QPropertyAnimation>

class AnimatedDialog : public QDialog
{
public:
  explicit AnimatedDialog(QWidget* parent = nullptr);

private:
  bool visible;
  QPropertyAnimation* fadeAnimation;
  int fadeDuration;

  void (* m_sendResultCode)(void);

private slots:
  void onFinished();

public:
  void fade(void (*sendResultCode)(void) = nullptr);
  void setFadeDuration(int fadeDuration);
  // inline bool isVisible() { return visible; }
};