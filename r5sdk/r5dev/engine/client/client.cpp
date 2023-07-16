//===============================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//
//===============================================================================//
// client.cpp: implementation of the CClient class.
//
///////////////////////////////////////////////////////////////////////////////////
#include "core/stdafx.h"
#include "tier1/cvar.h"
#include "tier1/strtools.h"
#include "engine/server/server.h"
#include "engine/client/client.h"

//---------------------------------------------------------------------------------
// Purpose: checks if this client is an actual human player
// Output : true if human, false otherwise
//---------------------------------------------------------------------------------
bool CClient::IsHumanPlayer(void) const
{
	if (!IsConnected())
		return false;

	if (IsFakeClient())
		return false;

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: throw away any residual garbage in the channel
//---------------------------------------------------------------------------------
void CClient::Clear(void)
{
#ifndef CLIENT_DLL
	g_ServerPlayer[GetUserID()].Reset(); // Reset ServerPlayer slot.
#endif // !CLIENT_DLL
	v_CClient_Clear(this);
}

//---------------------------------------------------------------------------------
// Purpose: throw away any residual garbage in the channel
// Input  : *pClient - 
//---------------------------------------------------------------------------------
void CClient::VClear(CClient* pClient)
{
#ifndef CLIENT_DLL
	g_ServerPlayer[pClient->GetUserID()].Reset(); // Reset ServerPlayer slot.
#endif // !CLIENT_DLL
	v_CClient_Clear(pClient);
}

//---------------------------------------------------------------------------------
// Purpose: connect new client
// Input  : *szName - 
//			*pNetChannel - 
//			bFakePlayer - 
//			*a5 - 
//			*szMessage -
//			nMessageSize - 
// Output : true if connection was successful, false otherwise
//---------------------------------------------------------------------------------
bool CClient::Connect(const char* szName, void* pNetChannel, bool bFakePlayer, void* a5, char* szMessage, int nMessageSize)
{
	return v_CClient_Connect(this, szName, pNetChannel, bFakePlayer, a5, szMessage, nMessageSize);
}

//---------------------------------------------------------------------------------
// Purpose: connect new client
// Input  : *pClient - 
//			*szName - 
//			*pNetChannel - 
//			bFakePlayer - 
//			*a5 - 
//			*szMessage -
//			nMessageSize - 
// Output : true if connection was successful, false otherwise
//---------------------------------------------------------------------------------
bool CClient::VConnect(CClient* pClient, const char* szName, void* pNetChannel, bool bFakePlayer, void* a5, char* szMessage, int nMessageSize)
{
	bool bResult = v_CClient_Connect(pClient, szName, pNetChannel, bFakePlayer, a5, szMessage, nMessageSize);
#ifndef CLIENT_DLL
	g_ServerPlayer[pClient->GetUserID()].Reset(); // Reset ServerPlayer slot.
#endif // !CLIENT_DLL
	return bResult;
}

//---------------------------------------------------------------------------------
// Purpose: disconnect client
// Input  : nRepLvl - 
//			*szReason - 
//			... - 
//---------------------------------------------------------------------------------
void CClient::Disconnect(const Reputation_t nRepLvl, const char* szReason, ...)
{
	if (m_nSignonState != SIGNONSTATE::SIGNONSTATE_NONE)
	{
		char szBuf[1024];
		{/////////////////////////////
			va_list vArgs;
			va_start(vArgs, szReason);

			vsnprintf(szBuf, sizeof(szBuf), szReason, vArgs);

			szBuf[sizeof(szBuf) - 1] = '\0';
			va_end(vArgs);
		}/////////////////////////////
		v_CClient_Disconnect(this, nRepLvl, szBuf);
	}
}

//---------------------------------------------------------------------------------
// Purpose: activate player
// Input  : *pClient - 
//---------------------------------------------------------------------------------
void CClient::VActivatePlayer(CClient* pClient)
{
	pClient->SetPersistenceState(PERSISTENCE::PERSISTENCE_READY); // Set the client instance to 'ready'.
	int nUserID = pClient->GetUserID();

	if (!g_ServerPlayer[nUserID].m_bPersistenceEnabled && sv_showconnecting->GetBool())
	{
		g_ServerPlayer[nUserID].m_bPersistenceEnabled = true;
		CNetChan* pNetChan = pClient->GetNetChan();

		DevMsg(eDLL_T::SERVER, "Enabled persistence for client #%d; channel %s(%s) ('%llu')\n",
			nUserID, pNetChan->GetName(), pNetChan->GetAddress(), pClient->GetNucleusID());
	}	///////////////////////////////////////////////////////////////////////

	v_CClient_ActivatePlayer(pClient);
}

//---------------------------------------------------------------------------------
// Purpose: send a net message with replay.
//			set 'CNetMessage::m_nGroup' to 'NoReplay' to disable replay.
// Input  : *pMsg - 
//			bLocal - 
//			bForceReliable - 
//			bVoice - 
//---------------------------------------------------------------------------------
bool CClient::SendNetMsgEx(CNetMessage* pMsg, char bLocal, bool bForceReliable, bool bVoice)
{
	if (!ShouldReplayMessage(pMsg))
	{
		// Don't copy the message into the replay buffer.
		pMsg->m_nGroup = NetMessageGroup::NoReplay;
	}

	return v_CClient_SendNetMsgEx(this, pMsg, bLocal, bForceReliable, bVoice);
}

//---------------------------------------------------------------------------------
// Purpose: send a snapshot
// Input  : *pClient - 
//			*pFrame - 
//			nTick - 
//			nTickAck - 
//---------------------------------------------------------------------------------
void* CClient::VSendSnapshot(CClient* pClient, CClientFrame* pFrame, int nTick, int nTickAck)
{
	return v_CClient_SendSnapshot(pClient, pFrame, nTick, nTickAck);
}

//---------------------------------------------------------------------------------
// Purpose: process string commands (kicking anyone attempting to DOS)
// Input  : *pClient - (ADJ)
//			*pMsg - 
// Output : false if cmd should be passed to CServerGameClients
//---------------------------------------------------------------------------------
bool CClient::VProcessStringCmd(CClient* pClient, NET_StringCmd* pMsg)
{
#ifndef CLIENT_DLL
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
	CClient* pClient_Adj = pClient;
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
	/* Original function called method "CClient::ExecuteStringCommand" with an optimization
	 * that shifted the 'this' pointer with 8 bytes.
	 * Since this has been inlined with "CClient::ProcessStringCmd" as of S2, the shifting
	 * happens directly to anything calling this function. */
	char* pShifted = reinterpret_cast<char*>(pClient) - 8;
	CClient* pClient_Adj = reinterpret_cast<CClient*>(pShifted);
#endif // !GAMEDLL_S0 || !GAMEDLL_S1
	int nUserID = pClient_Adj->GetUserID();
	ServerPlayer_t* pSlot = &g_ServerPlayer[nUserID];

	double flStartTime = Plat_FloatTime();
	int nCmdQuotaLimit = sv_quota_stringCmdsPerSecond->GetInt();

	if (!nCmdQuotaLimit)
		return true;

	const char* pCmd = pMsg->cmd;
	// Just skip if the cmd pointer is null, we still check if the
	// client sent too many commands and take appropriate actions.
	// The internal function discards the command if it's null.
	if (pCmd && !V_IsValidUTF8(pCmd))
	{
		Warning(eDLL_T::SERVER, "Removing client '%s' from slot #%i ('%llu' sent invalid string command!)\n",
			pClient_Adj->GetNetChan()->GetAddress(), pClient_Adj->GetUserID(), pClient_Adj->GetNucleusID());

		pClient_Adj->Disconnect(Reputation_t::REP_MARK_BAD, "#DISCONNECT_INVALID_STRINGCMD");
		return true;
	}

	if (flStartTime - pSlot->m_flStringCommandQuotaTimeStart >= 1.0)
	{
		pSlot->m_flStringCommandQuotaTimeStart = flStartTime;
		pSlot->m_nStringCommandQuotaCount = 0;
	}
	++pSlot->m_nStringCommandQuotaCount;

	if (pSlot->m_nStringCommandQuotaCount > nCmdQuotaLimit)
	{
		Warning(eDLL_T::SERVER, "Removing client '%s' from slot #%i ('%llu' exceeded string command quota!)\n",
			pClient_Adj->GetNetChan()->GetAddress(), pClient_Adj->GetUserID(), pClient_Adj->GetNucleusID());

		pClient_Adj->Disconnect(Reputation_t::REP_MARK_BAD, "#DISCONNECT_STRINGCMD_OVERFLOW");
		return true;
	}
#endif // !CLIENT_DLL

	return v_CClient_ProcessStringCmd(pClient, pMsg);
}

//---------------------------------------------------------------------------------
// Purpose: internal hook to 'CClient::SendNetMsgEx'
// Input  : *pClient - 
//			*pMsg - 
//			bLocal - 
//			bForceReliable - 
//			bVoice - 
//---------------------------------------------------------------------------------
bool CClient::VSendNetMsgEx(CClient* pClient, CNetMessage* pMsg, char bLocal, bool bForceReliable, bool bVoice)
{
	return pClient->SendNetMsgEx(pMsg, bLocal, bForceReliable, bVoice);
}
