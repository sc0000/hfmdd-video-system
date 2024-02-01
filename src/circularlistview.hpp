#pragma once
#include <QListView>

class CircularListView : public QListView {
	Q_OBJECT

public:
	CircularListView(QWidget *parent = nullptr) : QListView(parent){};

  void setName(const QModelIndex& index, const QString& name);

public slots:
	void cursorUp();
	void cursorDown();

protected:
	QModelIndex moveCursor(QAbstractItemView::CursorAction action,
			       Qt::KeyboardModifiers modifiers);
};
