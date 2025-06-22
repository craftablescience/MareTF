#pragma once

#include <array>
#include <string_view>

#include <vtfpp/VTF.h>

#ifdef _WIN32
#define MARETF_MSVC_SEH_IGNORE_BEGIN() __try {
#define MARETF_MSVC_SEH_IGNORE_END() } __except (1 /*EXCEPTION_EXECUTE_HANDLER*/) {}
#else
#define MARETF_MSVC_SEH_IGNORE_BEGIN() {
#define MARETF_MSVC_SEH_IGNORE_END() }
#endif

std::array<std::string_view, 32> getPrettyFlagNamesFor(uint16_t minorVersion, vtfpp::VTF::Platform platform);
