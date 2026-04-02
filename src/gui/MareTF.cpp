#include <cstdlib>

#include <discord_rpc.h>
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTranslator>

#include "../common/Config.h"
#include "QMareEmptyWindow.h"
#include "QMareOptions.h"
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

	const auto options = std::make_unique<QSettings>();
	QMareOptions::setupOptions(*options);

	const auto serverName = QString{"%1_%2_%3"}.arg(PROJECT_ORGANIZATION_NAME, PROJECT_NAME, PROJECT_VERSION);
	QLocalSocket socket;
	socket.connectToServer(serverName);
	if (socket.waitForConnected(500)) {
		QByteArray data;
		QDataStream stream{&data, QIODevice::WriteOnly};
		stream << QCoreApplication::arguments();
		socket.write(data);
		socket.waitForBytesWritten(500);
		return 0;
	}

	QLocalServer server;
	QLocalServer::removeServer(serverName);
	QObject::connect(&server, &QLocalServer::newConnection, [&server] {
		auto* client = server.nextPendingConnection();
		QObject::connect(client, &QLocalSocket::readyRead, [client] {
			if (g_ManeWindow) {
				const QByteArray data = client->readAll();
				QDataStream stream{data};
				QStringList args;
				stream >> args;
				for (int i = 1; i < args.size(); i++) {
					g_ManeWindow->loadTexture(args[i]);
				}
				g_ManeWindow->raise();
				g_ManeWindow->activateWindow();
			}
			client->deleteLater();
		});
	});
	server.listen(serverName);

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
		g_ManeWindow = new QMareTextureWindow;
		for (int i = 1; i < argc; i++) {
			g_ManeWindow->loadTexture(argv[i]);
		}
		g_ManeWindow->show();
	} else {
		auto* emptyWindow = new QMareEmptyWindow;
		emptyWindow->show();
	}

	return QApplication::exec();
}
