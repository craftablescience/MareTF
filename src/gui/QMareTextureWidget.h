#pragma once

#include <QWidget>
#include <vtfpp/VTF.h>

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

class QMareTextureWidget : public QWidget {
	Q_OBJECT;

public:
	explicit QMareTextureWidget(QWidget* parent = nullptr);

	void loadTexture(const QString& path_);

	[[nodiscard]] QIcon getIcon() const;

	[[nodiscard]] QString getPath() const;

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
