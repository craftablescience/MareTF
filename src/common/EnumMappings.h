#pragma once

#include <algorithm>
#include <concepts>
#include <optional>
#include <string_view>
#include <vector>

#include <sourcepp/String.h>
#include <vtfpp/vtfpp.h>

namespace not_magic_enum {

template<typename E>
concept SupportedEnum = std::same_as<E, vtfpp::HOT::Rect::Flags>
                     || std::same_as<E, vtfpp::ImageFormat>
                     || std::same_as<E, vtfpp::ImageConversion::FileFormat>
                     || std::same_as<E, vtfpp::ImageConversion::ResizeFilter>
                     || std::same_as<E, vtfpp::ImageConversion::ResizeMethod>
                     || std::same_as<E, vtfpp::CompressionMethod>
                     || std::same_as<E, vtfpp::Resource::Type>
                     || std::same_as<E, vtfpp::VTF::Flags>
                     || std::same_as<E, vtfpp::VTF::Platform>;

namespace detail {

extern const std::vector<vtfpp::HOT::Rect::Flags> HOT_RECT_FLAGS_E;
extern const std::vector<std::string_view>        HOT_RECT_FLAGS_S;
extern const std::vector<std::string_view>        HOT_RECT_FLAGS_P;

extern const std::vector<vtfpp::ImageFormat> IMAGE_FORMAT_E;
extern const std::vector<std::string_view>   IMAGE_FORMAT_S;
extern const std::vector<std::string_view>   IMAGE_FORMAT_P;

extern const std::vector<vtfpp::ImageConversion::FileFormat> IMAGE_CONVERSION_FILE_FORMAT_E;
extern const std::vector<std::string_view>                   IMAGE_CONVERSION_FILE_FORMAT_S;
extern const std::vector<std::string_view>                   IMAGE_CONVERSION_FILE_FORMAT_P;

extern const std::vector<vtfpp::ImageConversion::ResizeFilter> IMAGE_CONVERSION_RESIZE_FILTER_E;
extern const std::vector<std::string_view>                     IMAGE_CONVERSION_RESIZE_FILTER_S;
extern const std::vector<std::string_view>                     IMAGE_CONVERSION_RESIZE_FILTER_P;

extern const std::vector<vtfpp::ImageConversion::ResizeMethod> IMAGE_CONVERSION_RESIZE_METHOD_E;
extern const std::vector<std::string_view>                     IMAGE_CONVERSION_RESIZE_METHOD_S;
extern const std::vector<std::string_view>                     IMAGE_CONVERSION_RESIZE_METHOD_P;

extern const std::vector<vtfpp::CompressionMethod> COMPRESSION_METHOD_E;
extern const std::vector<std::string_view>         COMPRESSION_METHOD_S;
extern const std::vector<std::string_view>         COMPRESSION_METHOD_P;

extern const std::vector<vtfpp::Resource::Type> RESOURCE_TYPE_E;
extern const std::vector<std::string_view>      RESOURCE_TYPE_S;
extern const std::vector<std::string_view>      RESOURCE_TYPE_P;

extern const std::vector<vtfpp::VTF::Flags>  VTF_FLAGS_E;
extern const std::vector<std::string_view>   VTF_FLAGS_S;
extern const std::vector<std::string_view>   VTF_FLAGS_P;

extern const std::vector<vtfpp::VTF::Platform> VTF_PLATFORM_E;
extern const std::vector<std::string_view>     VTF_PLATFORM_S;
extern const std::vector<std::string_view>     VTF_PLATFORM_P;

template<SupportedEnum E>
[[nodiscard]] const std::vector<E>& e() {
	if constexpr (std::same_as<E, vtfpp::HOT::Rect::Flags>) {
		return HOT_RECT_FLAGS_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageFormat>) {
		return IMAGE_FORMAT_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::FileFormat>) {
		return IMAGE_CONVERSION_FILE_FORMAT_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeFilter>) {
		return IMAGE_CONVERSION_RESIZE_FILTER_E;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeMethod>) {
		return IMAGE_CONVERSION_RESIZE_METHOD_E;
	} else if constexpr (std::same_as<E, vtfpp::CompressionMethod>) {
		return COMPRESSION_METHOD_E;
	} else if constexpr (std::same_as<E, vtfpp::Resource::Type>) {
		return RESOURCE_TYPE_E;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Flags>) {
		return VTF_FLAGS_E;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Platform>) {
		return VTF_PLATFORM_E;
	} else {
		static std::vector<E> empty;
		return empty;
	}
}

template<SupportedEnum E>
[[nodiscard]] const std::vector<std::string_view>& s(bool pretty = false) {
	if constexpr (std::same_as<E, vtfpp::HOT::Rect::Flags>) {
		return pretty ? HOT_RECT_FLAGS_P : HOT_RECT_FLAGS_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageFormat>) {
		return pretty ? IMAGE_FORMAT_P : IMAGE_FORMAT_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::FileFormat>) {
		return pretty ? IMAGE_CONVERSION_FILE_FORMAT_P : IMAGE_CONVERSION_FILE_FORMAT_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeFilter>) {
		return pretty ? IMAGE_CONVERSION_RESIZE_FILTER_P : IMAGE_CONVERSION_RESIZE_FILTER_S;
	} else if constexpr (std::same_as<E, vtfpp::ImageConversion::ResizeMethod>) {
		return pretty ? IMAGE_CONVERSION_RESIZE_METHOD_P : IMAGE_CONVERSION_RESIZE_METHOD_S;
	} else if constexpr (std::same_as<E, vtfpp::CompressionMethod>) {
		return pretty ? COMPRESSION_METHOD_P : COMPRESSION_METHOD_S;
	} else if constexpr (std::same_as<E, vtfpp::Resource::Type>) {
		return pretty ? RESOURCE_TYPE_P : RESOURCE_TYPE_S;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Flags>) {
		return pretty ? VTF_FLAGS_P : VTF_FLAGS_S;
	} else if constexpr (std::same_as<E, vtfpp::VTF::Platform>) {
		return pretty ? VTF_PLATFORM_P : VTF_PLATFORM_S;
	} else {
		static std::vector<std::string_view> empty;
		return empty;
	}
}

} // namespace detail

template<SupportedEnum E>
[[nodiscard]] std::string_view enum_name(E value, bool pretty = false) {
	const auto& enums = detail::e<E>();
	const auto& strs = detail::s<E>(pretty);
	if (const auto it = std::find(enums.begin(), enums.end(), value); it != enums.end()) {
		return strs.at(std::distance(enums.begin(), it));
	}
	return "";
}

template<SupportedEnum E>
[[nodiscard]] const std::vector<std::string_view>& enum_names(bool pretty = false) {
	return detail::s<E>(pretty);
}

template<SupportedEnum E>
[[nodiscard]] std::optional<E> enum_cast(std::string_view value, bool pretty = false) {
	const auto& enums = detail::e<E>();
	const auto& strs = detail::s<E>(pretty);
	if (const auto it = std::find(strs.begin(), strs.end(), sourcepp::string::toUpper(value)); it != strs.end()) {
		return {enums.at(std::distance(strs.begin(), it))};
	}
	return std::nullopt;
}

template<SupportedEnum E>
[[nodiscard]] const std::vector<E>& enum_values() {
	return detail::e<E>();
}

template<SupportedEnum E>
[[nodiscard]] std::vector<std::pair<E, std::string_view>> enum_entries(bool pretty = false) {
	std::vector<std::pair<E, std::string_view>> out;
	const auto& enums = detail::e<E>();
	const auto& strs = detail::s<E>(pretty);
	for (int i = 0; i < enums.size(); i++) {
		out.push_back(std::make_pair(enums.at(i), strs.at(i)));
	}
	return out;
}

} // namespace not_magic_enum
