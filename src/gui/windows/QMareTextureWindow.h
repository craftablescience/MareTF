#pragma once

#include <QMainWindow>

class QAction;
class QCheckBox;
class QDockWidget;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTimer;

class QMareComboBox;
class QMareDoubleSpinBox;
class QMareFlagsWidget;
class QMareFlagsExtraWidget;
class QMareMiddleClickTabWidget;
class QMareSpinBox;

class QMareTextureWindow : public QMainWindow {
	friend class QMareEmptyWindow;

	Q_OBJECT;

public:
	QMareTextureWindow();

	void loadTexture(const QString& path);

	void regenerateDetails();

	bool saveTab(int index);

signals:
	void themeUpdated();

protected:
	void closeEvent(QCloseEvent* event) override;

	void dragEnterEvent(QDragEnterEvent* event) override;

	void dropEvent(QDropEvent* event) override;

	// todo(edit)
	QAction* saveAction;
	// ---- end

	QMareMiddleClickTabWidget* textureTabs;

	QDockWidget* previewDock;

	QGroupBox* previewGeneralGroup;
	QMareSpinBox* previewCurrentMip;
	QPushButton* previewR;
	QPushButton* previewG;
	QPushButton* previewB;
	QPushButton* previewA;
	QPushButton* previewAMask;
	QPushButton* previewTiled;

	QGroupBox* previewAnimationGroup;
	QMareSpinBox* previewCurrentFrame;
	QMareDoubleSpinBox* previewAnimationSpeed;
	QCheckBox* previewAnimate;
	QTimer* previewAnimateTimer;

	QGroupBox* previewCubemapGroup;
	QMareComboBox* previewCubemapMode;
	QMareSpinBox* previewCurrentFace;

	QGroupBox* previewDepthGroup;
	QMareSpinBox* previewCurrentDepth;

	QDockWidget* detailsDock;

	QGroupBox* detailsFileTypeGroup;
	QMareComboBox* detailsPlatform;
	QMareComboBox* detailsVersion;
	QMareComboBox* detailsFormat;
	QLabel* detailsFileSizeLabel;
	QMareDoubleSpinBox* detailsFileSize;

	QGroupBox* detailsDimsGroup;
	QMareSpinBox* detailsWidth;
	QMareSpinBox* detailsHeight;
	QMareSpinBox* detailsDepth;
	QMareSpinBox* detailsFrames;
	QMareSpinBox* detailsStartFrame;
	QCheckBox* detailsCubemap;
	QCheckBox* detailsMipmaps;
	QMareSpinBox* detailsConsoleMipScale;

	QGroupBox* detailsMiscellaneousGroup;
	QMareDoubleSpinBox* detailsBumpmapScale;
	QGroupBox* detailsReflectivityGroup;
	QLabel* detailsReflectivityR;
	QLabel* detailsReflectivityG;
	QLabel* detailsReflectivityB;

	QGroupBox* detailsCompressionGroup;
	QMareComboBox* detailsCompressionMethod;
	QMareSpinBox* detailsCompressionLevel;

	QDockWidget* resDock;

	QGroupBox* resThumbnailGroup;
	QLabel* resThumbnailPreview;
	QMareSpinBox* resThumbnailWidth;
	QMareSpinBox* resThumbnailHeight;

	QGroupBox* resPaletteGroup;
	QLabel* resPalettePreview;
	// todo: use the frame spinbox in the preview section
	QMareSpinBox* resPaletteFrame;

	QGroupBox* resFallbackGroup;
	QMareSpinBox* resFallbackWidth;
	QMareSpinBox* resFallbackHeight;
	QMareSpinBox* resFallbackMips;

	QGroupBox* resParallaxCorrectedCubemapGroup;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapOriginX = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapOriginY = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapOriginZ = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapOriginW = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV00 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV01 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV02 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV03 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV10 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV11 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV12 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV13 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV20 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV21 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV22 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV23 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV30 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV31 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV32 = nullptr;
	QMareDoubleSpinBox* resParallaxCorrectedCubemapInverseTransformV33 = nullptr;

	QGroupBox* resCRCGroup;
	QLineEdit* resCRCValue;

	QGroupBox* resTS0Group;
	QLineEdit* resTS0Value;

	QGroupBox* resLODGroup;
	QMareSpinBox* resLODValueU;
	QMareSpinBox* resLODValueV;
	QMareSpinBox* resLODValueU360;
	QMareSpinBox* resLODValueV360;

	QGroupBox* resKeyValuesGroup;
	QPlainTextEdit* resKeyValuesData;

	QGroupBox* resAuthorInfoGroup;
	QLineEdit* resAuthorInfoData;

	QGroupBox* resSourcePPFlagsGroup;
	QMareFlagsExtraWidget* resSourcePPFlagsList;

	QDockWidget* flagsDock;

	QMareFlagsWidget* flagsChecks;
};

extern QMareTextureWindow* g_ManeWindow;
