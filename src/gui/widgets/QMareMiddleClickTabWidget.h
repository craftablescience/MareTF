#pragma once

#include <QTabWidget>

class QKeyEvent;
class QMouseEvent;

/**
 * Fixes tab widget before Qt 6.11 so tabs close when they are middle-clicked.
 */
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
