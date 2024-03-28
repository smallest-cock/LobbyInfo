#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include <nlohmann.hpp>
#include <chrono>
#include <format>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

const std::vector<std::vector<std::string>> QuickChatGroups = {
	{"I got it!","Need boost!","Take the shot!","Defending...","Go for it!","Centering!","All yours.","In position.","Incoming!","Faking.","Bumping!","On your left.","On your right.","Passing!"},
	{"Nice shot!","Great pass!","Thanks!","What a save!","Nice one!","What a play!","Great clear!","Nice block!","Nice bump!","Nice demo!"},
	{"OMG!","Noooo!","Wow!","Close one!","No way!","Holy cow!","Whew.","Siiiick!","Calculated.","Savage!","Okay."},
	{"$#@%!","No problem.","Whoops...","Sorry!","My bad...","Oops!","My fault."},
	{"gg","Well played.","That was fun!","Rematch!","One. More. Game.","What a game!","Nice moves.","Everybody dance!"},
	{"Good luck!","Have fun!", "Get ready.","This is Rocket League!","Let's do this!","Here. We. Go.","Nice cars!","I'll do my best."}
};


struct Player {
	std::string name;
	std::map<std::string, SkillRank> ranks;
	int casMMR;
	int casMatchesPlayed;

	// Constructor to initialize member variables
	Player() : casMMR(0), casMatchesPlayed(0) {}
};


class LobbyInfo: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	//std::shared_ptr<bool> enabled;

	void onLoad() override;

	// Chat stuff 
	void onChat(void* params);
	void logChat(PriWrapper pri, std::string messageContent, std::string quickChat);
	void checkJsonFiles();
	void createJsonDataAndWriteToFile(std::string chat, std::string name, std::string relation, std::string time, std::string platform);
	void clearChatLog(std::filesystem::path filePath, std::string fileName);
	void retrieveUserData();
	void writeContent(std::filesystem::path FileName, std::string Buffer);
	std::string readContent(std::filesystem::path FileName);
	std::string parsePlatform(OnlinePlatform platform);
	std::string getCurrentTimeAndDate(std::string format = "full");
	
	// Rank stuff
	std::vector<Player> Players;
	void updatePlayers();
	void updateJson(std::vector<Player> playersVect);
	void updateJsonOnEvent(const char* eventFunction);

	// user info stuff
	static std::string userName;
	static std::string userPlatform;
	static std::string userUID;
	static std::string currentMonth;

	// file paths
	static std::filesystem::path lobbyInfoFolder;
	static std::filesystem::path ranksFilePath;
	static std::filesystem::path chatsFilePath;


	//void onUnload() override; // Uncomment and implement if you need a unload method

public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
