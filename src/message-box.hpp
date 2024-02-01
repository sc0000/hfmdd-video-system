#include <QDialog>
#include <QString>
#include <QWidget>


class MessageBox : public QDialog
{
  Q_OBJECT

public:
  explicit MessageBox(const QString& message, QWidget* parent = nullptr);
};