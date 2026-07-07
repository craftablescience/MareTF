#include "QMareMiddleClickTabWidget.h"

#include <QKeyEvent>
#include <QTabBar>

void QMareMiddleClickTabWidget::keyReleaseEvent(QKeyEvent* event) {
	if (this->tabsClosable() && event->keyCombination() == (Qt::CTRL | Qt::Key_W)) {
		if (const auto tabIndex = this->tabBar()->currentIndex(); tabIndex != -1) {
			emit this->tabCloseRequested(tabIndex);
			event->accept();
			return;
		}
	}
	QTabWidget::keyReleaseEvent(event);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 11, 0)
#include <QMouseEvent>

void QMareMiddleClickTabWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (this->tabsClosable() && event->button() == Qt::MiddleButton) {
		if (const auto tabIndex = this->tabBar()->tabAt(event->pos()); tabIndex != -1) {
			emit this->tabCloseRequested(tabIndex);
			event->accept();
			return;
		}
	}
	QTabWidget::mousePressEvent(event);
}
#endif
