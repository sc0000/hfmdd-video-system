#include <QRect>
#include <QScreen>

#include <obs-frontend-api.h>

#include "globals.hpp"

// QString Globals::currentEmail = "";
// const QString Globals::adminEmail = "oliver.fenk@hfmdd.de";
// EMode Globals::mode = EMode::Default;

void Globals::centerFullScreenWidget(QWidget* masterWidget)
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