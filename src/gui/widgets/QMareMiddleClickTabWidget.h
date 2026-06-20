#pragma once

#include <QTabWidget>

class QMouseEvent;

class QMareMiddleClickTabWidget : public QTabWidget {
	Q_OBJECT;

public:
	using QTabWidget::QTabWidget;

#if QT_VERSION < QT_VERSION_CHECK(6, 11, 0)
protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
#endif
};
