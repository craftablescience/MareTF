#pragma once

#include <QMainWindow>
#include <vtfpp/vtfpp.h>

class QListWidget;
class QMouseEvent;
class QResizeEvent;
class QTabWidget;
class QWheelEvent;

class QMareEmptyWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareEmptyWindow();

protected:
	void paintEvent(QPaintEvent*) override;
};

class QMareTextureWidget : public QWidget {
	Q_OBJECT;

public:
	explicit QMareTextureWidget(QWidget* parent = nullptr);

	void loadTexture(const QString& path_);

	[[nodiscard]] QIcon getIcon() const;

	[[nodiscard]] const vtfpp::VTF& getVTF() const;

	[[nodiscard]] vtfpp::VTF& getVTF();

	explicit operator bool() const;

protected:
	void mouseMoveEvent(QMouseEvent* e) override;

	void mousePressEvent(QMouseEvent* e) override;

	void paintEvent(QPaintEvent*) override;

	void resizeEvent(QResizeEvent* e) override;

	void wheelEvent(QWheelEvent* e) override;

	QString path;
	vtfpp::VTF vtf;
	std::vector<std::byte> textureCurrentData;
	QImage textureCurrent;
	QPointF textureOffset;
	float textureZoom = 1.f;

	QPointF mousePressPosition;
};

class QMareTextureWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareTextureWindow();

	void loadTexture(const QString& path) const;

	void regenerateDetails();

protected:
	QTabWidget* textureTabs;
	QListWidget* flagsChecks;
};
