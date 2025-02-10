#pragma once

#include <algorithm>
#include <concepts>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

#include <vtfpp/vtfpp.h>

namespace not_magic_enum {

namespace detail {

extern const std::vector<vtfpp::ImageFormat> IMAGE_FORMAT_E;
extern const std::vector<std::string_view>   IMAGE_FORMAT_S;

extern const std::vector<vtfpp::ImageConversion::ResizeFilter> RESIZE_FILTER_E;
extern const std::vector<std::string_view>                     RESIZE_FILTER_S;

extern const std::vector<vtfpp::VTF::Flags> VTF_FLAGS_E;
extern const std::vector<std::string_view>  VTF_FLAGS_S;

extern const std::vector<vtfpp::VTF::Platform> VTF_PLATFORM_E;
extern const std::vector<std::string_view>     VTF_PLATFORM_S;

extern const std::vector<vtfpp::CompressionMethod> COMPRESSION_METHOD_E;
extern const std::vector<std::string_view>         COMPRESSION_METHOD_S;

extern const std::vector<vtfpp::ImageConversion::ResizeMethod> RESIZE_METHOD_E;
extern const std::vector<std::string_view>                     RESIZE_METHOD_S;

template<typename E>
[[nodiscard]] const std::vector<E>& getEnums() {
	static_assert(
		std::same_as<E, vtfpp::ImageFormat> ||
		std::same_as<E, vtfpp::ImageConversion::ResizeFilter> ||
		std::same_as<E, vtfpp::VTF::Flags> ||
		std::same_as<E, vtfpp::VTF::Platform> ||
		std::same_as<E, vtfpp::CompressionMethod> ||
		std::same_as<E, vtfpp::ImageConversion::ResizeMethod>
	);
	if constexpr (std::same_as<E, vtfpp::ImageFormat>) {
		return IMAGE_FORMAT_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeFilter>) {
		return RESIZE_FILTER_E;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Flags>) {
		return VTF_FLAGS_E;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Platform>) {
		return VTF_PLATFORM_E;
	} else if constexpr (std::same_as<E, vtfpp::CompressionMethod>) {
		return COMPRESSION_METHOD_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeMethod>) {
		return RESIZE_METHOD_E;
	} else {
		static std::vector<E> empty;
		return empty;
	}
}

template<typename E>
[[nodiscard]] const std::vector<std::string_view>& getStrs() {
	static_assert(
		std::same_as<E, vtfpp::ImageFormat> ||
		std::same_as<E, vtfpp::ImageConversion::ResizeFilter> ||
		std::same_as<E, vtfpp::VTF::Flags> ||
		std::same_as<E, vtfpp::VTF::Platform> ||
		std::same_as<E, vtfpp::CompressionMethod> ||
		std::same_as<E, vtfpp::ImageConversion::ResizeMethod>
	);
	if constexpr (std::same_as<E, vtfpp::ImageFormat>) {
		return IMAGE_FORMAT_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeFilter>) {
		return RESIZE_FILTER_S;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Flags>) {
		return VTF_FLAGS_S;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Platform>) {
		return VTF_PLATFORM_S;
	} else if constexpr (std::same_as<E, vtfpp::CompressionMethod>) {
		return COMPRESSION_METHOD_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeMethod>) {
		return RESIZE_METHOD_S;
	} else {
		static std::vector<std::string_view> empty;
		return empty;
	}
}

} // namespace detail

template<typename E>
[[nodiscard]] std::string_view enum_name(E value) {
	const auto& enums = detail::getEnums<E>();
	const auto& strs = detail::getStrs<E>();
	if (const auto it = std::find(enums.begin(), enums.end(), value); it != enums.end()) {
		return strs.at(std::distance(enums.begin(), it));
	}
	return "";
}

template<typename E>
[[nodiscard]] const std::vector<std::string_view>& enum_names() {
	return detail::getStrs<E>();
}

template<typename E>
[[nodiscard]] std::optional<E> enum_cast(std::string_view value) {
	const auto& enums = detail::getEnums<E>();
	const auto& strs = detail::getStrs<E>();
	if (const auto it = std::find(strs.begin(), strs.end(), value); it != strs.end()) {
		return {enums.at(std::distance(strs.begin(), it))};
	}
	return std::nullopt;
}

template<typename E>
[[nodiscard]] const std::vector<E>& enum_values() {
	return detail::getEnums<E>();
}

template<typename E>
[[nodiscard]] std::vector<std::pair<E, std::string_view>> enum_entries() {
	std::vector<std::pair<E, std::string_view>> out;
	const auto& enums = detail::getEnums<E>();
	const auto& strs = detail::getStrs<E>();
	for (int i = 0; i < enums.size(); i++) {
		out.push_back(std::make_pair(enums.at(i), strs.at(i)));
	}
	return out;
}

} // namespace not_magic_enum
