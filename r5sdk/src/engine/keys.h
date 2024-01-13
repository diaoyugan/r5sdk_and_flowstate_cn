#ifndef ENGINE_KEYS_H
#define ENGINE_KEYS_H
#include "inputsystem/ButtonCode.h"

//-----------------------------------------------------------------------------
// Keypress event
//-----------------------------------------------------------------------------
struct KeyEvent_t
{
	const char* m_pCommand;
	int m_nTick;
	bool m_bDown;
};

//-----------------------------------------------------------------------------
// Current keypress state
//-----------------------------------------------------------------------------
struct KeyInfo_t
{
	enum
	{
		KEY_TAPPED_BIND = 0,
		KEY_HELD_BIND,

		KEY_BIND_COUNT
	};

	const char* m_pKeyBinding[KEY_BIND_COUNT];
	int m_nKeyUpTarget;
	int m_nKeyDownTarget;

	uint32_t m_nEventTick; // When was the event issued?
	int unknown;
	short m_nEventNumber; // The event number.

	bool m_bKeyDown;
	bool m_bEventIsButtonKey; // Is the event a button key (< ButtonCode_t::KEY_LAST)
	bool m_bTrapKeyUp;
	bool m_bBoundSecondKey; // Is the key bound to the second row?

	short paddingMaybe;
};

inline bool (*v_Input_Event)(const InputEvent_t& inputEvent, const int noKeyUpCheck);
inline bool(*v_Key_Event)(const KeyEvent_t& keyEvent);

extern KeyInfo_t* g_pKeyInfo;          // ARRAYSIZE = ButtonCode_t::BUTTON_CODE_LAST
extern ButtonCode_t* g_pKeyEventTicks; // ARRAYSIZE = ButtonCode_t::BUTTON_CODE_LAST
extern short* g_nKeyEventCount;

class VKeys : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("Input_Event", v_Input_Event);
		LogFunAdr("Key_Event", v_Key_Event);
		LogVarAdr("g_pKeyInfo", g_pKeyInfo);
		LogVarAdr("g_pKeyEventTicks", g_pKeyEventTicks);
		LogVarAdr("g_nKeyEventCount", g_nKeyEventCount);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 54 41 56").GetPtr(v_Input_Event);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 4C 63 41 08").GetPtr(v_Key_Event);
	}
	virtual void GetVar(void) const
	{
		g_pKeyInfo = g_GameDll.FindPatternSIMD("48 83 EC 28 33 D2 48 8D 0D ?? ?? ?? ?? 41 B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 33 C0 C6 05 ?? ?? ?? ?? ??")
			.FindPatternSelf("48 8D 0D", CMemory::Direction::DOWN, 40).ResolveRelativeAddressSelf(3, 7).RCast<KeyInfo_t*>();

		CMemory l_EngineApi_PumpMessages = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 48 81 EC ?? ?? ?? ?? 45 33 C9");

		// NOTE: g_nKeyEventCount's pattern is found earlier, thus searched for earlier to offset base for g_pKeyEventTicks.
		g_nKeyEventCount = l_EngineApi_PumpMessages.FindPatternSelf("0F B7 15").ResolveRelativeAddressSelf(3, 7).RCast<short*>();
		g_pKeyEventTicks = l_EngineApi_PumpMessages.FindPatternSelf("48 8D 35").ResolveRelativeAddressSelf(3, 7).RCast<ButtonCode_t*>();
	}
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const;
};

#endif // ENGINE_KEYS_H
