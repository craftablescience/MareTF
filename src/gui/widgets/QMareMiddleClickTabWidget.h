#pragma once

#include <QTabWidget>

class QKeyEvent;
class QMouseEvent;

class QMareMiddleClickTabWidget : public QTabWidget {
	Q_OBJECT;

public:
	using QTabWidget::QTabWidget;

protected:
	void keyReleaseEvent(QKeyEvent* event) override;

#if QT_VERSION < QT_VERSION_CHECK(6, 11, 0)
	void mouseReleaseEvent(QMouseEvent* event) override;
#endif
};
