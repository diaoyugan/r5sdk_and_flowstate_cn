#pragma once
#include "tier1/keyvalues.h"
#include "common/protocol.h"
#include "engine/net.h"
#include "engine/net_chan.h"
#include "public/edict.h"
#include "engine/server/datablock_sender.h"

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
enum Reputation_t
{
	REP_NONE = 0,
	REP_REMOVE_ONLY,
	REP_MARK_BAD
};

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CServer;
class CClient;
class CClientExtended;

struct Spike_t
{
public:
	Spike_t() :
		m_nBits(0)
	{
		m_szDesc[0] = 0;
	}

	// !TODO: !unconfirmed!
	char	m_szDesc[64];
	int		m_nBits;
};

class CNetworkStatTrace
{
public:
	CNetworkStatTrace() :
		m_nStartBit(0), m_nCurBit(0), m_nMinWarningBytes(0)
	{
	}
	int						m_nStartBit;
	int						m_nCurBit;
	int						m_nMinWarningBytes;
	CUtlVector< Spike_t >	m_Records;
};

class CClientFrame
{
	// !TODO: !unconfirmed!
	int last_entity;
	int tick_count;
	CClientFrame* m_pNext;
};

///////////////////////////////////////////////////////////////////////////////

class CClient : IClientMessageHandler, INetChannelHandler
{
	friend class ServerDataBlockSender;
public:
	inline int64_t GetTeamNum() const { return m_iTeamNum; }
	inline edict_t GetHandle(void) const { return m_nHandle; }
	inline int GetUserID(void) const { return m_nUserID; }
	inline uint64_t GetNucleusID(void) const { return m_nNucleusID; }

	inline SIGNONSTATE GetSignonState(void) const { return m_nSignonState; }
	inline PERSISTENCE GetPersistenceState(void) const { return m_nPersistenceState; }
	inline CNetChan* GetNetChan(void) const { return m_NetChannel; }
	inline CServer* GetServer(void) const { return m_pServer; }

#ifndef CLIENT_DLL
	CClientExtended* GetClientExtended(void) const;
#endif // !CLIENT_DLL

	inline int GetCommandTick(void) const { return m_nCommandTick; }
	inline const char* GetServerName(void) const { return m_szServerName; }
	inline const char* GetClientName(void) const { return m_szClientName; }

	inline void SetHandle(edict_t nHandle) { m_nHandle = nHandle; }
	inline void SetUserID(uint32_t nUserID) { m_nUserID = nUserID; }
	inline void SetNucleusID(uint64_t nNucleusID) { m_nNucleusID = nNucleusID; }

	inline void SetSignonState(SIGNONSTATE nSignonState) { m_nSignonState = nSignonState; }
	inline void SetPersistenceState(PERSISTENCE nPersistenceState) { m_nPersistenceState = nPersistenceState; }
	inline void SetNetChan(CNetChan* pNetChan) { m_NetChannel = pNetChan; }

	inline bool IsConnected(void) const { return m_nSignonState >= SIGNONSTATE::SIGNONSTATE_CONNECTED; }
	inline bool IsSpawned(void) const { return m_nSignonState >= SIGNONSTATE::SIGNONSTATE_NEW; }
	inline bool IsActive(void) const { return m_nSignonState == SIGNONSTATE::SIGNONSTATE_FULL; }
	inline bool IsPersistenceAvailable(void) const { return m_nPersistenceState >= PERSISTENCE::PERSISTENCE_AVAILABLE; }
	inline bool IsPersistenceReady(void) const { return m_nPersistenceState == PERSISTENCE::PERSISTENCE_READY; }
	inline bool IsFakeClient(void) const { return m_bFakePlayer; }
	inline bool IsHumanPlayer(void) const { if (!IsConnected() || IsFakeClient()) { return false; } return true; }

	bool SendNetMsgEx(CNetMessage* pMsg, bool bLocal, bool bForceReliable, bool bVoice);

