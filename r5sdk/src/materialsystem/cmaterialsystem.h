#ifndef MATERIALSYSTEM_H
#define MATERIALSYSTEM_H
#include "cmaterialglue.h"
#include "public/imaterialsystem.h"

#define STREAM_DB_EXT "stbsp"

class CMaterialSystem
{
public:
	static InitReturnVal_t Init(CMaterialSystem* thisptr);
	static int Shutdown(CMaterialSystem* thisptr);
#ifndef MATERIALSYSTEM_NODX
	static CMaterialGlue* FindMaterialEx(CMaterialSystem* pMatSys, const char* pMaterialName, uint8_t nMaterialType, int nUnk, bool bComplain);
	static Vector2D GetScreenSize(CMaterialSystem* pMatSys = nullptr);
#endif // !MATERIALSYSTEM_NODX

	// TODO: reverse the vftable!
	inline int GetCurrentFrameCount()
	{
		const static int index = 74;
		return CallVFunc<int>(index, this);
	}
};

#ifndef MATERIALSYSTEM_NODX
class CMaterialDeviceMgr
{
public:
	inline const MaterialAdapterInfo_t& GetAdapterInfo(int nIndex) const
	{
		Assert(nIndex >= 0 && nIndex < SDK_ARRAYSIZE(m_AdapterInfo));
		return m_AdapterInfo[nIndex];
	}
	inline const MaterialAdapterInfo_t& GetAdapterInfo() const
	{
		// Retrieve info of the selected adapter.
		return GetAdapterInfo(m_SelectedAdapter);
	}

private:
	enum
	{
		MAX_ADAPTER_COUNT = 4
	};

	IDXGIAdapter* m_Adapters[MAX_ADAPTER_COUNT];
	void* m_pUnknown1[MAX_ADAPTER_COUNT];
	MaterialAdapterInfo_t m_AdapterInfo[MAX_ADAPTER_COUNT];
	size_t m_AdapterMemorySize[MAX_ADAPTER_COUNT];
	int m_NumDisplayAdaptersProcessed;
	int m_SelectedAdapter;
	int m_NumDisplayAdapters;
};

inline CMaterialDeviceMgr* g_pMaterialAdapterMgr = nullptr;
#endif // !MATERIALSYSTEM_NODX

/* ==== MATERIALSYSTEM ================================================================================================================================================== */
inline InitReturnVal_t(*CMaterialSystem__Init)(CMaterialSystem* thisptr);
inline int(*CMaterialSystem__Shutdown)(CMaterialSystem* thisptr);
inline void(*CMaterialSystem__Disconnect)(void);

inline CMaterialSystem* g_pMaterialSystem = nullptr;
inline void* g_pMaterialVFTable = nullptr;
#ifndef MATERIALSYSTEM_NODX
inline CMaterialGlue*(*CMaterialSystem__FindMaterialEx)(CMaterialSystem* pMatSys, const char* pMaterialName, uint8_t nMaterialType, int nUnk, bool bComplain);
inline void(*CMaterialSystem__GetScreenSize)(CMaterialSystem* pMatSys, float* outX, float* outY);

inline void*(*v_DispatchDrawCall)(int64_t a1, uint64_t a2, int a3, int a4, int64_t a5, int a6, uint8_t a7, int64_t a8, uint32_t a9, uint32_t a10, int a11, __m128* a12, int a13, int64_t a14);
inline ssize_t(*v_SpinPresent)(void);
inline void(*CMaterialSystem__GetStreamOverlay)(const char* mode, char* buf, size_t bufSize);
inline const char*(*CMaterialSystem__DrawStreamOverlay)(void* thisptr, uint8_t* a2, void* unused, void* a4);

inline void** s_pRenderContext; // NOTE: This is some CMaterial instance or array.

inline int* g_nTotalStreamingTextureMemory    = nullptr;
inline int* g_nUnfreeStreamingTextureMemory   = nullptr;
inline int* g_nUnusableStreamingTextureMemory = nullptr;
#endif // !MATERIALSYSTEM_NODX

// TODO: move to materialsystem_global.h!
// TODO: reverse the vftable!
inline CMaterialSystem* MaterialSystem()
{
	return g_pMaterialSystem;
}

///////////////////////////////////////////////////////////////////////////////
class VMaterialSystem : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogConAdr("CMaterial::`vftable'", g_pMaterialVFTable);
		LogFunAdr("CMaterialSystem::Init", CMaterialSystem__Init);
		LogFunAdr("CMaterialSystem::Shutdown", CMaterialSystem__Shutdown);
		LogFunAdr("CMaterialSystem::Disconnect", CMaterialSystem__Disconnect);
