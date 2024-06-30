#include <QTimer>
#include <QDateTime>

#include "time-observer.hpp"

TimeObserver::TimeObserver(QDateTime threshold, void (*function)(), QObject* parent)
  : QObject(parent),
    m_threshold(threshold),
    m_function(function),
    m_timer(new QTimer (this))
{
  if (!m_timer) return;

  connect(m_timer, &QTimer::timeout, this, &TimeObserver::checkTime);
}

TimeObserver::~TimeObserver()
{
  delete(m_timer);
}

void TimeObserver::start()
{
  m_timer->start(60000);
}

void TimeObserver::stop()
{
  m_timer->stop();
}

void TimeObserver::checkTime()
{
  QDateTime currentTime = QDateTime::currentDateTime();

  if (currentTime > m_threshold && m_function)
      m_function();
}