	bool Authenticate(const char* const playerName, char* const reasonBuf, const size_t reasonBufLen);
	bool Connect(const char* szName, CNetChan* pNetChan, bool bFakePlayer,
		CUtlVector<NET_SetConVar::cvar_t>* conVars, char* szMessage, int nMessageSize);
	void Disconnect(const Reputation_t nRepLvl, const char* szReason, ...);
	void Clear(void);

public: // Hook statics:
	static void VClear(CClient* pClient);
	static bool VConnect(CClient* pClient, const char* szName, CNetChan* pNetChan, bool bFakePlayer,
		CUtlVector<NET_SetConVar::cvar_t>* conVars, char* szMessage, int nMessageSize);

	static void VActivatePlayer(CClient* pClient);
	static void* VSendSnapshot(CClient* pClient, CClientFrame* pFrame, int nTick, int nTickAck);
	static bool VSendNetMsgEx(CClient* pClient, CNetMessage* pMsg, bool bLocal, bool bForceReliable, bool bVoice);

	static bool VProcessStringCmd(CClient* pClient, NET_StringCmd* pMsg);
	static bool VProcessSetConVar(CClient* pClient, NET_SetConVar* pMsg);

private:
	// Stub reimplementation to avoid the 'no overrider' compiler errors in the
	// CServer class (contains a static array of MAX_PLAYERS of this class).
	virtual void* ConnectionStart(INetChannelHandler* chan) { return nullptr; }
	virtual void ConnectionClosing(const char* reason, int unk) {}
	virtual void ConnectionCrashed(const char* reason) {}
	virtual void PacketStart(int incoming_sequence, int outgoing_acknowledged) {}
	virtual void PacketEnd(void) {};
	virtual void FileRequested(const char* fileName, unsigned int transferID) {}
	virtual void ChannelDisconnect(const char* fileName) {}

	virtual void* ProcessStringCmd(void) { return nullptr; }
	virtual void* ProcessScriptMessage(void) { return nullptr; }
	virtual void* ProcessSetConVar(void) { return nullptr; }
	virtual bool nullsub_0(void) { return false; }
	virtual char ProcessSignonState(void* msg) { return false; } // NET_SignonState
	virtual void* ProcessMove(void) { return nullptr; }
	virtual void* ProcessVoiceData(void) { return nullptr; }
	virtual void* ProcessDurangoVoiceData(void) { return nullptr; }
	virtual bool nullsub_1(void) { return false; }
	virtual void* ProcessLoadingProgress(void) { return nullptr; }
	virtual void* ProcessPersistenceRequestSave(void) { return nullptr; }
	virtual bool nullsub_2(void) { return false; }
	virtual bool nullsub_3(void) { return false; }
	virtual void* ProcessSetPlaylistVarOverride(void) { return nullptr; }
	virtual void* ProcessClaimClientSidePickup(void) { return nullptr; }
	virtual void* ProcessCmdKeyValues(void) { return nullptr; }
	virtual void* ProcessClientTick(void) { return nullptr; }
	virtual void* ProcessClientSayText(void) { return nullptr; }
	virtual bool nullsub_4(void) { return false; }
	virtual bool nullsub_5(void) { return false; }
	virtual bool nullsub_6(void) { return false; }
	virtual void* ProcessScriptMessageChecksum(void) { return nullptr; }

private:
	uint32_t m_nUserID;
	edict_t m_nHandle;
	char m_szServerName[256];
	char m_szClientName[256];
	char m_szMachineName[256];
	int m_nCommandTick;
	bool m_bUsePersistence_MAYBE;
	char pad_0016[59];
	int64_t m_iTeamNum;
	KeyValues* m_ConVars;
	bool m_bConVarsChanged;
	bool m_bSendServerInfo;
	bool m_bSendSignonData;
	bool m_bFullStateAchieved;
	char pad_0368[4];
	CServer* m_pServer;
	char pad_0378[20];
	int m_nDisconnectTick;
	bool m_bKickedByFairFight_MAYBE;
	char pad_0398[3];
	int m_nSendtableCRC;
	int m_nMmDev;
	char pad_039C[4];
	CNetChan* m_NetChannel;
	char pad_03A8[8];
	SIGNONSTATE m_nSignonState;
	int unk0;
	uint64_t m_nNucleusID;
	int unk1;
	int unk2;
	int m_nDeltaTick;
	int m_nStringTableAckTick;
	int m_nSignonTick;
	int m_nBaselineUpdateTick_MAYBE;
	char pad_03C0[448];
	int unk3;
	int m_nForceWaitForTick;
	bool m_bFakePlayer;
	bool m_bReceivedPacket;
	bool m_bLowViolence;
	bool m_bFullyAuthenticated;
	char pad_05A4[24];
	PERSISTENCE m_nPersistenceState;
	char pad_05C0[48];
	ServerDataBlock m_DataBlock;
	char pad_4A3D8[60];
	int m_LastMovementTick;
	char pad_4A418[86];
	char pad_4A46E[80];
};
static_assert(sizeof(CClient) == 0x4A4C0);

