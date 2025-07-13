#include "QMareTextureWindow.h"

#include <cstdlib>

#include <discord_rpc.h>
#include <QApplication>

#include "../common/Common.h"
#include "../common/Config.h"
#include "QMareEmptyWindow.h"
#include "QMareTextureWindow.h"

int main(int argc, char* argv[]) {
	QApplication app{argc, argv};

	QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
	QCoreApplication::setApplicationName(PROJECT_NAME);
	QCoreApplication::setApplicationVersion(PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
	QGuiApplication::setDesktopFileName(PROJECT_NAME);
#endif

	// Discord integration
	// todo: make optional
	MARETF_MSVC_SEH_IGNORE_BEGIN()
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
	MARETF_MSVC_SEH_IGNORE_END()

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
