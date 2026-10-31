// Isolated from the precompiled header: the project's legacy macros break the
// DXGI types that wincodec.h includes.
#include <windows.h>
#include <wincodec.h>
#include "ScreenshotJpeg.h"

#pragma comment(lib, "windowscodecs.lib")

namespace {
template<class Interface>
struct Owned {
	Interface* value = nullptr;
	Owned() = default;
	Owned(const Owned&) = delete;
	Owned& operator=(const Owned&) = delete;
	~Owned() { if (value) value->Release(); }
};

HRESULT EncodeBgr24Jpeg(IWICImagingFactory* factory, const wchar_t* path, UINT width, UINT height,
	const std::vector<std::uint8_t>& pixels)
{
	Owned<IWICStream> stream;
	HRESULT hr = factory->CreateStream(&stream.value);
	if (SUCCEEDED(hr)) hr = stream.value->InitializeFromFilename(path, GENERIC_WRITE);
	Owned<IWICBitmapEncoder> encoder;
	if (SUCCEEDED(hr)) hr = factory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &encoder.value);
	if (SUCCEEDED(hr)) hr = encoder.value->Initialize(stream.value, WICBitmapEncoderNoCache);
	Owned<IWICBitmapFrameEncode> frame;
	Owned<IPropertyBag2> options;
	if (SUCCEEDED(hr)) hr = encoder.value->CreateNewFrame(&frame.value, &options.value);
	if (SUCCEEDED(hr)) hr = frame.value->Initialize(options.value);
	if (SUCCEEDED(hr)) hr = frame.value->SetSize(width, height);
	WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
	if (SUCCEEDED(hr)) hr = frame.value->SetPixelFormat(&format);
	if (SUCCEEDED(hr) && format != GUID_WICPixelFormat24bppBGR) hr = WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
	if (SUCCEEDED(hr)) hr = frame.value->WritePixels(height, width * 3, static_cast<UINT>(pixels.size()),
		const_cast<BYTE*>(pixels.data()));
	if (SUCCEEDED(hr)) hr = frame.value->Commit();
	if (SUCCEEDED(hr)) hr = encoder.value->Commit();
	return hr;
}
}

HRESULT SaveBgr24Jpeg(const char* fileName, UINT width, UINT height, const std::vector<std::uint8_t>& pixels)
{
	if (!fileName || !width || !height || pixels.size() != static_cast<size_t>(width) * height * 3) return E_INVALIDARG;
	wchar_t path[MAX_PATH]{};
	if (!MultiByteToWideChar(CP_ACP, 0, fileName, -1, path, MAX_PATH)) return HRESULT_FROM_WIN32(GetLastError());
	const HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(com) && com != RPC_E_CHANGED_MODE) return com;
	HRESULT hr;
	{
		Owned<IWICImagingFactory> factory;
		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory.value));
		if (SUCCEEDED(hr)) hr = EncodeBgr24Jpeg(factory.value, path, width, height, pixels);
	}
	if (FAILED(hr)) DeleteFileW(path);
	if (SUCCEEDED(com)) CoUninitialize();
	return hr;
}
