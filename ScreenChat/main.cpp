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

	fs::path screenPath( "chat_screens" );
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
		int m_pagesize;
		char pad[0x8];
		bool m_bTimestamps;
		char pad2[0x121];
		long m_lChatWindowBottom;
		struct Entry {
			char pad[0x4];
			char szPrefix[0x1C];
			char szText[0x90];
			char pad2[0x4C];
		} m_entries[0x64];
		void *m_pFontRenderer;
		char pad3[0x18];
		IDirect3DSurface9 *m_pSurface;
		char pad4[0x24];
		int m_iTimestampWidth;
	};
#pragma pack( pop )

	auto chat = *(ChatInfo **)( SAMP::Library() + ( SAMP::isR1() ? 0x21A0E4 : 0x26E8C8 ) );
	if ( !chat ) return;

	auto width = 0;
	auto entryCount = sizeof( chat->m_entries ) / sizeof( ChatInfo::Entry );
	for ( auto i = entryCount - chat->m_pagesize; i < entryCount; ++i ) {
		auto calcFontSize = SAMP::Library() + ( SAMP::isR1() ? 0x66B20 : 0x6AA90 );
		RECT textSize{ 0, 0, 0, 0 };
		CallFunc::thiscall( chat->m_pFontRenderer, calcFontSize, &textSize, chat->m_entries[i].szText, 0 );
		if ( chat->m_entries[i].szPrefix[0] ) {
			RECT prefixSize{ 0, 0, 0, 0 };
			CallFunc::thiscall( chat->m_pFontRenderer, calcFontSize, &prefixSize, chat->m_entries[i].szPrefix, 0 );
			textSize.left += prefixSize.left + 5;
		}
		if ( textSize.left > width ) width = textSize.left;
	}
	if ( chat->m_bTimestamps ) width += chat->m_iTimestampWidth + 5;

	int x = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DB1 : 0x67201 ) );
	int y = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DA0 : 0x671F0 ) );

	RECT rect{ x, y, static_cast<LONG>( width + x ), chat->m_lChatWindowBottom - y };
	D3DXSaveSurfaceToFileW( screenName.c_str(), D3DXIFF_PNG, chat->m_pSurface, nullptr, &rect );
	SAMP::Chat::Instance()->addMsgInfo( "Save chat screen to "s + screenName.string() );
}
