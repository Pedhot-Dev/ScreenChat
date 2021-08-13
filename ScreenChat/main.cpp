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
	SAMP::Chat::DeleteInstance();
	SAMP::Fonts::DeleteInstance();
}

void AsiPlugin::onKeyPressed( int key ) {
	if ( key != VK_F2 || !g_class.events->isKeyDown( VK_LSHIFT ) ) return;

	if ( SAMP::Version() == SAMP::eVerCode::unknown || SAMP::Version() == SAMP::eVerCode::notLoaded ) return;

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
	if ( SAMP::isR1() || SAMP::isR3() || SAMP::isDL() ) {
		x = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DB1 : ( SAMP::isR3() ? 0x67201 : 0x673F1 ) ) );
		y = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DA0 : ( SAMP::isR3() ? 0x671F0 : 0x673E0 ) ) );
	} else {
		x = 45;
		y = 10;
	}
	for ( auto i = 0; i < SAMP::Chat::Instance()->entryCount(); ++i ) {
		auto entry = (Entry *)SAMP::Chat::Instance()->entry( i );
		auto textWidth = SAMP::Fonts::Instance()->measureText( entry->szText ).width;
		if ( entry->szPrefix[0] ) textWidth += SAMP::Fonts::Instance()->measureText( entry->szPrefix ).width + 5;
		if ( textWidth > width ) width = textWidth;
	}
	if ( SAMP::Chat::Instance()->isTimestampEnabled() ) width += SAMP::Chat::Instance()->timestampWidth() + 5;

	RECT rect{ x, y, static_cast<LONG>( width + x ), SAMP::Chat::Instance()->chatWinBottom() - y };
	D3DXSaveSurfaceToFileW( screenName.c_str(), D3DXIFF_PNG, SAMP::Chat::Instance()->surface(), nullptr, &rect );
	SAMP::Chat::Instance()->addMsgInfo( "Save chat screen to "s + screenName.string() );
}
