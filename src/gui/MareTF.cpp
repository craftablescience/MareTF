#include "MareTF.h"

#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QVBoxLayout>

#ifdef _WIN32
	#include <QFontDatabase>
#endif

#include "../common/Config.h"

// Should match background.png
constexpr int M_MANE_WINDOW_WIDTH = 900;
constexpr int M_MANE_WINDOW_HEIGHT = 500;

constexpr int M_TEXTBOX_HEIGHT = 200;
constexpr int M_TEXTBOX_TEXT_WIDTH = 680;
constexpr int M_TEXTBOX_TEXT_HEIGHT = 128;
constexpr int M_TEXTBOX_TEXT_X_OFFSET = 180;
constexpr int M_TEXTBOX_TEXT_Y_OFFSET = 38;
constexpr int M_TEXTBOX_TEXT_FONT_SIZE = 18;

QMareTextBox::QMareTextBox(QWidget* parent)
		: QWidget(parent) {
	this->setFixedSize(M_MANE_WINDOW_WIDTH, M_TEXTBOX_HEIGHT);

	auto* textboxBackground = new QLabel{this};
	textboxBackground->setPixmap({":/text_box_background.png"});
	textboxBackground->move(0, 0);

	this->textboxFace = new QLabel{this};
	this->textboxFace->move(0, 0);

	auto* textboxForeground = new QLabel{this};
	textboxForeground->setPixmap({":/text_box_foreground.png"});
	textboxBackground->move(0, 0);

	this->textboxText = new QLabel{this};
	{
		auto textboxFont = this->textboxText->font();
		textboxFont.setPointSize(M_TEXTBOX_TEXT_FONT_SIZE);
		this->textboxText->setFont(textboxFont);
	}
	this->textboxText->setWordWrap(true);
	this->textboxText->move(M_TEXTBOX_TEXT_X_OFFSET, M_TEXTBOX_TEXT_Y_OFFSET);
	this->textboxText->setFixedWidth(M_TEXTBOX_TEXT_WIDTH);
	this->textboxText->setMaximumHeight(M_TEXTBOX_TEXT_HEIGHT);
}

void QMareTextBox::setText(const QString& text, Face face) {
	switch (face) {
		using enum Face;
		case SMILING:
			this->textboxFace->setPixmap({":/text_box_face_smiling.png"});
			break;
	}
	this->textboxText->setText(text);
}

QMareWindow::QMareWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon(":/logo.png"));
	this->setFixedSize(M_MANE_WINDOW_WIDTH, M_MANE_WINDOW_HEIGHT);

	//region Make window transparent
#if 1
	this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_NoSystemBackground, true);
	this->setAttribute(Qt::WA_TranslucentBackground, true);
#endif
	//endregion

	//region Background

	auto* background = new QLabel{this};
	background->setPixmap(QPixmap{":/background.png"}.scaled(M_MANE_WINDOW_WIDTH, M_MANE_WINDOW_HEIGHT));
	this->setCentralWidget(background);

	auto* backgroundLayout = new QVBoxLayout{background};
	backgroundLayout->setAlignment(Qt::AlignTop);

	//endregion

	//region Mane window buttons

	auto* maneWindowButtonsParent = new QWidget{background};
	backgroundLayout->addWidget(maneWindowButtonsParent, 0, Qt::AlignRight);
	auto* maneWindowButtonsLayout = new QHBoxLayout{maneWindowButtonsParent};

	auto* minimizeButton = new QToolButton{maneWindowButtonsParent};
	minimizeButton->setFixedSize(64, 64);
	minimizeButton->setStyleSheet(
		"QToolButton         { background-color: rgba(0,0,0,0); border: none; border-image: url(:/minimize.png)       0 0 0 0 stretch stretch; }\n"
		"QToolButton:hover   { background-color: rgba(0,0,0,0); border: none; border-image: url(:/minimize_hover.png) 0 0 0 0 stretch stretch; }\n"
		"QToolButton:pressed { background-color: rgba(0,0,0,0); border: none; border-image: url(:/minimize.png)       0 0 0 0 stretch stretch; }");
	minimizeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	maneWindowButtonsLayout->addWidget(minimizeButton);
	connect(minimizeButton, &QToolButton::released, this, &QMareWindow::showMinimized);

	auto* closeButton = new QToolButton{maneWindowButtonsParent};
	closeButton->setFixedSize(64, 64);
	closeButton->setStyleSheet(
		"QToolButton         { background-color: rgba(0,0,0,0); border: none; border-image: url(:/close.png)       0 0 0 0 stretch stretch; }\n"
		"QToolButton:hover   { background-color: rgba(0,0,0,0); border: none; border-image: url(:/close_hover.png) 0 0 0 0 stretch stretch; }\n"
		"QToolButton:pressed { background-color: rgba(0,0,0,0); border: none; border-image: url(:/close.png)       0 0 0 0 stretch stretch; }");
	closeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	maneWindowButtonsLayout->addWidget(closeButton);
	connect(closeButton, &QToolButton::released, this, &QMareWindow::close);

	//endregion

	//region Kirin

	auto* kirin = new QLabel{background};
	kirin->setPixmap({":/olive_shade_sleep.png"});
	backgroundLayout->addWidget(kirin, 0, Qt::AlignHCenter);

	//endregion

	//region Textbox

	backgroundLayout->addStretch(1);

	auto* textbox = new QMareTextBox{background};
	backgroundLayout->addWidget(textbox);
	textbox->setText("HI!!!! Nice to meet you!!! I'm Olive Shade!", QMareTextBox::Face::SMILING);

	//endregion
}

void QMareWindow::mousePressEvent(QMouseEvent* e) {
	this->oldMousePos = e->globalPosition();
}

void QMareWindow::mouseMoveEvent(QMouseEvent* e) {
	const QPointF delta = e->globalPosition() - this->oldMousePos;
	this->move(static_cast<int>(this->x() + delta.x()), static_cast<int>(this->y() + delta.y()));
	this->oldMousePos = e->globalPosition();
}

int main(int argc, char* argv[]) {
	QApplication app{argc, argv};

	QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
	QCoreApplication::setApplicationName(PROJECT_NAME);
	QCoreApplication::setApplicationVersion(PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
	QGuiApplication::setDesktopFileName(PROJECT_NAME);
#endif

	// Font is installed to system on non-Windows platforms
#ifdef _WIN32
	QApplication::setFont(QFont{QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/CelestiaMediumRedux.ttf")).at(0)});
#else
	QApplication::setFont(QFont{"Celestia Medium Redux"});
#endif

	auto* window = new QMareWindow;
	window->show();

	return QApplication::exec();
}
