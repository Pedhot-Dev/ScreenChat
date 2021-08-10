#include "loader.h"
#include "../main.h"
#include <llmo/SRHookFast.h>
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
#	include <llmo/SRHookPE.hpp>
#endif

const std::string_view PROJECT_NAME = PROJECT_NAME_C;
stGClass g_class;

[[maybe_unused]] class AsiPluginLoader {
	SRHook::Fast::Hook createDeviceHook{ 0x7F6800, 6 };
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
	SRHook::PE::Hook<SRHook::call_t::stdcall, int( UINT )> *exitProcessHook = nullptr;
#endif
	AsiPlugin *pAsiPlugin = nullptr;

public:
	AsiPluginLoader() noexcept {
		createDeviceHook.onBefore += std::tuple{ this, &AsiPluginLoader::Initialize };
		createDeviceHook.install();
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
		SRHook::PE::make_for( exitProcessHook, "kernel32", "ExitProcess" );
		exitProcessHook->onHook += std::tuple{ this, &AsiPluginLoader::ExitProcess };
		exitProcessHook->enable();
#endif
	}
	~AsiPluginLoader() noexcept {
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
		delete pAsiPlugin;
		pAsiPlugin = nullptr;
#endif
		delete g_class.cursor;
		delete g_class.events;
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
		delete exitProcessHook;
#endif
#if defined(USE_DRAW_HOOK) && __has_include(<DrawHook/DrawHook.h>)
		delete g_class.draw;
#endif
	}

protected:
	void Initialize() {
		g_class.cursor = new SRCursor();
#if defined(USE_DRAW_HOOK) && __has_include(<DrawHook/DrawHook.h>)
		if ( !g_class.draw ) g_class.draw = new DrawHook();
#endif
#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
		if ( !InstallD3DHook() ) return;
#endif

		g_class.events = new SREvents();
		pAsiPlugin = new AsiPlugin();
	}

#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
	bool InstallD3DHook() {
		static bool isDxHooked = false;
		auto device = *reinterpret_cast<IDirect3DDevice9 **>( 0xC97C28 );
		if ( !device ) return false;
		if ( isDxHooked ) return true;
		isDxHooked = true;
		g_class.DirectX = new hookIDirect3DDevice9( device );
		*reinterpret_cast<IDirect3DDevice9 **>( 0xC97C28 ) = dynamic_cast<IDirect3DDevice9 *>( g_class.DirectX );
		return true;
	}
	void ExitProcess( UINT &code ) {
		delete pAsiPlugin;
		pAsiPlugin = nullptr;
		if ( g_class.DirectX->d3d9_destroy() ) delete g_class.DirectX;
	}
#endif
} g_loader;

int MessageBox( std::string_view text, std::string_view title, UINT type ) {
	return MessageBoxA( *reinterpret_cast<HWND *>( 0xC97C1C ), text.data(), title.data(), type );
}
