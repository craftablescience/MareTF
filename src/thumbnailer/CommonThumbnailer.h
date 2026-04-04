#pragma once

#include <vtfpp/VTF.h>

std::pair<std::vector<std::byte>, vtfpp::ImageFormat> createThumbnail(const vtfpp::VTF& vtf, int& targetWidth, int& targetHeight);

std::pair<std::vector<std::byte>, vtfpp::ImageFormat> createThumbnail(const std::string& in, int& targetWidth, int& targetHeight);
