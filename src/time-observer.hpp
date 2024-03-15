#pragma once

#include <QDateTime>

class TimeObserver : public QObject {
  Q_OBJECT

public:
  TimeObserver(QDateTime threshold, void (*function)(), QObject* parent = nullptr);

private:
  void (*m_function)();
  QDateTime m_threshold;

private slots:
  void checkTime();
};