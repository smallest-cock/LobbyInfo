#include "pch.h"
#include "LobbyInfo.h"

void LobbyInfo::RenderSettings() {
    // chat logging options CVars
    CVarWrapper quickChatsCvar = cvarManager->getCvar("LobbyInfo_quickChatsEnabled");
    CVarWrapper userChatsCvar = cvarManager->getCvar("LobbyInfo_userChatsEnabled");
    CVarWrapper teammateChatsCvar = cvarManager->getCvar("LobbyInfo_teammateChatsEnabled");
    CVarWrapper opponentChatsCvar = cvarManager->getCvar("LobbyInfo_opponentChatsEnabled");
    CVarWrapper spectatorChatsCvar = cvarManager->getCvar("LobbyInfo_spectatorChatsEnabled");
    CVarWrapper spectatedPlayerChatsCvar = cvarManager->getCvar("LobbyInfo_spectatedPlayerChatsEnabled");

    CVarWrapper minifyChatsJSONCvar = cvarManager->getCvar("LobbyInfo_minifyChatLog");
    CVarWrapper minifyRanksJSONCvar = cvarManager->getCvar("LobbyInfo_minifyRankLog");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Checkboxes for logging options
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose the type of chats to access");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (!quickChatsCvar) { return; }
    bool quickChatsEnabled = quickChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Quick chats", &quickChatsEnabled)) {
        quickChatsCvar.setValue(quickChatsEnabled);
    }

    ImGui::Spacing();

    if (!userChatsCvar) { return; }
    bool userChatsEnabled = userChatsCvar.getBoolValue();
    if (ImGui::Checkbox("User chats (you)", &userChatsEnabled)) {
        userChatsCvar.setValue(userChatsEnabled);
    }
    
    ImGui::Spacing();

    if (!teammateChatsCvar) { return; }
    bool teammateChatsEnabled = teammateChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Teammate chats", &teammateChatsEnabled)) {
        teammateChatsCvar.setValue(teammateChatsEnabled);
    }
    
    ImGui::Spacing();

    if (!opponentChatsCvar) { return; }
    bool opponentChatsEnabled = opponentChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Opponent chats", &opponentChatsEnabled)) {
        opponentChatsCvar.setValue(opponentChatsEnabled);
    }
    
    ImGui::Spacing();

    if (!spectatorChatsCvar) { return; }
    bool spectatorChatsEnabled = spectatorChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectator chats", &spectatorChatsEnabled)) {
        spectatorChatsCvar.setValue(spectatorChatsEnabled);
    }

    ImGui::Spacing();

    if (!spectatedPlayerChatsCvar) { return; }
    bool spectatedPlayerChatsEnabled = spectatedPlayerChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectated player chats", &spectatedPlayerChatsEnabled)) {
        spectatedPlayerChatsCvar.setValue(spectatedPlayerChatsEnabled);
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Minify JSON files");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (!minifyChatsJSONCvar) { return; }
    bool minifyChatsJSONEnabled = minifyChatsJSONCvar.getBoolValue();
    if (ImGui::Checkbox("Chats.json", &minifyChatsJSONEnabled)) {
        minifyChatsJSONCvar.setValue(minifyChatsJSONEnabled);
    }

    ImGui::Spacing();

    if (!minifyRanksJSONCvar) { return; }
    bool minifyRanksJSONEnabled = minifyRanksJSONCvar.getBoolValue();
    if (ImGui::Checkbox("Ranks.json", &minifyRanksJSONEnabled)) {
        minifyRanksJSONCvar.setValue(minifyRanksJSONEnabled);
    }
}