#pragma once

#include <array>
#include <string_view>

#include <vtfpp/VTF.h>

std::array<std::string_view, 32> getPrettyFlagNamesFor(uint16_t minorVersion, vtfpp::VTF::Platform platform);
