#pragma once

#include <QDoubleSpinBox>
#include <QSpinBox>

class QWheelEvent;

class QMareSpinBox : public QSpinBox {
	Q_OBJECT;

public:
	explicit QMareSpinBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};

class QMareDoubleSpinBox : public QDoubleSpinBox {
	Q_OBJECT;

public:
	explicit QMareDoubleSpinBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};
