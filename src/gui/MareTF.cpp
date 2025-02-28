#include "MareTF.h"

#include <QApplication>
#include <QLabel>

#ifdef _WIN32
	#include <QFontDatabase>
#endif

#include "../common/Config.h"

QMareWindow::QMareWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon(":/logo.png"));

	// Transparent window
#if 1
	this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_NoSystemBackground, true);
	this->setAttribute(Qt::WA_TranslucentBackground, true);
#endif

	auto* label = new QLabel{"Test", this};
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
