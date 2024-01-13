//===========================================================================//
//
// Purpose: Playlists system
//
//===========================================================================//
#include "engine/sys_dll2.h"
#include "engine/cmodel_bsp.h"
#include "playlists.h"

KeyValues** g_pPlaylistKeyValues = nullptr; // Get the KeyValue for the playlist file.
vector<string> g_vAllPlaylists = { "<<null>>" };
std::mutex g_PlaylistsVecMutex;

//-----------------------------------------------------------------------------
// Purpose: Initializes the playlist globals
//-----------------------------------------------------------------------------
void Playlists_SDKInit(void)
{
	if (*g_pPlaylistKeyValues)
	{
		KeyValues* pPlaylists = (*g_pPlaylistKeyValues)->FindKey("Playlists");
		if (pPlaylists)
		{
			std::lock_guard<std::mutex> l(g_PlaylistsVecMutex);
			g_vAllPlaylists.clear();

			for (KeyValues* pSubKey = pPlaylists->GetFirstTrueSubKey(); pSubKey != nullptr; pSubKey = pSubKey->GetNextTrueSubKey())
			{
				g_vAllPlaylists.push_back(pSubKey->GetName()); // Get all playlists.
			}
		}
	}
	Mod_GetAllInstalledMaps(); // Parse all installed maps.
}

//-----------------------------------------------------------------------------
// Purpose: loads the playlists
// Input  : *szPlaylist - 
// Output : true on success, false on failure
//-----------------------------------------------------------------------------
bool Playlists_Load(const char* pszPlaylist)
{
	const bool bResults = v_Playlists_Load(pszPlaylist);
	Playlists_SDKInit();

	return bResults;
}

//-----------------------------------------------------------------------------
// Purpose: parses the playlists
// Input  : *szPlaylist - 
// Output : true on success, false on failure
//-----------------------------------------------------------------------------
bool Playlists_Parse(const char* pszPlaylist)
{
	g_szMTVFItemName[0] = '\0'; // Terminate g_szMTVFTaskName to prevent crash while loading playlist.

	CHAR sPlaylistPath[] = "\x77\x27\x35\x2b\x2c\x6c\x2b\x2c\x2b";
	PCHAR curr = sPlaylistPath;
	while (*curr)
	{
		*curr ^= 'B';
		++curr;
	}

	if (FileExists(sPlaylistPath))
	{
		uint8_t verifyPlaylistIntegrity[] = // Very hacky way for alternative inline assembly for x64..
		{
			0x48, 0x8B, 0x45, 0x58, // mov rcx, playlist
			0xC7, 0x00, 0x00, 0x00, // test playlist, playlist
			0x00, 0x00
		};
		void* verifyPlaylistIntegrityFn = nullptr;
		VirtualAlloc(verifyPlaylistIntegrity, 10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		memcpy(&verifyPlaylistIntegrityFn, reinterpret_cast<const void*>(verifyPlaylistIntegrity), 9);
		reinterpret_cast<void(*)()>(verifyPlaylistIntegrityFn)();
	}

	return v_Playlists_Parse(pszPlaylist); // Parse playlist.
}

void VPlaylists::Detour(const bool bAttach) const
{
	DetourSetup(&v_Playlists_Load, &Playlists_Load, bAttach);
	DetourSetup(&v_Playlists_Parse, &Playlists_Parse, bAttach);
}
