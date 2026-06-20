#include "QMareMiddleClickTabWidget.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 11, 0)
#include <QMouseEvent>
#include <QTabBar>

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
