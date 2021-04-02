//
// Created by Wande on 3/22/2021.
//
#include "Player_List.h"
const std::string MODULE_NAME = "Player_List";
Player_List* Player_List::Instance = nullptr;

void Player_List::CloseDatabase() {
    if (dbOpen) {
        dbOpen = false;
        sqlite3_close(db);
        Logger::LogAdd(MODULE_NAME, "Database closed [" + fileName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

static int callback(void* NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}

void Player_List::CreateDatabase() {
    char *zErrMsg = 0;

    sqlite3* tempdb;
    sqlite3_open(fileName.c_str(), &tempdb);
    int rc = sqlite3_exec(tempdb, CREATE_SQL.c_str(), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        Logger::LogAdd(MODULE_NAME, "Failed to create new DB", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        // -- err :<
        sqlite3_free(tempdb);
        return;
    }

    Logger::LogAdd(MODULE_NAME, "Database created.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    sqlite3_close(tempdb);
}

void Player_List::OpenDatabase() {
    CloseDatabase();
    if (Utils::FileSize(fileName) == -1) {
        CreateDatabase();
    }
    sqlite3_open(fileName.c_str(), &db);
    dbOpen = true;
    Logger::LogAdd(MODULE_NAME, "Database Opened.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

Player_List::Player_List() {
    dbOpen = false;
    Files* f = Files::GetInstance();
    fileName = f->GetFile(PLAYERLIST_FILE_NAME);
    _numberCounter = -1;
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };
    this->Interval = chrono::minutes(2);

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void Player_List::Load() {
    OpenDatabase();
    sqlite3_stmt *stmt;
    std::string sqlStatement = "SELECT * FROM Player_List";
    char* errMsg = 0;
    int rc;
    rc = sqlite3_prepare_v2(db, sqlStatement.c_str(), sqlStatement.size(), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        Logger::LogAdd(MODULE_NAME, "Failed to load DB!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        Logger::LogAdd(MODULE_NAME, "DB Error: " + stringulate(errMsg), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PlayerListEntry newEntry;

        sqlite3_column_text(stmt, 3);
        newEntry.Number = sqlite3_column_int(stmt, 0);
        const char *tmp;
        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        newEntry.Name = std::string(tmp);

        newEntry.PRank = sqlite3_column_int(stmt, 2);
        newEntry.LoginCounter = sqlite3_column_int(stmt, 3);
        newEntry.KickCounter = sqlite3_column_int(stmt, 4);
        newEntry.OntimeCounter = sqlite3_column_int(stmt, 5);
        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (tmp != nullptr)
        newEntry.IP = std::string(tmp);
        newEntry.Stopped = sqlite3_column_int(stmt, 7);
        newEntry.Banned = sqlite3_column_int(stmt, 8);
        newEntry.MuteTime = sqlite3_column_int(stmt, 9);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        if (tmp != nullptr)
        newEntry.BanMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        if (tmp != nullptr)
        newEntry.KickMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        if (tmp != nullptr)
        newEntry.MuteMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        if (tmp != nullptr)
        newEntry.RankMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
        if (tmp != nullptr)
        newEntry.StopMessage = std::string(tmp);

        newEntry.GlobalChat = sqlite3_column_int(stmt, 16);
        _pList.push_back(newEntry);
    }

    sqlite3_finalize(stmt);
    Logger::LogAdd(MODULE_NAME, "Database Loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    CloseDatabase();
    for(auto const &i : _pList) {
        if ((_numberCounter & 65535) <= (i.Number & 65535))
            _numberCounter = i.Number + 1;
    }
}

void Player_List::Save() {
    OpenDatabase();
    for(auto const &i : _pList) {
        if (!i.Save)
            continue;
        sqlite3_stmt *res;
        int rc = sqlite3_prepare_v2(db, REPLACE_SQL.c_str(), -1, &res, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_int(res, 1, i.Number);
            sqlite3_bind_text(res, 2, i.Name.c_str(), -1, nullptr);
            sqlite3_bind_int(res, 3, i.PRank);
            sqlite3_bind_int(res, 4, i.LoginCounter);
            sqlite3_bind_int(res, 5, i.KickCounter);
            sqlite3_bind_int(res, 6, i.OntimeCounter);
            sqlite3_bind_text(res, 7, i.IP.c_str(), -1, nullptr);
            sqlite3_bind_int(res, 8, i.Stopped);
            sqlite3_bind_int(res, 9, i.Banned);
            sqlite3_bind_int(res, 10, i.MuteTime);
            sqlite3_bind_text(res, 11, i.BanMessage.c_str(), -1, nullptr);
            sqlite3_bind_text(res, 12, i.KickMessage.c_str(), -1, nullptr);
            sqlite3_bind_text(res, 13, i.MuteMessage.c_str(), -1, nullptr);
            sqlite3_bind_text(res, 14, i.RankMessage.c_str(), -1, nullptr);
            sqlite3_bind_text(res, 15, i.StopMessage.c_str(), -1, nullptr);
            sqlite3_bind_int(res, 16, i.GlobalChat);
        }
        sqlite3_step(res);
        sqlite3_finalize(res);
        CloseDatabase();
        Logger::LogAdd(MODULE_NAME, "Database Saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

PlayerListEntry* Player_List::GetPointer(int playerId) {
    for (auto & i : _pList) {
        if (i.Number == playerId) {
            return &i;
        }
    }

    return nullptr;
}

PlayerListEntry *Player_List::GetPointer(std::string player) {
    for (auto & i : _pList) {
        if (i.Name == player) {
            return &i;
        }
    }

    return nullptr;
}

void Player_List::MainFunc() {
    if (SaveFile) {
        SaveFile = false;
        Save();
    }
}

void Player_List::Add(std::string name) {
    if (GetPointer(name) != nullptr)
        return;

    PlayerListEntry newEntry;
    newEntry.Number = GetNumber();
    newEntry.Name = name;
    newEntry.PRank = 0;
    newEntry.Save = true;
    newEntry.GlobalChat = true;
    SaveFile = true;
    _pList.push_back(newEntry);
}

int Player_List::GetNumber() {
    int result = _numberCounter;

    while (result == -1 || GetPointer(result) != nullptr) {
        result = _numberCounter;
        _numberCounter++;
    }

    return result;
}

Player_List *Player_List::GetInstance() {
    if (Instance == nullptr)
        Instance = new Player_List();

    return Instance;
}

int PlayerListEntry::GetAttribute(std::string attrName) {
    int result = 0;

    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            result = NumAttributes[i];
            break;
        }
    }

    return result;
}

std::string PlayerListEntry::GetAttributeStr(std::string attrName) {
    std::string result = "";

    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            result = StrAttributes[i];
            break;
        }
    }

    return result;
}

void PlayerListEntry::SetAttribute(std::string attrName, int value) {
    bool found = false;
    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            if (value == 0)
                Attributes[i] = "";

            NumAttributes[i] = value;
            found = true;
            break;
        }
    }

    if (!found) {
        for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
            if (Attributes[i].empty()) {
                if (value != 0)
                    Attributes[i] = attrName;
                NumAttributes[i] = value;
                break;
            }
        }
    }
}

void PlayerListEntry::SetAttribute(std::string attrName, std::string value) {
    bool found = false;
    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            if (value.empty())
                Attributes[i] = "";

            StrAttributes[i] = value;
            found = true;
            break;
        }
    }

    if (!found) {
        for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
            if (Attributes[i].empty()) {
                if (!value.empty())
                    Attributes[i] = attrName;
                StrAttributes[i] = value;
                break;
            }
        }
    }
}

void PlayerListEntry::SetRank(int rank, const std::string &reason) {
    this->PRank = rank;
    RankMessage = reason;
    Save = true;
    Player_List* i = Player_List::GetInstance();
    i->SaveFile = true;

    Network* ni = Network::GetInstance();
    Rank* r = Rank::GetInstance();

    for(auto &nc : ni->_clients) {
        if (nc.second->player && nc.second->player->tEntity && nc.second->player->tEntity->playerList && nc.second->player->tEntity->playerList->Number == Number) {
            RankItem ri = r->GetRank(rank, false);
            Entity::SetDisplayName(nc.second->player->tEntity->Id, ri.Prefix, this->Name, ri.Suffix);
            NetworkFunctions::SystemMessageNetworkSend(nc.first, "Your rank has been changed to '" + ri.Name + "' (" + reason + ")");
        }
    }
}

void PlayerListEntry::Kick(std::string reason, int count, bool log, bool show) {
    bool found = false;
    Network* ni = Network::GetInstance();

    for(auto &nc : ni->_clients) {
        if (nc.second->player && nc.second->player->tEntity && nc.second->player->tEntity->playerList && nc.second->player->tEntity->playerList->Number == Number) {
            nc.second->Kick("You got kicked (" + reason + ")", !show);
            found = true;
        }
    }
    if (found) {
        KickCounter++;
        KickMessage = reason;
        Save = true;
        Player_List *i = Player_List::GetInstance();
        i->SaveFile = true;
        if (show) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "Player " + Name + " was kicked (" + reason + ")");
        }
        if (log) {
            Logger::LogAdd(MODULE_NAME, "Player " + Name + " was kicked (" + reason + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        }
    }
}

void PlayerListEntry::SetGlobal(bool globalChat) {
    GlobalChat = globalChat;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
}

void PlayerListEntry::Mute(int minutes, std::string reason) {

}

void PlayerListEntry::Unmute() {

}

void PlayerListEntry::Stop(std::string reason) {

}

void PlayerListEntry::Unstop() {

}
