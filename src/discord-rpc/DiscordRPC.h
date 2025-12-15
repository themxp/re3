#pragma once

#include "common.h"

class CPlayerInfo;
class CPlayerPed;

class CDiscordRPC
{
private:
	static bool ms_bInitialized;
	static bool ms_bEnabled;
	static int64_t ms_nStartTimestamp;
	static uint32 ms_nLastUpdateTime;
	static int32 ms_nLastHealth;
	static int32 ms_nLastMoney;
	static int32 ms_nLastWantedLevel;

	static char ms_aStateBuffer[128];
	static char ms_aDetailsBuffer[128];
	static char ms_aLargeImageTextBuffer[128];
	static char ms_aSmallImageTextBuffer[128];

	static void HandleReady(const void* request);
	static void HandleDisconnected(int errorCode, const char* message);
	static void HandleErrored(int errorCode, const char* message);

	static void UpdatePresenceData(void);
	static void GetPlayerState(char* buffer, int32 bufferSize);
	static void GetPlayerDetails(char* buffer, int32 bufferSize);
	static void GetHealthText(char* buffer, int32 bufferSize, int32 health);
	static void GetMoneyText(char* buffer, int32 bufferSize, int32 money);
	static void GetWantedStars(char* buffer, int32 bufferSize, int32 wantedLevel);

public:
	static void Initialize(void);
	static void Shutdown(void);
	static void Update(void);

	static void Enable(void);
	static void Disable(void);
	static bool IsEnabled(void) { return ms_bEnabled; }
};