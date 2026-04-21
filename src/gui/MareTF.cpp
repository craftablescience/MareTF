#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTranslator>

#include "Config.h"

#include "utility/QMareOptions.h"
#include "windows/QMareEmptyWindow.h"
#include "windows/QMareTextureWindow.h"

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

#ifndef Q_OS_WASM
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

				if (QMareOptions::get<bool>(QMareOptions::BOOL_RAISE_TO_TOP_OPENING_FILE))
#ifdef Q_OS_WIN
				{
					const auto flags = g_ManeWindow->windowFlags();
					g_ManeWindow->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
					g_ManeWindow->show();
					g_ManeWindow->setWindowFlags(flags);
				}
				// Will simply flash in taskbar if above code didn't run
#endif
				{
					// Yes we need all of these
					g_ManeWindow->show();
					g_ManeWindow->raise();
					g_ManeWindow->activateWindow();
				}

			}
			client->deleteLater();
		});
	});
	server.listen(serverName);
#endif

	const auto languageOverride = QMareOptions::get<QString>(QMareOptions::STR_LANGUAGE_OVERRIDE);
	const auto locale = languageOverride.isEmpty() ? QLocale{} : QLocale{languageOverride};
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
	QTranslator translator;
	if (translator.load(locale, PROJECT_NAME, "_", ":/i18n")) {
		QCoreApplication::installTranslator(&translator);
	}

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
