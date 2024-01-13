﻿//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//
#include "core/stdafx.h"
#include "tier0/commandline.h"
#include "windows/id3dx.h"
#include "windows/input.h"
#include "engine/sys_mainwind.h"
#include "engine/sys_engine.h"
#include "engine/keys.h"
#include "gameui/IConsole.h"
#include "gameui/IBrowser.h"

//-----------------------------------------------------------------------------
// Purpose: plays the startup video's
//-----------------------------------------------------------------------------
void CGame::PlayStartupVideos(void)
{
	if (!CommandLine()->CheckParm("-novid"))
	{
		CGame__PlayStartupVideos();
	}
}

//-----------------------------------------------------------------------------
// Purpose: main windows procedure
//-----------------------------------------------------------------------------
int CGame::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!g_bImGuiInitialized)
		return CGame__WindowProc(hWnd, uMsg, wParam, lParam);

	const IEngine::EngineState_t state = g_pEngine->GetState();

	if (state == IEngine::DLL_CLOSE ||
		state == IEngine::DLL_RESTART)
		return CGame__WindowProc(hWnd, uMsg, wParam, lParam);

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
	{
		if (wParam == g_pImGuiConfig->m_ConsoleConfig.m_nBind0 ||
			wParam == g_pImGuiConfig->m_ConsoleConfig.m_nBind1)
		{
			g_pConsole->m_bActivate ^= true;
			ResetInput(); // Disable input to game when console is drawn.
		}

		if (wParam == g_pImGuiConfig->m_BrowserConfig.m_nBind0 ||
			wParam == g_pImGuiConfig->m_BrowserConfig.m_nBind1)
		{
			g_pBrowser->m_bActivate ^= true;
			ResetInput(); // Disable input to game when browser is drawn.
		}
	}

	if (g_pConsole->m_bActivate || g_pBrowser->m_bActivate)
	{//////////////////////////////////////////////////////////////////////////////
		g_bBlockInput = true;

		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_MOUSEACTIVATE:
		case WM_MOUSEHOVER:
		case WM_MOUSEHWHEEL:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_SETCURSOR:
			uMsg = WM_NULL;
			break;
		default:
			break;
		}
	}//////////////////////////////////////////////////////////////////////////////
	else
	{
		g_bBlockInput = false;
	}

	return CGame__WindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Purpose: gets the window rect
//-----------------------------------------------------------------------------
void CGame::GetWindowRect(int* const x, int* const y, int* const w, int* const h) const
{
	if (x)
	{
		*x = m_x;
	}
	if (y)
	{
		*y = m_y;
	}
	if (w)
	{
		*w = m_width;
	}
	if (h)
	{
		*h = m_height;
	}
}

//-----------------------------------------------------------------------------
// Purpose: dispatch key event
//-----------------------------------------------------------------------------
void CGame::DispatchKeyEvent(const uint64_t currentTick, const ButtonCode_t buttonCode) const
{
	// Controller 'hold' keys are delayed longer.
	// TODO[ AMOS ]: use ConVar's instead?
	const float delay = buttonCode == KEY_XBUTTON_BACK ? 1.0f : 0.2f;
	KeyInfo_t& keyInfo = g_pKeyInfo[buttonCode];

	if (!keyInfo.m_bKeyDown && ((currentTick - keyInfo.m_nEventTick) * 0.001f) >= delay)
	{
		KeyEvent_t keyEvent;

		keyEvent.m_pCommand = keyInfo.m_pKeyBinding[KeyInfo_t::KEY_HELD_BIND];
		keyEvent.m_nTick = buttonCode;
		keyEvent.m_bDown = true;

		v_Key_Event(keyEvent);
		keyInfo.m_bKeyDown = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: dispatch all the queued up messages
//-----------------------------------------------------------------------------
void CGame::DispatchAllStoredGameMessages() const
{
	const uint64_t ticks = Plat_MSTime();
	const short eventCount = *g_nKeyEventCount;

	for (short i = 0; i < eventCount; i++)
	{
		DispatchKeyEvent(ticks, g_pKeyEventTicks[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////
void VGame::Detour(const bool bAttach) const
{
	DetourSetup(&CGame__PlayStartupVideos, &CGame::PlayStartupVideos, bAttach);
	DetourSetup(&CGame__WindowProc, &CGame::WindowProc, bAttach);
}
