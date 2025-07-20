#include "QMareTextureWindow.h"

#include <cstdlib>

#include <discord_rpc.h>
#include <QApplication>
#include <QTranslator>

#include "../common/Common.h"
#include "../common/Config.h"
#include "QMareEmptyWindow.h"
#include "QMareTextureWindow.h"

namespace {

void initDiscord() {
	DiscordEventHandlers handlers{};
	Discord_Initialize("1384260711202422845", &handlers, true, nullptr);

	static constexpr DiscordButton buttons[2] {{"View on GitHub", PROJECT_HOMEPAGE_URL}, {}};
	static constexpr DiscordRichPresence discordPresence{
		.state = "v" PROJECT_VERSION,
		.largeImageKey = "icon",
		.largeImageText = PROJECT_TITLE,
		.buttons = buttons,
	};
	Discord_UpdatePresence(&discordPresence);

	std::atexit(&Discord_Shutdown);
}

} // namespace

int main(int argc, char* argv[]) {
	QApplication app{argc, argv};

	QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
	QCoreApplication::setApplicationName(PROJECT_NAME);
	QCoreApplication::setApplicationVersion(PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
	QGuiApplication::setDesktopFileName(PROJECT_NAME);
#endif

	const QLocale locale;
	QTranslator translatorQtBase;
	if (translatorQtBase.load(locale, "qtbase", "_", ":/i18n")) {
		QCoreApplication::installTranslator(&translatorQtBase);
	}
	QTranslator translatorQtHelp;
	if (translatorQtHelp.load(locale, "qt_help", "_", ":/i18n")) {
		QCoreApplication::installTranslator(&translatorQtHelp);
	}
	QTranslator translatorQt;
	if (translatorQt.load(locale, "qt", "_", ":/i18n")) {
		QCoreApplication::installTranslator(&translatorQt);
	}

	// Discord integration
	// todo: make optional
	::initDiscord();

	// Show a window
	if (argc > 1) {
		auto* window = new QMareTextureWindow;
		for (int i = 1; i < argc; i++) {
			window->loadTexture(argv[i]);
		}
		window->show();
	} else {
		auto* emptyWindow = new QMareEmptyWindow;
		emptyWindow->show();
	}

	return QApplication::exec();
}
