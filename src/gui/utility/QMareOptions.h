#pragma once

#include <string_view>

#include <QSettings>

namespace QMareOptions {

#define MARETF_OPTION(name) constexpr const char* name = #name

// Options
MARETF_OPTION(BOOL_RAISE_TO_TOP_OPENING_FILE);
MARETF_OPTION(BOOL_ALLOW_MULTIPLE_APP_INSTANCES);
MARETF_OPTION(BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE);
MARETF_OPTION(BOOL_HIGH_QUALITY_THUMBNAILS);
MARETF_OPTION(BOOL_HIGH_QUALITY_MINIMAP);
MARETF_OPTION(INT_MINIMAP_SCALE);
MARETF_OPTION(STR_STYLE);
MARETF_OPTION(STR_LANGUAGE_OVERRIDE);
MARETF_OPTION(BOOL_ENABLE_DISCORD_RICH_PRESENCE);

// Storage
MARETF_OPTION(STR_DEFAULT_CREATE_DIALOG_DIR);

#undef MARETF_OPTION

void setupOptions(QSettings& options);

QSettings* getOptions();

template<typename T>
T get(std::string_view option) {
	return getOptions()->value(option).value<T>();
}

template<typename T>
void set(std::string_view option, T value) {
	getOptions()->setValue(option, value);
}

// Only use for booleans!
void invert(std::string_view option);

} // namespace QMareOptions
