#include "pch.h"
#include "LobbyInfo.h"


struct ChatMessageParams
{
    void* InPRI;
    uint8_t Message[0x10];
    uint8_t ChatChannel;
    bool bPreset;
};


std::string LobbyInfo::userName = "?";
std::string LobbyInfo::userUID = "?";
std::string LobbyInfo::userPlatform = "?";
std::string LobbyInfo::currentMonth = "?";
std::filesystem::path LobbyInfo::lobbyInfoFolder;
std::filesystem::path LobbyInfo::ranksFilePath;
std::filesystem::path LobbyInfo::chatsFilePath;


void LobbyInfo::onChat(void* params)
{
    if (!params) return;
    ChatMessageParams* chatParams = reinterpret_cast<ChatMessageParams*>(params);
    PriWrapper pri(reinterpret_cast<uintptr_t>(chatParams->InPRI));
    std::string message = (UnrealStringWrapper(reinterpret_cast<uintptr_t>(&chatParams->Message))).ToString();

    if (!pri) {
        LOG("pri is NULL! ... returning");
        return;
    }

    // parse quickchat if necessary
    std::string quickChat = "";
    if (message.size() <= 15 && message.contains("Group") && message.contains("Message")) {
        std::vector<int> messageDigits = {};
        for (char ch : message) {
            if (isdigit(ch)) {
                int digitInt = ch - '0';
                messageDigits.push_back(digitInt);
            }
        }
        if (messageDigits.size() == 2) {
            quickChat = QuickChatGroups[messageDigits[0] - 1][messageDigits[1] - 1];
        }
        else if (messageDigits.size() == 3) {
            int secondIndex = (messageDigits[1] * 10) + messageDigits[2];
            quickChat = QuickChatGroups[messageDigits[0] - 1][secondIndex - 1];
        }
    }

    logChat(pri, message, quickChat);
}


void LobbyInfo::logChat(PriWrapper pri, std::string chat, std::string quickChat) {
    // handle disabled quick chats
    bool isQuickChat = !(quickChat == "");
    bool quickChatsEnabled = cvarManager->getCvar("LobbyInfo_quickChatsEnabled").getBoolValue();
    if (isQuickChat && !quickChatsEnabled) {
        LOG("Didn't update chat log... (quick chat logging is disabled)");
        return;
    }

    bool isUser = false;
    bool isTeammate = false;
    bool isOpponent = false;
    bool isSpectator = pri.IsSpectator();
    bool isBeingSpectated = false;

    bool userChatsEnabled = cvarManager->getCvar("LobbyInfo_userChatsEnabled").getBoolValue();
    bool teammateChatsEnabled = cvarManager->getCvar("LobbyInfo_teammateChatsEnabled").getBoolValue();
    bool opponentChatsEnabled = cvarManager->getCvar("LobbyInfo_opponentChatsEnabled").getBoolValue();
    bool spectatorChatsEnabled = cvarManager->getCvar("LobbyInfo_spectatorChatsEnabled").getBoolValue();
    bool spectatedPlayerChatsEnabled = cvarManager->getCvar("LobbyInfo_spectatedPlayerChatsEnabled").getBoolValue();

    std::string dateAndTime = getCurrentTimeAndDate();
    std::string chatterUID = pri.GetUniqueIdWrapper().GetIdString();
    std::string relation;

    // handle disabled chats for user
    if (chatterUID == userUID) {
        isUser = true;
        relation = "me";
    }
    if (isUser && !userChatsEnabled) {
        LOG("Didn't update chat log... (user chat logging is disabled)");
        return;
    }

    if (isSpectator && !isUser) {
        relation = "spectator";
    }

    if (!isUser && !isSpectator) {
        // find chatter's team
        unsigned char chatterTeam = pri.GetTeamNum2();

        // find user's team
        PlayerControllerWrapper userPlayerController = gameWrapper->GetPlayerController();
        if (!userPlayerController) { return; }
        PriWrapper userPri = userPlayerController.GetPRI();
        if (!userPri) { return; }
        unsigned char usersTeam = userPri.GetTeamNum2();     // user's team is 255 if theyre spectating (unsigned char converts -1 to 255)

        // determine chatter's relation to user
        if (usersTeam != 255) {
            if (chatterTeam == usersTeam) {
                isTeammate = true;
                relation = "teammate";
            }
            else {
                isOpponent = true;
                relation = "opponent";
            }
        }
        else {
            isBeingSpectated = true;
            relation = "spectated by me";
        }
    }

    // handle disabled chats for spectator/teammate/opponent
    if (isSpectator && !spectatorChatsEnabled && !isUser) {
        LOG("Didn't update chat log... (spectator chat logging is disabled)");
        return;
    }
    if (isTeammate && !teammateChatsEnabled) {
        LOG("Didn't update chat log... (teammate chat logging is disabled)");
        return;
    }
    if (isOpponent && !opponentChatsEnabled) {
        LOG("Didn't update chat log... (opponent chat logging is disabled)");
        return;
    }
    if (isBeingSpectated && !spectatedPlayerChatsEnabled) {
        LOG("Didn't update chat log... (spectated player chat logging is disabled)");
        return;
    }

    // get additional info to be logged
    std::string chatterName = pri.GetPlayerName().ToString();
    chat = isQuickChat ? quickChat : chat;  // makes sure quickchat is parsed before logging

    OnlinePlatform platform = pri.GetPlatform();
    if (!platform) { return; }
    std::string platformStr = parsePlatform(platform);

    // console log the chat which will be saved
    LOG("\t\t{}", dateAndTime);
    LOG("Player:\t\t{}\t({})", chatterName, relation);
    LOG("Chat:\t\t\"{}\"", chat);

    // log chat in JSON file
    createJsonDataAndWriteToFile(chat, chatterName, relation, dateAndTime, platformStr);
}


