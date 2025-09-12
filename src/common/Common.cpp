#include "Common.h"

std::array<std::string_view, 32> getPrettyFlagNamesFor(uint16_t minorVersion, vtfpp::VTF::Platform platform) {
	std::array<std::string_view, 32> flags{
		"Point Sample",
		"Trilinear",
		"Clamp S",
		"Clamp T",
		"Anisotropic",
		"Hint DXT5",
		"Unused (1<<6)",
		"Normal",
		"No Mips",
		"No LOD",
		"Min Mip",
		"Procedural",
		"One-bit Alpha",
		"Multi-bit Alpha",
		"Envmap",
		"Rendertarget",
		"Depth Rendertarget",
		"No Debug Override",
		"Single Copy",
		"Unused (1<<19)",
		"Unused (1<<20)",
		"Unused (1<<21)",
		"Unused (1<<22)",
		"Unused (1<<23)",
		"Unused (1<<24)",
		"Unused (1<<25)",
		"Unused (1<<26)",
		"Unused (1<<27)",
		"Unused (1<<28)",
		"Unused (1<<29)",
		"Unused (1<<30)",
		"Unused (1<<31)",
	};
	if (minorVersion >= 2) {
		flags[23] = "No Depth Buffer";
		flags[25] = "Clamp U";
		if (platform == vtfpp::VTF::Platform::PLATFORM_XBOX) {
			flags[27] = "[XBOX] Cacheable";
			flags[28] = "[XBOX] Unfilterable OK";
		}
	}
	if (minorVersion >= 3) {
		flags[10] = "Load All Mips";
		flags[26] = "Vertex Texture";
		flags[27] = "SSBump";
		flags[28] = "Unused (1<<28)";
		flags[29] = "Border";
	}
	if (minorVersion >= 4) {
		flags[6] = "sRGB";

		flags[19] = "[TF2] Staging Memory";
		flags[20] = "[TF2] Immediate Cleanup";
		flags[21] = "[TF2] Ignore mat_picmip";
		flags[30] = "[TF2] Streamable (Coarse)";
	}
	if (minorVersion >= 5) {
		flags[6] = "PWL Corrected";
		flags[10] = "Unused (1<<10)";
		flags[19] = "sRGB";
		flags[20] = "Default Pool";
		flags[28] = "Load Most Mips";

		flags[21] = "[CS:GO] Combined";
		flags[22] = "[CS:GO] Async Download";
		flags[24] = "[CS:GO] Skip Initial Download";
		flags[30] = "[CS:GO] YCoCg";
		flags[31] = "[CS:GO] Async Skip Initial Low Res";
	}
	if (minorVersion >= 6) {
		flags[21] = "Combined";
		flags[22] = "Async Download";
		flags[24] = "Skip Initial Download";
		flags[30] = "YCoCg";
		flags[31] = "Async Skip Initial Low Res";
	}
	return flags;
}
