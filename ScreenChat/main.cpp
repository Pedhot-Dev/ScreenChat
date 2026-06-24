#include "main.h"
#include <filesystem>
#include <string>
#include <samp.hpp>
#include <CDXUT/ScrollBar.h>
#include <callfunc.hpp>
#include <d3dx9.h>
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")

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

	char documentsPath[MAX_PATH];
	SHGetFolderPathA( nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, documentsPath );
	fs::path screenPath = fs::path( documentsPath ) / "GTA San Andreas User Files" / "chat_screens";
	if ( !fs::exists( screenPath ) ) { fs::create_directories( screenPath ); }

	std::time_t now = std::time(nullptr);
	std::tm* lt = std::localtime(&now);

	char timeBuf[128];
	std::sprintf(
		timeBuf,
		"%lld_%02d%02d%04d_%02d%02d%02d",
		static_cast<long long>(now),   // unix timestamp
		lt->tm_mday,                   // DD
		lt->tm_mon + 1,                // MM
		lt->tm_year + 1900,            // YYYY
		lt->tm_hour,                   // HH
		lt->tm_min,                    // MM
		lt->tm_sec                     // SS
	);

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

	auto width = 0, height = 0, x = 0, y = 0;
	if ( SAMP::isR1() || SAMP::isR3() || SAMP::isDL() ) {
		x = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DB1 : ( SAMP::isR3() ? 0x67201 : 0x673F1 ) ) );
		y = *(int *)( SAMP::Library() + ( SAMP::isR1() ? 0x63DA0 : ( SAMP::isR3() ? 0x671F0 : 0x673E0 ) ) );
	} else {
		x = 45;
		y = 10;
	}

	auto from = SAMP::Chat::Instance()->scroll()->scrollBarData.curentPos;
	auto to = from + SAMP::Chat::Instance()->scroll()->scrollBarData.pagesize;
	bool allowSkipEmpty = true;
	for ( auto i = from; i < to; ++i ) {
		auto allNextLinesEmpty = []( int from, int to ) {
			if ( from == to ) return false;
			for ( auto i = from; i < to; ++i ) {
				auto entry = (Entry *)SAMP::Chat::Instance()->entry( i );
				if ( entry->szText[0] || entry->szPrefix[0] ) {
					if ( entry->szPrefix[0] )
						return false;
					else if ( entry->szText[0] != ' ' && entry->szText[1] )
						return false;
				}
			}
			return true;
		};
		auto entry = (Entry *)SAMP::Chat::Instance()->entry( i );
		if ( ( !entry->szText[0] || ( entry->szText[0] == ' ' && !entry->szText[1] ) ) && !entry->szPrefix[0] && allowSkipEmpty ) {
			y += SAMP::Chat::Instance()->stringHeight() + 1;
			continue;
		} else {
			allowSkipEmpty = false;
			if ( allNextLinesEmpty( i + 1, to ) ) break;
		}
		auto textWidht = SAMP::Fonts::Instance()->measureText( entry->szText ).width;
		if ( entry->szPrefix[0] ) textWidht += SAMP::Fonts::Instance()->measureText( entry->szPrefix ).width + 5;
		if ( textWidht > width ) width = textWidht;
		height += SAMP::Chat::Instance()->stringHeight() + 1;
	}
	if ( SAMP::Chat::Instance()->isTimestampEnabled() ) width += SAMP::Chat::Instance()->timestampWidth() + 5;
	if ( !height ) return; // all strings is empty

	RECT rect{ x, y, width + x, height + y };
	D3DXSaveSurfaceToFileW( screenName.c_str(), D3DXIFF_PNG, SAMP::Chat::Instance()->surface(), nullptr, &rect );
	SAMP::Chat::Instance()->addMsgInfo( "Save chat screen to "s + screenName.string() );
}
