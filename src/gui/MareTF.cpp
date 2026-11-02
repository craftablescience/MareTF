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

	QMareTextureWindow* maneWindow = nullptr;
	QMareEmptyWindow* emptyWindow = nullptr;
	const auto closeAllWindows = [&maneWindow, &emptyWindow] {
		if (maneWindow) {
			maneWindow->close();
			maneWindow = nullptr;
		}
		if (emptyWindow) {
			emptyWindow->close();
			emptyWindow = nullptr;
		}
	};
	const auto showManeWindow = [&maneWindow, &closeAllWindows](const QStringList& files) {
		closeAllWindows();
		maneWindow = new QMareTextureWindow;
		for (const auto& file : files) {
			maneWindow->loadTexture(file);
		}
		maneWindow->show();
	};
	const auto showEmptyWindow = [&maneWindow, &emptyWindow, &closeAllWindows, &showManeWindow] {
		closeAllWindows();
		emptyWindow = new QMareEmptyWindow{maneWindow};
		emptyWindow->show();
		QObject::connect(emptyWindow, &QMareEmptyWindow::showManeWindow, showManeWindow);
	};

#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	QLocalServer server;
	if (!QMareOptions::get<bool>(QMareOptions::BOOL_ALLOW_MULTIPLE_APP_INSTANCES)) {
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

		QLocalServer::removeServer(serverName);
		QObject::connect(&server, &QLocalServer::newConnection, [maneWindow, showManeWindow, &server] {
			auto* client = server.nextPendingConnection();
			QObject::connect(client, &QLocalSocket::readyRead, [maneWindow, showManeWindow, client] {
				const QByteArray data = client->readAll();
				QDataStream stream{data};
				QStringList args;
				stream >> args;

				if (maneWindow) {
					for (int i = 1; i < args.size(); i++) {
						maneWindow->loadTexture(args[i]);
					}

					if (QMareOptions::get<bool>(QMareOptions::BOOL_RAISE_TO_TOP_OPENING_FILE))
#ifdef Q_OS_WIN
					{
						const auto flags = maneWindow->windowFlags();
						maneWindow->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
						maneWindow->show();
						maneWindow->setWindowFlags(flags);
					}
					// Will simply flash in taskbar if above code didn't run
#endif
					{
						// Yes we need all of these
						maneWindow->show();
						maneWindow->raise();
						maneWindow->activateWindow();
					}
				} else {
					showManeWindow(args);
				}
				client->deleteLater();
			});
		});
		server.listen(serverName);
	}
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
		QStringList args;
		for (int i = 1; i < argc; i++) {
			args << argv[i];
		}
		showManeWindow(args);
	} else {
		showEmptyWindow();
	}

	return QApplication::exec();
}
