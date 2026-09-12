#pragma once

#include <QComboBox>

class QWheelEvent;

/**
 * Changes combobox behavior to only respond to wheel events when strongly focused.
 */
class QMareComboBox : public QComboBox {
	Q_OBJECT;

public:
	explicit QMareComboBox(QWidget* parent = nullptr);

protected:
	void wheelEvent(QWheelEvent* event) override;
};
