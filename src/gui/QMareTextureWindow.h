#pragma once

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
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
	QSpinBox* detailsConsoleMipScale;

	QGroupBox* detailsMiscellaneousGroup;
	QDoubleSpinBox* detailsBumpmapScale;
	QGroupBox* detailsReflectivityGroup;
	QLabel* detailsReflectivityR;
	QLabel* detailsReflectivityG;
	QLabel* detailsReflectivityB;

	QGroupBox* detailsCompressionGroup;
	QComboBox* detailsCompressionMethod;
	QSpinBox* detailsCompressionLevel;

	QGroupBox* resThumbnailGroup;
	QLabel* resThumbnailPreview;
	QSpinBox* resThumbnailWidth;
	QSpinBox* resThumbnailHeight;

	QGroupBox* resPaletteGroup;
	QLabel* resPalettePreview;
	// todo: use the frame spinbox in the preview section
	QSpinBox* resPaletteFrame;

	QGroupBox* resFallbackGroup;
	QSpinBox* resFallbackWidth;
	QSpinBox* resFallbackHeight;
	QSpinBox* resFallbackMips;

	QGroupBox* resCRCGroup;
	QLineEdit* resCRCValue;

	QGroupBox* resTS0Group;
	QLineEdit* resTS0Value;

	QGroupBox* resLODGroup;
	QSpinBox* resLODValueU;
	QSpinBox* resLODValueV;
	QSpinBox* resLODValueU360;
	QSpinBox* resLODValueV360;

	QGroupBox* resKeyValuesGroup;
	QPlainTextEdit* resKeyValuesData;

	QListWidget* flagsChecks;
};
