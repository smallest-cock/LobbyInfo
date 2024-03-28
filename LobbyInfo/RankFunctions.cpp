#include "pch.h"
#include "LobbyInfo.h"


const std::unordered_map<int, std::string> playlists = {
	{1, "casual"},
	{10, "1v1"},
	{11, "2v2"},
	{13, "3v3"}
};

const std::unordered_map<int, std::string> RankNames = {
	{1 , "B1"},
	{2, "B2"},
	{3, "B3"},
	{4, "S1"},
	{5, "S2"},
	{6, "S3"},
	{7, "G1"},
	{8, "G2"},
	{9, "G3"},
	{10, "P1"},
	{11, "P2"},
	{12, "P3"},
	{13, "D1"},
	{14, "D2"},
	{15, "D3"},
	{16, "C1"},
	{17, "C2"},
	{18, "C3"},
	{19, "GC1"},
	{20, "GC2"},
	{21, "GC3"},
	{22, "SSL"}
};

enum class PlaylistEnum
{
	Unranked = 1,
	Solo = 10,
	Twos = 11,
	Threes = 13
};
const static auto PlaylistValues = {
	PlaylistEnum::Unranked,
	PlaylistEnum::Solo,
	PlaylistEnum::Twos,
	PlaylistEnum::Threes
};

enum class Rank
{
	SupersonicLegend = 22,
	GrandChamp3 = 21,
	GrandChamp2 = 20,
	GrandChamp1 = 19,
	Champ3 = 18,
	Champ2 = 17,
	Champ1 = 16,
	Diamond3 = 15,
	Diamond2 = 14,
	Diamond1 = 13,
	Platinum3 = 12,
	Platinum2 = 11,
	Platinum1 = 10,
	Gold3 = 9,
	Gold2 = 8,
	Gold1 = 7,
	Silver3 = 6,
	Silver2 = 5,
	Silver1 = 4,
	Bronze3 = 3,
	Bronze2 = 2,
	Bronze1 = 1,
	Unranked = 0,
};


void LobbyInfo::updatePlayers() {
	if (!(gameWrapper->IsInOnlineGame())) { return; }
	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (!server) { return; }

	// delete and yeet existing player rank data (replace everything with new/updated data from pris)
	Players.clear();

	// get/set new data
	ArrayWrapper<PriWrapper> priList = server.GetPRIs();
	MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();

	for (PriWrapper playerPri : priList) {
		if (!playerPri) { return; }
		if (playerPri.GetbBot()) { return; }

		Player player;
		player.name = playerPri.GetPlayerName().ToString();
		player.casMatchesPlayed = 0;
		player.casMMR = 0;
		UniqueIDWrapper playerUID = playerPri.GetUniqueIdWrapper();

		for (const PlaylistEnum& p : PlaylistValues) {

			// retrieve playlist name based on playlist int
			auto it = playlists.find(int(p));
			if (it == playlists.end()) {
				LOG("playlist name wasnt found in map based on given integer: {}", std::to_string(int(p)));
				break;
			}
			std::string playlistName = it->second;

			if (playlistName != "casual") {
				SkillRank rank = mmrWrapper.GetPlayerRank(playerUID, int(p));

				// if rank Tier attribute isn't valid (e.g. player hasn't played the playlist in a while), skip
				if (!rank.Tier) {
					LOG("****** {} rank tier for {} == 0!! (maybe {} hasn't touched the gamemode in a while)", player.name, playlistName, player.name);
				}

				player.ranks.insert({ playlistName, rank });
			}
			else {
				float mmr = mmrWrapper.GetPlayerMMR(playerUID, int(p));
				player.casMMR = int(round(mmr));	// round float mmr
				SkillRank casSkillRank = mmrWrapper.GetPlayerRank(playerUID, int(p));
				if (!(casSkillRank.MatchesPlayed || casSkillRank.MatchesPlayed == 0)) {
					LOG("casSkillRank.MatchesPlayed is null!");
					continue;
				}
				player.casMatchesPlayed = casSkillRank.MatchesPlayed;
			}
		}
		Players.push_back(player);
	}

	// write to json file
	updateJson(Players);
}

void LobbyInfo::updateJson(std::vector<Player> playersVect) {
	bool minifyJSON = cvarManager->getCvar("LobbyInfo_minifyRankLog").getBoolValue();

	// create new json object and populate it with data from chat
	nlohmann::json lobbyRanksObj;
	lobbyRanksObj["user"] = userName;

	for (Player p : playersVect) {

		// skip player if their ranks map wasn't populated in updatePlayers()
		if (p.ranks.size() == 0) { continue; }

		for (auto playlist : p.ranks) {
			std::string playlistName = playlist.first;
			SkillRank playlistRankInfo = playlist.second;

			LOG("logging {} info for {}", playlistName, p.name);

			std::string rankTier = "n/a";
			if (playlistRankInfo.Tier != 0) {

				// check if tier int is valid for RankNames map
				auto itForTier = RankNames.find(playlistRankInfo.Tier);
				if (itForTier == RankNames.end()) {
					LOG("****** rank name wasn't found in RankNames map based on given int: {}", std::to_string(playlistRankInfo.Tier));
					continue;
				}
				rankTier = itForTier->second;
			}
			std::string rankDiv = rankTier != "n/a" ? std::to_string(playlistRankInfo.Division + 1) : "n/a";

			// create json data
			lobbyRanksObj["lobbyRanks"][p.name][playlistName]["rank"]["tier"] = rankTier;
			lobbyRanksObj["lobbyRanks"][p.name][playlistName]["rank"]["div"] = rankDiv;
			lobbyRanksObj["lobbyRanks"][p.name][playlistName]["matches"] = playlistRankInfo.MatchesPlayed;
		}
		// handle casual here
		lobbyRanksObj["lobbyRanks"][p.name]["casual"]["mmr"] = p.casMMR;
		lobbyRanksObj["lobbyRanks"][p.name]["casual"]["matches"] = p.casMatchesPlayed;
	}

	try {
		writeContent(ranksFilePath, lobbyRanksObj.dump(minifyJSON ? -1 : 4));
		LOG("Updated Ranks.json :)");
	}
	catch (...) {
		LOG("*** Couldn't read the 'Ranks.json' file! Make sure it contains valid JSON... ***");
	}
}

void LobbyInfo::updateJsonOnEvent(const char* eventFunction) {
	gameWrapper->HookEvent(eventFunction, [this](std::string eventName) {
		updatePlayers();
		});
}