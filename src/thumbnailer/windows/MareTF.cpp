#include <cmath>
#include <cstddef>
#include <vector>

#include "CommonThumbnailer.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <initguid.h>
#include <ShlObj_core.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include <thumbcache.h>
#include <wrl.h>

#include "Config.h"

#define MARETF_THUMBNAILER_CLSID_STR  L"{8b206795-0606-40ca-9eac-1d049c7ff3bf}"
DEFINE_GUID(MARETF_THUMBNAILER_CLSID, 0x8b206795, 0x0606, 0x40ca, 0x9e, 0xac, 0x1d, 0x04, 0x9c, 0x7f, 0xf3, 0xbf);

#define GLOBAL(ret) extern "C" [[maybe_unused]] ret __stdcall

// Thumbnailer ------------------------------------------------------------------------------------------------

class MareTFThumbnailProvider : public IThumbnailProvider, public IInitializeWithStream {
public:
	MareTFThumbnailProvider()
			: refCount(1)
			, stream(nullptr) {}

	~MareTFThumbnailProvider() {
		if (this->stream) {
			this->stream->Release();
			this->stream = nullptr;
		}
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
		static const QITAB qit[] = {
			QITABENT(MareTFThumbnailProvider, IThumbnailProvider),
			QITABENT(MareTFThumbnailProvider, IInitializeWithStream),
			{nullptr},
		};
		return QISearch(this, qit, riid, ppv);
	}

	STDMETHOD_(ULONG, AddRef)() override {
		return InterlockedIncrement(&this->refCount);
	}

	STDMETHOD_(ULONG, Release)() override {
		const ULONG rc = InterlockedDecrement(&this->refCount);
		if (rc == 0) {
			delete this;
			return 0;
		}
		return rc;
	}

	STDMETHOD(Initialize)(IStream* stream_, DWORD) override {
		if (!stream_) {
			return E_POINTER;
		}
		this->stream = stream_;
		this->stream->AddRef();
		return S_OK;
	}

	STDMETHOD(GetThumbnail)(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha) override {
		if (!phbmp || !pdwAlpha) {
			return E_POINTER;
		}
		if (!this->stream) {
			return E_UNEXPECTED;
		}

		STATSTG stat;
		HRESULT hr = this->stream->Stat(&stat, STATFLAG_NONAME);
		if (FAILED(hr)) {
			return hr;
		}

		ULONG bytesRead = 0;
		std::vector<std::byte> data(stat.cbSize.QuadPart);
		hr = this->stream->Read(data.data(), static_cast<ULONG>(stat.cbSize.QuadPart), &bytesRead);
		if (FAILED(hr) || bytesRead != stat.cbSize.QuadPart) {
			return E_FAIL;
		}

		try {
			const vtfpp::VTF vtf{data};
			if (!vtf) {
				return E_UNEXPECTED;
			}

			int targetWidth = vtf.getWidth();
			int outWidth = targetWidth;
			int targetHeight = vtf.getHeight();
			int outHeight = targetHeight;
			if (cx > outWidth || cx > outHeight) {
				const double scalingFactor = min(static_cast<double>(cx) / outWidth, static_cast<double>(cx) / outHeight);
				outWidth = targetWidth = std::floor(targetWidth * scalingFactor);
				outHeight = targetHeight = std::floor(targetHeight * scalingFactor);
			}

			auto [data, format] = ::createThumbnail(vtf, outWidth, outHeight);
			if (data.empty() || format == vtfpp::ImageFormat::EMPTY) {
				return E_UNEXPECTED;
			}

			if (format != vtfpp::ImageFormat::BGRA8888) {
				data = vtfpp::ImageConversion::convertImageDataToFormat(data, format, vtfpp::ImageFormat::BGRA8888, outWidth, outHeight);
				format = vtfpp::ImageFormat::BGRA8888;
			}
			if (targetWidth != outWidth || targetHeight != outHeight) {
				data = vtfpp::ImageConversion::resizeImageData(data, format, outWidth, targetWidth, outHeight, targetHeight, !vtfpp::ImageFormatDetails::large(format), vtfpp::ImageConversion::ResizeFilter::BILINEAR);
			}

			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = static_cast<LONG>(targetWidth);
			bmi.bmiHeader.biHeight = -static_cast<LONG>(targetHeight);
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			void* pBits = nullptr;
			HDC hdc = GetDC(nullptr);
			HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
			ReleaseDC(nullptr, hdc);

			if (!hBitmap) {
				return E_OUTOFMEMORY;
			}

			std::memcpy(pBits, data.data(), data.size());

			*phbmp = hBitmap;
			*pdwAlpha = WTSAT_ARGB;

		} catch (const std::overflow_error&) {
			return E_UNEXPECTED;
		} catch (const std::runtime_error&) {
			return E_UNEXPECTED;
		}
		return S_OK;
	}

private:
	ULONG refCount;
	IStream* stream;
};

GLOBAL(HRESULT) MareTFThumbnailProvider_CreateInstance(REFIID riid, void** ppv) {
	auto* pNew = new(std::nothrow) MareTFThumbnailProvider;
	HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr)) {
		hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();
	}
	return hr;
}

// Factory ------------------------------------------------------------------------------------------------

typedef HRESULT(*PFNCREATEINSTANCE)(REFIID, void**);
struct CLASS_OBJECT_INIT {
	const CLSID* pClsid;
	PFNCREATEINSTANCE pfnCreate;
};
const CLASS_OBJECT_INIT c_rgClassObjectInit[] = {{&MARETF_THUMBNAILER_CLSID, MareTFThumbnailProvider_CreateInstance}};

HINSTANCE g_hInst = nullptr;
ULONG g_cRefModule = 0;

