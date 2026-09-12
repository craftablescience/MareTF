#pragma once

#include <QDoubleSpinBox>
#include <QSpinBox>

class QWheelEvent;

/**
 * Changes spinbox behavior to only respond to wheel events when strongly focused.
 */
class QMareSpinBox : public QSpinBox {
	Q_OBJECT;

public:
	explicit QMareSpinBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};

/**
 * Changes spinbox behavior to only respond to wheel events when strongly focused.
 */
class QMareDoubleSpinBox : public QDoubleSpinBox {
	Q_OBJECT;

public:
	explicit QMareDoubleSpinBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};
