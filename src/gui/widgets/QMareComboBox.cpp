#include "QMareComboBox.h"

#include <QWheelEvent>

QMareComboBox::QMareComboBox(QWidget* parent) : QComboBox{parent} {
	this->setFocusPolicy(Qt::StrongFocus);
}

void QMareComboBox::wheelEvent(QWheelEvent* event) {
	if (!this->hasFocus()) {
		event->ignore();
	} else {
		QComboBox::wheelEvent(event);
	}
}
