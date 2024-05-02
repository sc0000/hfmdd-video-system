#pragma once

#include <QDialog>
#include <QWidget>
#include <QPropertyAnimation>

class FullScreenDialog : public QDialog
{
public:
  FullScreenDialog(QWidget* parent);

  static void loadAnims();
  
protected:
  void center(QWidget* masterWidget);
  void fade(FullScreenDialog* next);
  virtual void reload() = 0;

private:
  static QPropertyAnimation* fadeInAnim;
  static QPropertyAnimation* fadeOutAnim;
};