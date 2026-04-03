#include "QMareSpinBox.h"

#include <QWheelEvent>

QMareSpinBox::QMareSpinBox(QWidget* parent) : QSpinBox{parent} {
	this->setFocusPolicy(Qt::StrongFocus);
}

void QMareSpinBox::wheelEvent(QWheelEvent* event) {
	if (!this->hasFocus()) {
		event->ignore();
	} else {
		QSpinBox::wheelEvent(event);
	}
}

QMareDoubleSpinBox::QMareDoubleSpinBox(QWidget* parent) : QDoubleSpinBox{parent} {
	this->setFocusPolicy(Qt::StrongFocus);
}

void QMareDoubleSpinBox::wheelEvent(QWheelEvent* event) {
	if (!this->hasFocus()) {
		event->ignore();
	} else {
		QDoubleSpinBox::wheelEvent(event);
	}
}
