#pragma once

#include <QDialog>
#include <QPushButton>
#include <QFrame>
#include <QMouseEvent>
#include <QPropertyAnimation>

class Handlebar : public QFrame
{
  Q_OBJECT

public:
  explicit Handlebar(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
  class AnimatedDialog* animatedDialogParent;
  QPushButton* closeButton;

  QPoint dragStartPosition;
  QPoint dragStartParentPosition;

  void onCloseButtonClicked();
};

//----------------------------------------------------

class AnimatedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AnimatedDialog(QWidget* parent = nullptr);

private:
  bool visible;
  QPropertyAnimation* fadeAnimation;
  int fadeDuration;

private slots:
  void onFinished();

public:
  void fade(void (*result)(void) = nullptr);
  void setFadeDuration(int fadeDuration);
  void (* sendResultCode)(void);
  // inline bool isVisible() { return visible; }
};