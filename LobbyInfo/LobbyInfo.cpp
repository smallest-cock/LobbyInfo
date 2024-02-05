#include "pch.h"
#include "LobbyInfo.h"



BAKKESMOD_PLUGIN(LobbyInfo, "Lobby Info", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;


void LobbyInfo::onLoad()
{
	_globalCvarManager = cvarManager;

    retrieveUserData();
    checkJsonFiles();

    // register console variables and set default options
    cvarManager->registerCvar("LobbyInfo_quickChatsEnabled", "1", "Toggle quick chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_userChatsEnabled", "0", "Toggle user chat logging", true, true, 0, true, 1);
    //cvarManager->registerCvar("LobbyInfo_partyChatsEnabled", "1", "Toggle party chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_teammateChatsEnabled", "1", "Toggle teammate chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_opponentChatsEnabled", "1", "Toggle opponent logging", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_spectatorChatsEnabled", "1", "Toggle spectator chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_spectatedPlayerChatsEnabled", "1", "Toggle spectator chat logging", true, true, 0, true, 1);

    cvarManager->registerCvar("LobbyInfo_saveChatsAndClearLog", "0", "Toggle saving Chats and clearing ChatLog.json after every match", false, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_clearLog", "1", "Toggle clearing 'Chats.json' after every match", false, true, 0, true, 1);

    cvarManager->registerCvar("LobbyInfo_minifySavedChats", "0", "Toggle whether or not to minify JSON file to save space", true, true, 0, true, 1);
    cvarManager->registerCvar("LobbyInfo_minifyChatLog", "1", "Toggle whether or not to minify JSON file to save space", true, true, 0, true, 1);

    // register console commands
    cvarManager->registerNotifier("LobbyInfo_clearChatLog", [this](std::vector<std::string> args) {
        clearChatLog(chatsFilePath, "Chats.json");
        }, "", PERMISSION_ALL);

    cvarManager->registerNotifier("LobbyInfo_clearSavedChats", [this](std::vector<std::string> args) {
        clearChatLog(savedChatsFilePath, currentMonth + "_SavedChats.json");
        }, "", PERMISSION_ALL);

    // triggered on chats that aren't party chats or lobby notifications
    gameWrapper->HookEventWithCallerPost<PlayerControllerWrapper>("Function TAGame.PlayerController_TA.ChatMessage_TA",
        [this](PlayerControllerWrapper caller, void* params, std::string eventname) {
            onChat(params);
        });

    // when user leaves match     
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", [this](std::string eventName) {
        bool saveChatsAndClearLogEnabled = cvarManager->getCvar("LobbyInfo_saveChatsAndClearLog").getBoolValue();
        bool clearLogEnabled = cvarManager->getCvar("LobbyInfo_clearLog").getBoolValue();

        if (saveChatsAndClearLogEnabled) {
            saveChatsAndClearLog();
        }
        else if (clearLogEnabled) {
            clearChatLog(chatsFilePath, "Chats.json");
        }
        });

    // hooks to update rank info
    updateJsonOnEvent("Function GameEvent_TA.Countdown.BeginState");
    updateJsonOnEvent("Function TAGame.PRI_TA.OnTeamChanged");
    updateJsonOnEvent("Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated");
    updateJsonOnEvent("Function TAGame.PRI_TA.GetBotName");
    updateJsonOnEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard");
    updateJsonOnEvent("Function GameEvent_Soccar_TA.Active.StartRound");

    LOG("Lobby Info loaded :)");
}
