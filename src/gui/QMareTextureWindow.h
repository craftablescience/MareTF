#pragma once

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QListWidget;
class QSpinBox;
class QTabWidget;
class QTimer;

class QMareTextureWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareTextureWindow();

	void loadTexture(const QString& path);

	void regenerateDetails();

protected:
	QTabWidget* textureTabs;

	QGroupBox* previewGeneralGroup;
	QSpinBox* previewCurrentMip;
	QCheckBox* previewAlpha;
	QCheckBox* previewBackground;

	QGroupBox* previewAnimationGroup;
	QSpinBox* previewCurrentFrame;
	QDoubleSpinBox* previewAnimationSpeed;
	QCheckBox* previewAnimate;
	QTimer* previewAnimateTimer;

	QGroupBox* previewCubemapGroup;
	QSpinBox* previewCurrentFace;

	QGroupBox* previewDepthGroup;
	QSpinBox* previewCurrentDepth;

	QGroupBox* detailsFileTypeGroup;
	QComboBox* detailsPlatform;
	QComboBox* detailsVersion;
	QComboBox* detailsFormat;

	QGroupBox* detailsDimsGroup;
	QSpinBox* detailsWidth;
	QSpinBox* detailsHeight;
	QSpinBox* detailsDepth;
	QSpinBox* detailsFrames;
	QSpinBox* detailsStartFrame;
	QCheckBox* detailsCubemap;
	QCheckBox* detailsMipmaps;

	QGroupBox* detailsMiscellaneousGroup;
	QDoubleSpinBox* detailsBumpmapScale;
	QDoubleSpinBox* detailsReflectivityR;
	QDoubleSpinBox* detailsReflectivityG;
	QDoubleSpinBox* detailsReflectivityB;

	QGroupBox* detailsCompressionGroup;
	QComboBox* detailsCompressionMethod;
	QSpinBox* detailsCompressionLevel;

	QListWidget* flagsChecks;
};
