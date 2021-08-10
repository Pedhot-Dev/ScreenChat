#include "main.h"
#include <filesystem>
#include <string>
#include <samp.hpp>
#include <callfunc.hpp>
#include <d3dx9.h>

namespace fs = std::filesystem;
using namespace std::string_literals;

AsiPlugin::AsiPlugin() : SRDescent( nullptr ) {
	kid = g_class.events->onKeyPressed += std::tuple{ this, &AsiPlugin::onKeyPressed };
}

AsiPlugin::~AsiPlugin() {
	g_class.events->onKeyPressed -= kid;
}

void AsiPlugin::onKeyPressed( int key ) {
	if ( key != VK_F2 || !g_class.events->isKeyDown( VK_LSHIFT ) ) return;

	fs::path screenPath("chat_screens");
	if ( !fs::exists( screenPath ) ) { fs::create_directories( screenPath ); }

	auto time = ::time( 0 );
	auto lt = ::localtime( &time );
	char timeBuf[128];
	sprintf( timeBuf, "%02d.%02d.%02d", lt->tm_hour, lt->tm_min, lt->tm_sec );

	auto screenName = screenPath / ( timeBuf + ".png"s );
	if ( fs::exists( screenName ) ) {
		int i = 2;
		while ( true ) {
			if ( !fs::exists( screenPath / ( timeBuf + "_"s + std::to_string( i ) + ".png" ) ) ) break;
			++i;
		}
		screenName = screenPath / ( timeBuf + "_"s + std::to_string( i ) + ".png" );
	}

#pragma pack( push, 1 )
	struct ChatInfo {
		char pad[0x12E];
		long m_lChatWindowBottom;
		char pad2[0x628C];
		IDirect3DSurface9 *m_pSurface;
	};
#pragma pack( pop )

	auto chat = *(ChatInfo **)( SAMP::Library() + ( SAMP::isR1() ? 0x21A0E4 : 0x26E8C8 ) );
	if ( !chat ) return;

	D3DSURFACE_DESC desc;
	chat->m_pSurface->GetDesc(&desc);
	auto width = desc.Width < g_class.params.BackBufferWidth ? desc.Width : g_class.params.BackBufferWidth;
	RECT rect{0, 0, static_cast<LONG>(width), chat->m_lChatWindowBottom};
	D3DXSaveSurfaceToFileW( screenName.c_str(), D3DXIFF_PNG, chat->m_pSurface, nullptr, &rect );
	SAMP::Chat::Instance()->addMsgInfo("Save chat screen to "s + screenName.string());
}
