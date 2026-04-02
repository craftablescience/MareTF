#pragma once

#include <string_view>

#include <QSettings>

namespace QMareOptions {

#define MARETF_OPTION(name) constexpr std::string_view name = #name

// Options
// nothing here yet

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
