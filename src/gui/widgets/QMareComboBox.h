#pragma once

#include <QComboBox>

class QWheelEvent;

class QMareComboBox : public QComboBox {
	Q_OBJECT;

public:
	explicit QMareComboBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};