//-----------------------------------------------------------------------------
// Extended CClient class
//-----------------------------------------------------------------------------
// NOTE: since we interface directly with the engine, we cannot modify the
// client structure. In order to add new data to each client instance, we
// need to use this new class which we link directly to the corresponding
// client instance through its UserID.
//-----------------------------------------------------------------------------
class CClientExtended
{
	friend class CClient;
public:
	CClientExtended(void)
	{
		Reset();
	}
	inline void Reset(void)
	{
		m_flNetProcessingTimeMsecs = 0.0;
		m_flNetProcessTimeBase = 0.0;
		m_flLastClockSyncTime = 0.0;
		m_flStringCommandQuotaTimeStart = 0.0;
		m_nStringCommandQuotaCount = NULL;
		m_bRetryClockSync = false;
		m_bInitialConVarsSet = false;
	}

public: // Inlines:
	inline void SetNetProcessingTimeMsecs(const double flStartTime, const double flCurrentTime)
	{ m_flNetProcessingTimeMsecs = (flCurrentTime * 1000) - (flStartTime * 1000); }
	inline double GetNetProcessingTimeMsecs(void) const { return m_flNetProcessingTimeMsecs; }

	inline void SetNetProcessingTimeBase(const double flTime) { m_flNetProcessTimeBase = flTime; }
	inline double GetNetProcessingTimeBase(void) const { return m_flNetProcessTimeBase; }

	inline void SetLastClockSyncTime(const double flTime) { m_flLastClockSyncTime = flTime; }
	inline double GetLastClockSyncTime(void) const { return m_flLastClockSyncTime; }

	inline void SetStringCommandQuotaTimeStart(const double flTime) { m_flStringCommandQuotaTimeStart = flTime; }
	inline double GetStringCommandQuotaTimeStart(void) const { return m_flStringCommandQuotaTimeStart; }

	inline void SetStringCommandQuotaCount(const int iCount) { m_nStringCommandQuotaCount = iCount; }
	inline int GetStringCommandQuotaCount(void) const { return m_nStringCommandQuotaCount; }

	inline void SetRetryClockSync(const bool bSet) { m_bRetryClockSync = bSet; }
	inline bool ShouldRetryClockSync() const { return m_bRetryClockSync; }

private:
	// Measure how long this client's packets took to process.
	double m_flNetProcessingTimeMsecs;
	double m_flNetProcessTimeBase;

	// When was the last clock sync?
	double m_flLastClockSyncTime;

	// The start time of the first stringcmd since reset.
	double m_flStringCommandQuotaTimeStart;
	int m_nStringCommandQuotaCount;

