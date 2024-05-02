#include <QRect>
#include <QScreen>

#include <obs-frontend-api.h>

#include "fullscreen-dialog.hpp"

QPropertyAnimation* FullScreenDialog::fadeInAnim = new QPropertyAnimation(nullptr, "windowOpacity");
QPropertyAnimation* FullScreenDialog::fadeOutAnim = new QPropertyAnimation(nullptr, "windowOpacity");

FullScreenDialog::FullScreenDialog(QWidget* parent)
  : QDialog(parent)
{
  hide();
}

void FullScreenDialog::loadAnims()
{
  assert(fadeInAnim && fadeOutAnim);

  fadeInAnim->setDuration(10);
  fadeInAnim->setStartValue(0.f);
  fadeInAnim->setEndValue(1.f);

  fadeOutAnim->setDuration(200);
  fadeOutAnim->setStartValue(1.f);
  fadeOutAnim->setEndValue(0.f);
}

void FullScreenDialog::center(QWidget* masterWidget)
{
  QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return;

  QRect screenGeometry = mainWindow->screen()->geometry();

  int screenWidth = screenGeometry.width();
  int screenHeight = screenGeometry.height();

  QPoint centerPoint(screenWidth / 2, screenHeight / 2);

  int masterWidgetX = centerPoint.x() - (masterWidget->width() / 2);
  int masterWidgetY = centerPoint.y() - (masterWidget->height() / 2);

  masterWidget->move(masterWidgetX, masterWidgetY);
}

void FullScreenDialog::fade(FullScreenDialog* next)
{
  next->reload();
  raise();
  
  fadeOutAnim->setTargetObject(this);
  fadeInAnim->setTargetObject(next);

  fadeInAnim->start();
  fadeOutAnim->start();
}