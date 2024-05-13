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
  void center(QWidget* masterWidget, int xOffset = 0, int yOffset = 0);
  void fade(FullScreenDialog* next);
  virtual void reload() = 0;

private:
  static QPropertyAnimation* fadeInAnim;
  static QPropertyAnimation* fadeOutAnim;
};