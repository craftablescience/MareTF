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

	void reloadCurrentTexture();

	[[nodiscard]] QIcon getIcon() const;

	[[nodiscard]] QString getPath() const;

	[[nodiscard]] const vtfpp::VTF& getVTF() const;

	[[nodiscard]] vtfpp::VTF& getVTF();

	void applyIncrementalZoom(float increment);

	[[nodiscard]] uint8_t getCurrentMip() const;

	void setCurrentMip(uint8_t mip);

	[[nodiscard]] uint16_t getCurrentFrame() const;

	void setCurrentFrame(uint16_t frame);

	[[nodiscard]] uint8_t getCurrentFace() const;

	void setCurrentFace(uint8_t face);

	[[nodiscard]] uint16_t getCurrentDepth() const;

	void setCurrentDepth(uint16_t depth);

	explicit operator bool() const;

	[[nodiscard]] static bool& useBackground();

	[[nodiscard]] static bool& useAlpha();

protected:
	void mouseMoveEvent(QMouseEvent* e) override;

	void mousePressEvent(QMouseEvent* e) override;

	void mouseReleaseEvent(QMouseEvent* e) override;

	void paintEvent(QPaintEvent*) override;

	void resizeEvent(QResizeEvent* e) override;

	void wheelEvent(QWheelEvent* e) override;

	QString path;
	vtfpp::VTF vtf;
	std::vector<std::byte> textureCurrentData;
	QImage textureCurrent;
	QPointF textureOffset;
	float textureZoom = 1.f;
	uint8_t currentMip = 0;
	uint8_t currentFace = 0;
	uint16_t currentFrame = 0;
	uint16_t currentDepth = 0;

	QPointF mousePressPosition;
};
