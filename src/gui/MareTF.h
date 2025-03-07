#pragma once

#include <QMainWindow>

class QLabel;
class QMouseEvent;

class QMareTextBox : public QWidget {
	Q_OBJECT;

public:
	enum class Face {
		SMILING,
	};

	explicit QMareTextBox(QWidget* parent = nullptr);

	void setText(const QString& text, Face face);

protected:
	QLabel* textboxFace;
	QLabel* textboxText;
};

class QMareWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareWindow();

protected:
	void mousePressEvent(QMouseEvent* e) override;

	void mouseMoveEvent(QMouseEvent* e) override;

private:
	QPointF oldMousePos;
};
