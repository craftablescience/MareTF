#pragma once

#include <string>
#include <tuple>

#ifndef MARETF_CLI
#include <QWidget>
#endif

namespace maretf {

enum class HDRIMode {
    FLAT,
    CUBEMAP,
};

} // namespace maretf

#ifndef MARETF_CLI
[[nodiscard]] std::tuple<int, std::string> maretf_cli(int argc, const char* const argv[], QWidget* guiParent = nullptr);
#endif