std::string LobbyInfo::parsePlatform(OnlinePlatform platform) {
    switch (platform)
    {
    case OnlinePlatform_Steam:
        return "Steam";
    case OnlinePlatform_PS4:
    case OnlinePlatform_PS3:
        return "PlayStation";
    case OnlinePlatform_Dingo:
        return "Xbox";
    case OnlinePlatform_OldNNX:
    case OnlinePlatform_NNX:
        return "Nintendo";
    case OnlinePlatform_PsyNet:
    case OnlinePlatform_Epic:
        return "Epic";
    default:
    case OnlinePlatform_QQ:
    case OnlinePlatform_Deleted:
    case OnlinePlatform_WeGame:
    case OnlinePlatform_MAX:
        return "Unknown";
    }
}


void LobbyInfo::createJsonDataAndWriteToFile(std::string chat, std::string name, std::string relation, std::string time, std::string platform) {
    bool minifyJSON = cvarManager->getCvar("LobbyInfo_minifyChatLog").getBoolValue();
    
    // create new json object and populate it with data from chat
    nlohmann::json chatObj;
    chatObj["chat"] = chat;
    chatObj["playerName"] = name;
    chatObj["platform"] = platform;
    chatObj["relation"] = relation;
    chatObj["time"] = time;

    //std::filesystem::path chatLogFilePath = bmDataFolderFilePath / "Chat Logs" / "ChatLog.json";
    std::string jsonFileRawStr = readContent(chatsFilePath);

    // prevent crash on reading invalid JSON data
    try {
        auto chatJsonData = nlohmann::json::parse(jsonFileRawStr);
        chatJsonData["chatMessages"].push_back(chatObj);

        writeContent(chatsFilePath, chatJsonData.dump(minifyJSON ? -1 : 4));
        LOG("Recorded chat :)");
    }
    catch (...) {
        LOG("*** Couldn't read the 'Chats.json' file! Make sure it contains valid JSON... ***");
    }
}


void LobbyInfo::clearChatLog(std::filesystem::path filePath, std::string fileName) {
    try {
		if (std::filesystem::exists(filePath)) {
			std::ofstream NewFile(filePath);
			NewFile << "{ \"user\": \"" + userName + "\", \"chatMessages\":[]}";
			NewFile.close();
			LOG("Cleared '{}' :)", fileName);
		}
		else {
			LOG("*** Couldn't clear '{}' ... it doesn't exist! ***", fileName);
		}
    }
    catch (...) {
        LOG("*** Something went wrong while clearing '{}' ***", fileName);
    }
}


std::string LobbyInfo::getCurrentTimeAndDate(std::string format) {
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    return format == "full" ? std::format("{:%m/%d/%Y %r}", time) : std::format("{:%B_%Y}", time);
}


std::string LobbyInfo::readContent(std::filesystem::path FileName) {
    std::ifstream Temp(FileName);
    std::stringstream Buffer;
    Buffer << Temp.rdbuf();
    return (Buffer.str());
}


void LobbyInfo::writeContent(std::filesystem::path FileName, std::string Buffer) {
    std::ofstream File(FileName, std::ofstream::trunc);
    File << Buffer;
    File.close();
}


void LobbyInfo::checkJsonFiles() {
    // create 'Lobby Info' folder if it doesn't exist
    if (!std::filesystem::exists(lobbyInfoFolder)) {
        std::filesystem::create_directory(lobbyInfoFolder);
        LOG("'Lobby Info' folder didn't exist... so I created it.");
    }

    // create JSON files if they don't exist
    if (!std::filesystem::exists(ranksFilePath)) {
        std::ofstream NewFile(ranksFilePath);
        NewFile << "{ \"user\": \"" + userName + "\", \"lobbyRanks\": {}}";
        NewFile.close();
        LOG("'Ranks.json' didn't exist... so I created it.");
    }
    if (!std::filesystem::exists(chatsFilePath)) {
        std::ofstream NewFile(chatsFilePath);
        NewFile << "{ \"user\": \"" + userName + "\", \"chatMessages\": []}";
        NewFile.close();
        LOG("'Chats.json' didn't exist... so I created it.");
    }
}


void LobbyInfo::retrieveUserData() {
    try {
        bool isEpic = gameWrapper->IsUsingEpicVersion();
        userPlatform = isEpic ? "Epic" : "Steam";
        userName = gameWrapper->GetPlayerName().ToString();
        userUID = gameWrapper->GetUniqueID().GetIdString();
        currentMonth = getCurrentTimeAndDate("month");

        // store file paths
        std::filesystem::path bmDataFolderFilePath = gameWrapper->GetDataFolder();
        lobbyInfoFolder = bmDataFolderFilePath / "Lobby Info";
        ranksFilePath = lobbyInfoFolder / "Ranks.json";
        chatsFilePath = lobbyInfoFolder / "Chats.json";

        LOG("Successfully retrieved user data");
        LOG("Player name: {}", userName);
        LOG("Player UID: {}", userUID);
        LOG("Platform: {}", userPlatform);
    }
    catch (...) {
        LOG("Couldn't retrieve user data :(");
    }
}