#include "pch.h"
#include "LobbyInfo.h"

void LobbyInfo::RenderSettings() {
    // get CVars for logging options
    CVarWrapper quickChatsCvar = cvarManager->getCvar("LobbyInfo_quickChatsEnabled");
    CVarWrapper userChatsCvar = cvarManager->getCvar("LobbyInfo_userChatsEnabled");
    CVarWrapper teammateChatsCvar = cvarManager->getCvar("LobbyInfo_teammateChatsEnabled");
    CVarWrapper opponentChatsCvar = cvarManager->getCvar("LobbyInfo_opponentChatsEnabled");
    CVarWrapper spectatorChatsCvar = cvarManager->getCvar("LobbyInfo_spectatorChatsEnabled");
    CVarWrapper spectatedPlayerChatsCvar = cvarManager->getCvar("LobbyInfo_spectatedPlayerChatsEnabled");
    //CVarWrapper partyChatsCvar = cvarManager->getCvar("LobbyInfo_partyChatsEnabled");

    // get CVars for handling 'ChatLog.json'
    CVarWrapper saveChatsAndClearLogCvar = cvarManager->getCvar("LobbyInfo_saveChatsAndClearLog");
    CVarWrapper clearLogCvar = cvarManager->getCvar("LobbyInfo_clearLog");

    // get CVars for minify options
    CVarWrapper minifySavedChatsCvar = cvarManager->getCvar("LobbyInfo_minifySavedChats");
    CVarWrapper minifyChatLogCvar = cvarManager->getCvar("LobbyInfo_minifyChatLog");

    std::string savedChatsFileName = currentMonth + "_SavedChats.json";

    // Checkboxes for logging options
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose which chats will be logged");
    ImGui::Spacing();

    if (!quickChatsCvar) { return; }
    bool quickChatsEnabled = quickChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Quick chats", &quickChatsEnabled)) {
        quickChatsCvar.setValue(quickChatsEnabled);
    }
    if (!userChatsCvar) { return; }
    bool userChatsEnabled = userChatsCvar.getBoolValue();
    if (ImGui::Checkbox("User chats (you)", &userChatsEnabled)) {
        userChatsCvar.setValue(userChatsEnabled);
    }
    /*if (!partyChatsCvar) { return; }
    bool partyChatsEnabled = partyChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Party chats", &partyChatsEnabled)) {
        partyChatsCvar.setValue(partyChatsEnabled);
    }*/
    if (!teammateChatsCvar) { return; }
    bool teammateChatsEnabled = teammateChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Teammate chats", &teammateChatsEnabled)) {
        teammateChatsCvar.setValue(teammateChatsEnabled);
    }
    if (!opponentChatsCvar) { return; }
    bool opponentChatsEnabled = opponentChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Opponent chats", &opponentChatsEnabled)) {
        opponentChatsCvar.setValue(opponentChatsEnabled);
    }
    if (!spectatorChatsCvar) { return; }
    bool spectatorChatsEnabled = spectatorChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectator chats", &spectatorChatsEnabled)) {
        spectatorChatsCvar.setValue(spectatorChatsEnabled);
    }
    if (!spectatedPlayerChatsCvar) { return; }
    bool spectatedPlayerChatsEnabled = spectatedPlayerChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectated player chats", &spectatedPlayerChatsEnabled)) {
        spectatedPlayerChatsCvar.setValue(spectatedPlayerChatsEnabled);
    }

    // Radio buttons for handling chats
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose what happens to match chats after each game\t(toggle saving chats)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Useful toggle if you happen to change your mind about saving chats mid-game");
    }
    ImGui::Spacing();

    if (!saveChatsAndClearLogCvar) { return; }
    if (!clearLogCvar) { return; }

    // to save the radio button toggle state between RL restarts
    static int selection = saveChatsAndClearLogCvar.getBoolValue() ? 0 : 1;

    if (ImGui::RadioButton("Save chats", &selection, 0)) {
        saveChatsAndClearLogCvar.setValue(true);
        clearLogCvar.setValue(false);
        LOG("Chats will be saved to '{}' and 'Chats.json' will be cleared after each match", savedChatsFileName);
    }
    if (ImGui::RadioButton("Clear chats", &selection, 1)) {
        saveChatsAndClearLogCvar.setValue(false);
        clearLogCvar.setValue(true);
        LOG("Chats in 'Chats.json' will be cleared after each match");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Chats won't be saved!");
    }

    // Checkboxes for minifying JSON files
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose which JSON files to minify\t(saves space at the cost of readability)");
    ImGui::Spacing();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Good option if JSON files will be processed programmatically");
    }

    if (!minifyChatLogCvar) { return; }
    bool minifyChatLogEnabled = minifyChatLogCvar.getBoolValue();
    if (ImGui::Checkbox("Chats.json", &minifyChatLogEnabled)) {
        minifyChatLogCvar.setValue(minifyChatLogEnabled);
    }
    if (!minifySavedChatsCvar) { return; }
    bool minifySavedChatsEnabled = minifySavedChatsCvar.getBoolValue();
    if (ImGui::Checkbox(savedChatsFileName.c_str(), &minifySavedChatsEnabled)) {
        minifySavedChatsCvar.setValue(minifySavedChatsEnabled);
    }

    // Buttons to clear JSON files
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clear all chats in JSON file");
    ImGui::Spacing();

    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
    if (ImGui::Button("Clear Chats.json")) {
        cvarManager->executeCommand("LobbyInfo_clearChatLog");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Erase all chats in 'Chats.json'\t** PERMANENT **");
    }
    ImGui::NewLine();
    std::string clearSavedChatsButton = "Clear " + savedChatsFileName;
    if (ImGui::Button(clearSavedChatsButton.c_str(), ImVec2(0, 0))) {
        cvarManager->executeCommand("LobbyInfo_clearSavedChats");
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    std::string clearSavedChatsToolTipStr = "Erase all chats in '" + savedChatsFileName + "'\t** PERMANENT **";
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(clearSavedChatsToolTipStr.c_str());
    }
}