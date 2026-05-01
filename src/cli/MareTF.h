#pragma once

#include <string>
#include <tuple>

#ifndef MARETF_CLI
#include <QWidget>

[[nodiscard]] std::tuple<int, std::string> maretf_cli(int argc, const char* const argv[], QWidget* guiParent = nullptr);
#endif
