#include "QMareOptions.h"

QSettings* opts = nullptr;

void QMareOptions::setupOptions(QSettings& options) {
	if (!options.contains(STR_DEFAULT_CREATE_DIALOG_DIR)) {
		options.setValue(STR_DEFAULT_CREATE_DIALOG_DIR, QString{});
	}

	opts = &options;
}

QSettings* QMareOptions::getOptions() {
	return opts;
}

void QMareOptions::invert(std::string_view option) {
	set(option, !get<bool>(option));
}