#ifndef MATERIALSYSTEM_NODX
		LogFunAdr("CMaterialSystem::FindMaterialEx", CMaterialSystem__FindMaterialEx);
		LogFunAdr("CMaterialSystem::GetScreenSize", CMaterialSystem__GetScreenSize);
		LogFunAdr("CMaterialSystem::GetStreamOverlay", CMaterialSystem__GetStreamOverlay);
		LogFunAdr("CMaterialSystem::DrawStreamOverlay", CMaterialSystem__DrawStreamOverlay);
		LogFunAdr("DispatchDrawCall", v_DispatchDrawCall);
		LogFunAdr("SpinPresent", v_SpinPresent);
		LogVarAdr("g_nTotalStreamingTextureMemory", g_nTotalStreamingTextureMemory);
		LogVarAdr("g_nUnfreeStreamingTextureMemory", g_nUnfreeStreamingTextureMemory);
		LogVarAdr("g_nUnusableStreamingTextureMemory", g_nUnusableStreamingTextureMemory);
		LogVarAdr("s_pRenderContext", s_pRenderContext);
		LogVarAdr("g_MaterialAdapterMgr", g_pMaterialAdapterMgr);
#endif // !MATERIALSYSTEM_NODX
		LogVarAdr("g_pMaterialSystem", g_pMaterialSystem);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 70 48 83 3D ?? ?? ?? ?? ??").GetPtr(CMaterialSystem__Init);
		g_GameDll.FindPatternSIMD("48 83 EC 58 48 89 6C 24 ??").GetPtr(CMaterialSystem__Shutdown);
		g_GameDll.FindPatternSIMD("48 83 EC 28 8B 0D ?? ?? ?? ?? 48 89 6C 24 ??").GetPtr(CMaterialSystem__Disconnect);
#ifndef MATERIALSYSTEM_NODX
		g_GameDll.FindPatternSIMD("44 89 4C 24 ?? 44 88 44 24 ?? 48 89 4C 24 ??").GetPtr(CMaterialSystem__FindMaterialEx);
		g_GameDll.FindPatternSIMD("8B 05 ?? ?? ?? ?? 89 02 8B 05 ?? ?? ?? ?? 41 89 ?? C3 CC CC CC CC CC CC CC CC CC CC CC CC CC CC 8B 05 ?? ?? ?? ??").GetPtr(CMaterialSystem__GetScreenSize);
		g_GameDll.FindPatternSIMD("E8 ?? ?? ?? ?? 80 7C 24 ?? ?? 0F 84 ?? ?? ?? ?? 48 89 9C 24 ?? ?? ?? ??").FollowNearCallSelf().GetPtr(CMaterialSystem__GetStreamOverlay);
		g_GameDll.FindPatternSIMD("41 56 B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 2B E0 C6 02 ??").GetPtr(CMaterialSystem__DrawStreamOverlay);

		g_GameDll.FindPatternSIMD("44 89 4C 24 ?? 44 89 44 24 ?? 48 89 4C 24 ?? 55 53 56").GetPtr(v_DispatchDrawCall);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC ?? ?? ?? ?? 8B 15 ?? ?? ?? ??").GetPtr(v_SpinPresent);
#endif // !MATERIALSYSTEM_NODX
	}
	virtual void GetVar(void) const
	{
#ifndef MATERIALSYSTEM_NODX
		g_nTotalStreamingTextureMemory = CMemory(CMaterialSystem__DrawStreamOverlay).Offset(0x1C).FindPatternSelf("48 8B 05", CMemory::Direction::DOWN).ResolveRelativeAddressSelf(0x3, 0x7).RCast<int*>();
		g_nUnfreeStreamingTextureMemory = CMemory(CMaterialSystem__DrawStreamOverlay).Offset(0x2D).FindPatternSelf("48 8B 05", CMemory::Direction::DOWN).ResolveRelativeAddressSelf(0x3, 0x7).RCast<int*>();
		g_nUnusableStreamingTextureMemory = CMemory(CMaterialSystem__DrawStreamOverlay).Offset(0x50).FindPatternSelf("48 8B 05", CMemory::Direction::DOWN).ResolveRelativeAddressSelf(0x3, 0x7).RCast<int*>();

		CMemory(v_DispatchDrawCall).FindPattern("48 8B ?? ?? ?? ?? 01").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(s_pRenderContext);
		CMemory(CMaterialSystem__Disconnect).FindPattern("48 8D").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_pMaterialAdapterMgr);
#endif // !MATERIALSYSTEM_NODX
		g_pMaterialSystem = g_GameDll.FindPatternSIMD("8B 41 28 85 C0 7F 18").FindPatternSelf("48 8D 0D").ResolveRelativeAddressSelf(3, 7).RCast<CMaterialSystem*>();
	}
	virtual void GetCon(void) const
	{
		g_pMaterialVFTable = g_GameDll.GetVirtualMethodTable(".?AVCMaterial@@").RCast<void*>();
	}
	virtual void Detour(const bool bAttach) const;
};
///////////////////////////////////////////////////////////////////////////////

#endif // MATERIALSYSTEM_H
