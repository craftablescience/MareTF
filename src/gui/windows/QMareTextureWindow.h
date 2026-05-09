#pragma once

#include <QMainWindow>

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
	Q_OBJECT;

public:
	QMareTextureWindow();

	void loadTexture(const QString& path);

	void regenerateDetails();

signals:
	void themeUpdated();

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;

	void dropEvent(QDropEvent* event) override;

	QMareMiddleClickTabWidget* textureTabs;

	QDockWidget* previewDock;

	QGroupBox* previewGeneralGroup;
	QMareSpinBox* previewCurrentMip;
	QCheckBox* previewR;
	QCheckBox* previewG;
	QCheckBox* previewB;
	QCheckBox* previewA;
	QCheckBox* previewBackground;
	QCheckBox* previewTiled;

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
