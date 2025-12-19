#include "common.h"
#include "DiscordRPC.h"
#include "../../vendor/discord-rpc/header/discord_rpc.h"
#include "World.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Wanted.h"
#include "Timer.h"
#include "Vehicle.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#define DISCORD_APP_ID "1451455359335141526"
#define DISCORD_UPDATE_INTERVAL 2000
#define LARGE_IMAGE_KEY "rpc-image"
#define SMALL_IMAGE_KEY "money-icon"

bool CDiscordRPC::ms_bInitialized = false;
bool CDiscordRPC::ms_bEnabled = true;
int64_t CDiscordRPC::ms_nStartTimestamp = 0;
uint32 CDiscordRPC::ms_nLastUpdateTime = 0;
int32 CDiscordRPC::ms_nLastHealth = -1;
int32 CDiscordRPC::ms_nLastMoney = -1;
int32 CDiscordRPC::ms_nLastWantedLevel = -1;

char CDiscordRPC::ms_aStateBuffer[128];
char CDiscordRPC::ms_aDetailsBuffer[128];
char CDiscordRPC::ms_aLargeImageTextBuffer[128];
char CDiscordRPC::ms_aSmallImageTextBuffer[128];

void
CDiscordRPC::HandleReady(const void* request)
{
	printf("Discord RPC: Connected\n");
}

void
CDiscordRPC::HandleDisconnected(int errorCode, const char* message)
{
	printf("Discord RPC: Disconnected (%d: %s)\n", errorCode, message);
}

void
CDiscordRPC::HandleErrored(int errorCode, const char* message)
{
	printf("Discord RPC: Error (%d: %s)\n", errorCode, message);
}

void
CDiscordRPC::Initialize(void)
{
	if(ms_bInitialized)
		return;

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = (void(*)(const DiscordUser*))HandleReady;
	handlers.disconnected = HandleDisconnected;
	handlers.errored = HandleErrored;

	Discord_Initialize(DISCORD_APP_ID, &handlers, 1, NULL);

	ms_nStartTimestamp = (int64_t)time(NULL);
	ms_nLastUpdateTime = 0;
	ms_nLastHealth = -1;
	ms_nLastMoney = -1;
	ms_nLastWantedLevel = -1;

	memset(ms_aStateBuffer, 0, sizeof(ms_aStateBuffer));
	memset(ms_aDetailsBuffer, 0, sizeof(ms_aDetailsBuffer));
	memset(ms_aLargeImageTextBuffer, 0, sizeof(ms_aLargeImageTextBuffer));
	memset(ms_aSmallImageTextBuffer, 0, sizeof(ms_aSmallImageTextBuffer));

	ms_bInitialized = true;

	printf("Discord RPC: Initialized\n");
	
	DiscordRichPresence initialPresence;
	memset(&initialPresence, 0, sizeof(initialPresence));
	initialPresence.state = "Starting game...";
	initialPresence.details = "Loading Liberty City";
	initialPresence.startTimestamp = ms_nStartTimestamp;
	initialPresence.largeImageKey = LARGE_IMAGE_KEY;
	initialPresence.largeImageText = "RE III";
	Discord_UpdatePresence(&initialPresence);
	
	Discord_RunCallbacks();
}

void
CDiscordRPC::Shutdown(void)
{
	if(!ms_bInitialized)
		return;

	Discord_ClearPresence();
	Discord_Shutdown();

	ms_bInitialized = false;

	printf("Discord RPC: Shutdown\n");
}

void
CDiscordRPC::Update(void)
{
	if(!ms_bInitialized || !ms_bEnabled)
		return;

	//always run callbacks to maintain connection
	Discord_RunCallbacks();

	uint32 currentTime = CTimer::GetTimeInMilliseconds();
	
	//force first update immediately after init
	if(ms_nLastUpdateTime == 0) {
		ms_nLastUpdateTime = currentTime;
		UpdatePresenceData();
		return;
	}
	
	if(currentTime - ms_nLastUpdateTime < DISCORD_UPDATE_INTERVAL)
		return;

	ms_nLastUpdateTime = currentTime;
	UpdatePresenceData();
}

