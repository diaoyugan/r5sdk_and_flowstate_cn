#pragma once

typedef vector<std::pair<string, uint64_t>> BannedVec_t;

enum EKickType
{
	KICK_NAME = 0,
	KICK_ID,
	BAN_NAME,
	BAN_ID
};

class CBanSystem
{
public:
	void Load(void);
	void Save(void) const;

	bool AddEntry(const char* ipAddress, const uint64_t nucleusId);
	bool DeleteEntry(const char* ipAddress, const uint64_t nucleusId);

	bool IsBanned(const char* ipAddress, const uint64_t nucleusId) const;
	bool IsBanListValid(void) const;

	void KickPlayerByName(const char* playerName, const char* reason = nullptr);
	void KickPlayerById(const char* playerHandle, const char* reason = nullptr);

	void BanPlayerByName(const char* playerName, const char* reason = nullptr);
	void BanPlayerById(const char* playerHandle, const char* reason = nullptr);

	void UnbanPlayer(const char* criteria);

private:
	void AuthorPlayerByName(const char* playerName, const bool bBan, const char* reason = nullptr);
	void AuthorPlayerById(const char* playerHandle, const bool bBan, const char* reason = nullptr);

	BannedVec_t m_vBanList;
};

extern CBanSystem* g_pBanSystem;
