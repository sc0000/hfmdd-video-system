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

public:
  void fade();
  inline bool isVisible() { return visible; }
  void setFadeDuration(int fadeDuration);
};