void
CDiscordRPC::UpdatePresenceData(void)
{
	CPlayerPed* player = FindPlayerPed();
	
	// Build presence struct
	DiscordRichPresence presence;
	memset(&presence, 0, sizeof(presence));
	
	if(!player) {
		// Menu state
		presence.state = "In Menu";
		presence.details = "Starting game...";
		presence.startTimestamp = ms_nStartTimestamp;
		presence.largeImageKey = LARGE_IMAGE_KEY;
		presence.largeImageText = "GTA III";
		Discord_UpdatePresence(&presence);
		return;
	}

	CPlayerInfo* playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	int32 health = (int32)player->m_fHealth;
	int32 money = playerInfo->m_nMoney;
	int32 wantedLevel = playerInfo->m_pPed->m_pWanted->GetWantedLevel();

	//update cache
	ms_nLastHealth = health;
	ms_nLastMoney = money;
	ms_nLastWantedLevel = wantedLevel;

	//build state and details
	GetPlayerState(ms_aStateBuffer, sizeof(ms_aStateBuffer));
	GetPlayerDetails(ms_aDetailsBuffer, sizeof(ms_aDetailsBuffer));
	GetHealthText(ms_aLargeImageTextBuffer, sizeof(ms_aLargeImageTextBuffer), health);
	GetMoneyText(ms_aSmallImageTextBuffer, sizeof(ms_aSmallImageTextBuffer), money);

	//set all required fields
	presence.state = ms_aStateBuffer;
	presence.details = ms_aDetailsBuffer;
	presence.startTimestamp = ms_nStartTimestamp;
	presence.largeImageKey = LARGE_IMAGE_KEY;
	presence.largeImageText = ms_aLargeImageTextBuffer;
	
	//only set small image if we have money to show
	if(money > 0) {
		presence.smallImageKey = SMALL_IMAGE_KEY;
		presence.smallImageText = ms_aSmallImageTextBuffer;
	}
	
	presence.instance = 1;

	Discord_UpdatePresence(&presence);
	
	//debug output
	printf("Discord RPC: Presence updated - %s | %s\n", ms_aStateBuffer, ms_aDetailsBuffer);
}

void
CDiscordRPC::GetPlayerState(char* buffer, int32 bufferSize)
{
	CPlayerPed* player = FindPlayerPed();
	if(!player) {
		snprintf(buffer, bufferSize, "In Menu");
		return;
	}

	CPlayerInfo* playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	int32 wantedLevel = playerInfo->m_pPed->m_pWanted->GetWantedLevel();

	if(wantedLevel > 0) {
		char stars[16];
		GetWantedStars(stars, sizeof(stars), wantedLevel);
		snprintf(buffer, bufferSize, "Wanted: %s", stars);
	} else {
		CVehicle* vehicle = FindPlayerVehicle();
		if(vehicle) {
			snprintf(buffer, bufferSize, "Driving in Liberty City");
		} else {
			snprintf(buffer, bufferSize, "Exploring Liberty City");
		}
	}
}

void
CDiscordRPC::GetPlayerDetails(char* buffer, int32 bufferSize)
{
	CPlayerPed* player = FindPlayerPed();
	if(!player) {
		snprintf(buffer, bufferSize, "Starting game...");
		return;
	}

	int32 health = (int32)player->m_fHealth;

	if(health <= 0) {
		snprintf(buffer, bufferSize, "Wasted");
	} else if(health < 50) {
		snprintf(buffer, bufferSize, "Health: Critical");
	} else {
		snprintf(buffer, bufferSize, "Health: %d", health);
	}
}

void
CDiscordRPC::GetHealthText(char* buffer, int32 bufferSize, int32 health)
{
	if(health <= 0) {
		snprintf(buffer, bufferSize, "Wasted");
	} else if(health >= 100) {
		snprintf(buffer, bufferSize, "Full Health (%d)", health);
	} else if(health >= 75) {
		snprintf(buffer, bufferSize, "Healthy (%d)", health);
	} else if(health >= 50) {
		snprintf(buffer, bufferSize, "Injured (%d)", health);
	} else if(health >= 25) {
		snprintf(buffer, bufferSize, "Critical (%d)", health);
	} else {
		snprintf(buffer, bufferSize, "Near Death (%d)", health);
	}
}

void
CDiscordRPC::GetMoneyText(char* buffer, int32 bufferSize, int32 money)
{
	if(money >= 1000000) {
		snprintf(buffer, bufferSize, "$%d.%dM", money / 1000000, (money % 1000000) / 100000);
	} else if(money >= 1000) {
		snprintf(buffer, bufferSize, "$%d,%03d", money / 1000, money % 1000);
	} else {
		snprintf(buffer, bufferSize, "$%d", money);
	}
}

void
CDiscordRPC::GetWantedStars(char* buffer, int32 bufferSize, int32 wantedLevel)
{
	if(wantedLevel <= 0) {
		buffer[0] = '\0';
		return;
	}

	int32 stars = wantedLevel > 6 ? 6 : wantedLevel;
	int32 i;

	for(i = 0; i < stars && i < bufferSize - 1; i++) {
		buffer[i] = '*';
	}
	buffer[i] = '\0';
}

void
CDiscordRPC::Enable(void)
{
	ms_bEnabled = true;
	if(ms_bInitialized) {
		UpdatePresenceData();
	}
}

void
CDiscordRPC::Disable(void)
{
	ms_bEnabled = false;
	if(ms_bInitialized) {
		Discord_ClearPresence();
	}
}