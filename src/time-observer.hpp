#pragma once

#include <QDateTime>

class TimeObserver : public QObject {
  Q_OBJECT

public:
  TimeObserver(QDateTime threshold, void (*function)(), QObject* parent = nullptr);
  ~TimeObserver();

  void start();
  void stop();
  
  inline void setThreshold(const QDateTime& threshold) { m_threshold = threshold; }

private:
  void (*m_function)();
  QDateTime m_threshold;
  QTimer* m_timer;

private slots:
  void checkTime();
};