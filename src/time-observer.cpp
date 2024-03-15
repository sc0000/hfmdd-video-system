#include <QTimer>
#include <QDateTime>

#include "time-observer.hpp"

TimeObserver::TimeObserver(QDateTime threshold, void (*function)(), QObject* parent)
  : QObject(parent),
    m_threshold(threshold),
    m_function(function)
{
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &TimeObserver::checkTime);
  timer->start(30000); // check every 30sec
}

void TimeObserver::checkTime()
{
  QDateTime currentTime = QDateTime::currentDateTime();

  if (currentTime > m_threshold) {
    if (m_function)
      m_function();
  }
}