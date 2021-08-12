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
	struct Entry {
		char pad[0x4];
		char szPrefix[0x1C];
		char szText[0x90];
		char pad2[0x4C];
	};
#pragma pack( pop )

	auto width = 0, x = 0, y = 0;
	if ( SAMP::isR1() || SAMP::isR3() ) {
		for ( auto i = 0; i < SAMP::Chat::Instance()->entryCount(); ++i ) {
			auto entry = (Entry *)SAMP::Chat::Instance()->entry( i );
			auto calcFontSize = SAMP::Library() + ( SAMP::isR1() ? 0x66B20 : 0x6AA90 );
			RECT textSize{ 0, 0, 0, 0 };
			CallFunc::thiscall( SAMP::Chat::Instance()->fonts(), calcFontSize, &textSize, entry->szText, 0 );
			if ( entry->szPrefix[0] ) {
				RECT prefixSize{ 0, 0, 0, 0 };
				CallFunc::thiscall( SAMP::Chat::Instance()->fonts(), calcFontSize, &prefixSize, entry->szPrefix, 0 );
				textSize.left += prefixSize.left + 5;
			}
			if ( textSize.left > width ) width = textSize.left;
		}
		if ( SAMP::Chat::Instance()->isTimestampEnabled() ) width += SAMP::Chat::Instance()->timestampWidth() + 5;

		x = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DB1 : 0x67201 ) );
		y = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DA0 : 0x671F0 ) );
	} else {
		D3DSURFACE_DESC desc;
		SAMP::Chat::Instance()->surface()->GetDesc( &desc );
		width = desc.Width < g_class.params.BackBufferWidth ? desc.Width : g_class.params.BackBufferWidth;
		x = 45;
		y = 10;
	}

	RECT rect{ x, y, static_cast<LONG>( width + x ), SAMP::Chat::Instance()->chatWinBottom() - y };
	D3DXSaveSurfaceToFileW( screenName.c_str(), D3DXIFF_PNG, SAMP::Chat::Instance()->surface(), nullptr, &rect );
	SAMP::Chat::Instance()->addMsgInfo( "Save chat screen to "s + screenName.string() );
}
