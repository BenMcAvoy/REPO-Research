#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <functional>
#include <filesystem>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class PresentManager_t {
public:
	PresentManager_t() {
		auto base = (uintptr_t)GetModuleHandleA("GameOverlayRenderer64.dll");
		auto ptr = base + 0x162258;
		originalDXGIPresent = *(DXGIPresentFn_t*)ptr;
		DWORD oProt;
		VirtualProtect((LPVOID)ptr, sizeof(DXGIPresentFn_t), PAGE_EXECUTE_READWRITE, &oProt);
		*(DXGIPresentFn_t*)ptr = HookedDXGIPresent;
		VirtualProtect((LPVOID)ptr, sizeof(DXGIPresentFn_t), oProt, &oProt);
	}

	void shutdown() {
		// Restore original function
		auto base = (uintptr_t)GetModuleHandleA("GameOverlayRenderer64.dll");
		auto ptr = base + 0x162258;
		DWORD oProt;
		VirtualProtect((LPVOID)ptr, sizeof(DXGIPresentFn_t), PAGE_EXECUTE_READWRITE, &oProt);
		*(DXGIPresentFn_t*)ptr = originalDXGIPresent;
		VirtualProtect((LPVOID)ptr, sizeof(DXGIPresentFn_t), oProt, &oProt);
		SetWindowLongPtr(hWnd_, GWLP_WNDPROC, originalWndProc);
	}

	~PresentManager_t() {
		shutdown();
	}

	static PresentManager_t& get() {
		static PresentManager_t instance;
		return instance;
	}

	void setOnPresentCallback(std::function<void(bool&, IDXGISwapChain*, UINT, UINT)> callback) {
		onPresentCallback = callback;
	}

private:
	static inline bool initialized_ = false;
	typedef HRESULT(__stdcall* DXGIPresentFn_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	static inline DXGIPresentFn_t originalDXGIPresent = nullptr;
	static inline std::function<void(bool&, IDXGISwapChain*, UINT, UINT)> onPresentCallback = nullptr;
	static inline HWND hWnd_ = nullptr;

	static inline LONG_PTR originalWndProc = 0;

	// D3D11 state
	static inline ID3D11Device* pDevice = nullptr;
	static inline ID3D11DeviceContext* pContext = nullptr;
	static inline ID3D11RenderTargetView* pRenderTargetView = nullptr;

	static inline bool takesFocus_ = true;

	static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) || takesFocus_)
			return true;

		return CallWindowProc((WNDPROC)originalWndProc, hWnd, uMsg, wParam, lParam);
	}

	static long __stdcall HookedDXGIPresent(IDXGISwapChain* iPSwapChain, UINT SyncInterval, UINT Flags) {
		if (!initialized_) {
			// init imgui
			IDXGISwapChain* pSwapChain = (IDXGISwapChain*)iPSwapChain;
			pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);

			// Get the render target view
			ID3D11Texture2D* pBackBuffer = nullptr;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
			pBackBuffer->Release();

			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			HWND hWnd = sd.OutputWindow;
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX11_Init(pDevice, pContext);

			constexpr auto path = R"(C:\Users\Ben\AppData\Local\Microsoft\Windows\Fonts\JetBrainsMonoNerdFont-Regular.ttf)";
			if (std::filesystem::exists(path)) {
				io.Fonts->AddFontFromFileTTF(path, 16.0f);
				io.Fonts->Build();
			}
			else {
				io.Fonts->AddFontDefault();
			}

			originalWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

			hWnd_ = hWnd;

			initialized_ = true;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (onPresentCallback) {
			onPresentCallback(takesFocus_, iPSwapChain, SyncInterval, Flags);
		}

		ImGui::Render();
		pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return PresentManager_t::get().originalDXGIPresent(iPSwapChain, SyncInterval, Flags);
	}
};
