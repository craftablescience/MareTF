#include "QMareCLIWrapper.h"

#include "MareTF.h"

namespace {

#if defined(Q_OS_WIN) || defined(Q_OS_WASM) || defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
constexpr auto MARETF_PATH_SEPARATOR = ';';
#else
constexpr auto MARETF_PATH_SEPARATOR = ':';
#endif

} // namespace

QMareCLIWrapper::QMareCLIWrapper(const QString& mode, QObject* parent, const QString& program) : QObject{parent} {
	this->arguments.push_back(program);
	this->arguments.push_back(mode);
}

QString QMareCLIWrapper::getCommand() const {
	QString command;
	for (const auto& argument : this->arguments) {
		if (argument.contains(QRegularExpression{"\\s"})) {
			command.append('\"').append(argument).append('\"');
		} else {
			command.append(argument);
		}
		command.append(' ');
	}
	command.resize(command.size() - 1);
	return command;
}

int QMareCLIWrapper::exec() const {
	std::vector<std::string> argumentStrs;
	for (const auto& arg : this->arguments) {
		argumentStrs.emplace_back(arg.toUtf8().constData());
	}

	std::unique_ptr<const char*[]> cArgs{new const char*[argumentStrs.size()]};
	for (int i = 0; i < argumentStrs.size(); i++) {
		cArgs[i] = argumentStrs[i].c_str();
	}

	return maretf_cli(static_cast<int>(argumentStrs.size()), cArgs.get());
}

QString QMareCLIWrapper::joinPaths(const QStringList& paths) {
	return paths.join(MARETF_PATH_SEPARATOR);
}

QStringList QMareCLIWrapper::splitPaths(const QString& paths) {
	return paths.split(MARETF_PATH_SEPARATOR);
}
