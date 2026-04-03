#pragma once

#include <QTabWidget>

class QMouseEvent;

class QMareMiddleClickTabWidget : public QTabWidget {
	Q_OBJECT;

public:
	using QTabWidget::QTabWidget;

protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
};
