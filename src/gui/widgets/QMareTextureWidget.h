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

	void saveCurrentTexture();

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

	[[nodiscard]] bool useR() const;

	[[nodiscard]] bool useG() const;

	[[nodiscard]] bool useB() const;

	void setRGB(bool newR, bool newG, bool newB);

	[[nodiscard]] bool useA() const;

	void setA(bool newA);

	[[nodiscard]] bool useAMask() const;

	void setAMask(bool newAMask);

	[[nodiscard]] bool useTiled() const;

	void setTiled(bool newTiled);

	[[nodiscard]] int getCurrentCubemapMode() const;

	void setCurrentCubemapMode(int mode);

	explicit operator bool() const;

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
	bool r = true;
	bool g = true;
	bool b = true;
	bool a = true;
	bool aMask = true;
	bool tiled = false;
	int cubemapMode = 0;

	QPointF mousePressPosition;
};
