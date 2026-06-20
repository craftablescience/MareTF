#include "QMareOptions.h"

#include <QApplication>
#include <QStyle>

static QSettings* opts = nullptr;

void QMareOptions::setupOptions(QSettings& options) {
	if (!options.contains(BOOL_RAISE_TO_TOP_OPENING_FILE)) {
		options.setValue(BOOL_RAISE_TO_TOP_OPENING_FILE, true);
	}

	if (!options.contains(BOOL_ALLOW_MULTIPLE_APP_INSTANCES)) {
		options.setValue(BOOL_ALLOW_MULTIPLE_APP_INSTANCES, false);
	}

	if (!options.contains(BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE)) {
		options.setValue(BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE, true);
	}

	if (!options.contains(BOOL_HIGH_QUALITY_THUMBNAILS)) {
		options.setValue(BOOL_HIGH_QUALITY_THUMBNAILS, true);
	}

	if (!options.contains(BOOL_HIGH_QUALITY_MINIMAP)) {
		options.setValue(BOOL_HIGH_QUALITY_MINIMAP, true);
	}

	if (!options.contains(INT_MINIMAP_SCALE)) {
		options.setValue(INT_MINIMAP_SCALE, -1);
	}

	if (!options.contains(STR_STYLE)) {
		options.setValue(STR_STYLE, QApplication::style()->name());
	} else {
		QApplication::setStyle(options.value(STR_STYLE).toString());
	}

	if (!options.contains(STR_LANGUAGE_OVERRIDE)) {
		options.setValue(STR_LANGUAGE_OVERRIDE, QString{});
	}

	if (!options.contains(BOOL_ENABLE_DISCORD_RICH_PRESENCE)) {
		options.setValue(BOOL_ENABLE_DISCORD_RICH_PRESENCE, true);
	}

	if (!options.contains(BOOL_ENABLE_TRYPANOPHOBIA_MODE)) {
		options.setValue(BOOL_ENABLE_TRYPANOPHOBIA_MODE, false);
	}

	if (!options.contains(STR_DEFAULT_CREATE_DIALOG_DIR)) {
		options.setValue(STR_DEFAULT_CREATE_DIALOG_DIR, QString{});
	}

	if (!options.contains(STR_DEFAULT_OPEN_OR_SAVE_DIALOG_DIR)) {
		options.setValue(STR_DEFAULT_OPEN_OR_SAVE_DIALOG_DIR, QString{});
	}

	if (!options.contains(STR_DEFAULT_EXTRACT_DIALOG_DIR)) {
		options.setValue(STR_DEFAULT_EXTRACT_DIALOG_DIR, QString{});
	}

	opts = &options;
}

QSettings* QMareOptions::getOptions() {
	return opts;
}

void QMareOptions::invert(std::string_view option) {
	set(option, !get<bool>(option));
}