	bool m_bRetryClockSync;    // Whether or not we should retry sending clock sync msg.
	bool m_bInitialConVarsSet; // Whether or not the initial ConVar KV's are set
};

/* ==== CBASECLIENT ===================================================================================================================================================== */
inline bool(*CClient__Connect)(CClient* pClient, const char* szName, CNetChan* pNetChan, bool bFakePlayer, CUtlVector<NET_SetConVar::cvar_t>* conVars, char* szMessage, int nMessageSize);
inline bool(*CClient__Disconnect)(CClient* pClient, const Reputation_t nRepLvl, const char* szReason, ...);
inline void(*CClient__Clear)(CClient* pClient);
inline void(*CClient__ActivatePlayer)(CClient* pClient);
inline bool(*CClient__SetSignonState)(CClient* pClient, SIGNONSTATE signon);
inline bool(*CClient__SendNetMsgEx)(CClient* pClient, CNetMessage* pMsg, bool bLocal, bool bForceReliable, bool bVoice);
inline void*(*CClient__SendSnapshot)(CClient* pClient, CClientFrame* pFrame, int nTick, int nTickAck);
inline bool(*CClient__ProcessStringCmd)(CClient* pClient, NET_StringCmd* pMsg);
inline bool(*CClient__ProcessSetConVar)(CClient* pClient, NET_SetConVar* pMsg);

///////////////////////////////////////////////////////////////////////////////
class VClient : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("CClient::Connect", CClient__Connect);
		LogFunAdr("CClient::Disconnect", CClient__Disconnect);
		LogFunAdr("CClient::Clear", CClient__Clear);
		LogFunAdr("CClient::ActivatePlayer", CClient__ActivatePlayer);
		LogFunAdr("CClient::SetSignonState", CClient__SetSignonState);
		LogFunAdr("CClient::SendNetMsgEx", CClient__SendNetMsgEx);
		LogFunAdr("CClient::SendSnapshot", CClient__SendSnapshot);
		LogFunAdr("CClient::ProcessStringCmd", CClient__ProcessStringCmd);
		LogFunAdr("CClient::ProcessSetConVar", CClient__ProcessSetConVar);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 56 57 41 56 48 83 EC 20 41 0F B6 E9").GetPtr(CClient__Connect);
		g_GameDll.FindPatternSIMD("48 8B C4 4C 89 40 18 4C 89 48 20 53 56 57 48 81 EC ?? ?? ?? ?? 83 B9 ?? ?? ?? ?? ?? 49 8B F8 8B F2").GetPtr(CClient__Disconnect);
		g_GameDll.FindPatternSIMD("40 53 41 56 41 57 48 83 EC 20 48 8B D9 48 89 74").GetPtr(CClient__Clear);
		g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 8B 81 B0 03 ?? ?? 48 8B D9 C6").GetPtr(CClient__ActivatePlayer);
		g_GameDll.FindPatternSIMD("40 53 55 56 57 41 56 48 83 EC 40 48 8B 05 ?? ?? ?? ??").GetPtr(CClient__SendNetMsgEx);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 56 41 55 41 56 41 57 48 8D 6C 24 ??").GetPtr(CClient__SendSnapshot);
		g_GameDll.FindPatternSIMD("48 89 6C 24 ?? 57 48 81 EC ?? ?? ?? ?? 48 8B 7A 20").GetPtr(CClient__ProcessStringCmd);

		g_GameDll.FindPatternSIMD("48 83 EC 28 48 83 C2 20").GetPtr(CClient__ProcessSetConVar);
		g_GameDll.FindPatternSIMD("48 8B C4 48 89 58 10 48 89 70 18 57 48 81 EC ?? ?? ?? ?? 0F 29 70 E8 8B F2").GetPtr(CClient__SetSignonState);
	}
	virtual void GetVar(void) const { }
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const;
};
///////////////////////////////////////////////////////////////////////////////