GLOBAL(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void*) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		g_hInst = hInstance;
		DisableThreadLibraryCalls(hInstance);
	}
	return TRUE;
}

GLOBAL(void) DllAddRef() {
	InterlockedIncrement(&g_cRefModule);
}

GLOBAL(void) DllRelease() {
	InterlockedDecrement(&g_cRefModule);
}

GLOBAL(HRESULT) DllCanUnloadNow() {
	return !g_cRefModule ? S_OK : S_FALSE;
}

class MareTFThumbnailProviderFactory : public IClassFactory {
public:
	static HRESULT CreateInstance(REFCLSID clsid, const CLASS_OBJECT_INIT* pClassObjectInits, size_t cClassObjectInits, REFIID riid, void** ppv) {
		*ppv = nullptr;
		HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
		for (size_t i = 0; i < cClassObjectInits; i++) {
			if (clsid == *pClassObjectInits[i].pClsid) {
				IClassFactory *pClassFactory = new(std::nothrow) MareTFThumbnailProviderFactory(pClassObjectInits[i].pfnCreate);
				hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
				if (SUCCEEDED(hr)) {
					hr = pClassFactory->QueryInterface(riid, ppv);
					pClassFactory->Release();
				}
				break;
			}
		}
		return hr;
	}

	explicit MareTFThumbnailProviderFactory(PFNCREATEINSTANCE pfnCreate_)
			: refCount(1)
			, pfnCreate(pfnCreate_) {
		::DllAddRef();
	}

	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
		static const QITAB qit[] = {
			QITABENT(MareTFThumbnailProviderFactory, IClassFactory),
			{nullptr},
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef() override {
		return InterlockedIncrement(&this->refCount);
	}

	IFACEMETHODIMP_(ULONG) Release() override {
		const ULONG rc = InterlockedDecrement(&this->refCount);
		if (!rc) {
			delete this;
		}
		return rc;
	}

	IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override {
		return pUnkOuter ? CLASS_E_NOAGGREGATION : this->pfnCreate(riid, ppv);
	}

	IFACEMETHODIMP LockServer(BOOL fLock) override {
		if (fLock) {
			::DllAddRef();
		} else {
			::DllRelease();
		}
		return S_OK;
	}

private:
	~MareTFThumbnailProviderFactory() {
		::DllRelease();
	}

	ULONG refCount;
	PFNCREATEINSTANCE pfnCreate;
};

GLOBAL(HRESULT) DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv) {
	return MareTFThumbnailProviderFactory::CreateInstance(clsid, c_rgClassObjectInit, ARRAYSIZE(c_rgClassObjectInit), riid, ppv);
}

// Installer ------------------------------------------------------------------------------------------------

namespace {

HRESULT SetHKCRRegistryKey(LPCWSTR subKey, LPCWSTR valueName, LPCWSTR data) {
	HKEY hKey;
	LONG result = RegCreateKeyExW(HKEY_CLASSES_ROOT, subKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
	if (result != ERROR_SUCCESS) {
		return HRESULT_FROM_WIN32(result);
	}
	result = RegSetValueExW(hKey, valueName, 0, REG_SZ, (const BYTE*) data, (lstrlenW(data) + 1) * sizeof(WCHAR));
	RegCloseKey(hKey);
	return HRESULT_FROM_WIN32(result);
}

HRESULT DeleteHKCRRegistryKey(LPCWSTR subKey) {
	LONG result = SHDeleteKeyW(HKEY_CLASSES_ROOT, subKey);
	return HRESULT_FROM_WIN32(result == ERROR_FILE_NOT_FOUND ? ERROR_SUCCESS : result);
}

} // namespace

GLOBAL(HRESULT) DllNotifyShell() {
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

GLOBAL(HRESULT) DllRegisterServer() {
	wchar_t modulePath[MAX_PATH];
	if (!GetModuleFileNameW(g_hInst, modulePath, MAX_PATH)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	HRESULT hr;
	hr = ::SetHKCRRegistryKey(L"CLSID\\" MARETF_THUMBNAILER_CLSID_STR, nullptr, L"Valve Texture Format Files (" PROJECT_NAME_PRETTY ")");
	if (FAILED(hr)) {
		return hr;
	}
	hr = ::SetHKCRRegistryKey(L"CLSID\\" MARETF_THUMBNAILER_CLSID_STR "\\InProcServer32", nullptr, modulePath);
	if (FAILED(hr)) {
		return hr;
	}
	hr = ::SetHKCRRegistryKey(L"CLSID\\" MARETF_THUMBNAILER_CLSID_STR "\\InProcServer32", L"ThreadingModel", L"Apartment");
	if (FAILED(hr)) {
		return hr;
	}
	hr = ::SetHKCRRegistryKey(L".vtf\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", nullptr, MARETF_THUMBNAILER_CLSID_STR);
	if (SUCCEEDED(hr)) {
		::DllNotifyShell();
	}
	hr = ::SetHKCRRegistryKey(L".xtf\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", nullptr, MARETF_THUMBNAILER_CLSID_STR);
	if (SUCCEEDED(hr)) {
		::DllNotifyShell();
	}
	return hr;
}

GLOBAL(HRESULT) DllUnregisterServer() {
	HRESULT hr;
	hr = ::DeleteHKCRRegistryKey(L"CLSID\\" MARETF_THUMBNAILER_CLSID_STR);
	if (FAILED(hr)) {
		return hr;
	}
	hr = ::DeleteHKCRRegistryKey(L".vtf\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}");
	if (FAILED(hr)) {
		return hr;
	}
	return ::DeleteHKCRRegistryKey(L".xtf\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}");
}
