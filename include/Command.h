//
// Created by Wande on 6/4/2021.
//

#ifndef D3PP_COMMAND_H
#define D3PP_COMMAND_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include "TaskScheduler.h"

class Network;
class NetworkClient;

const int COMMAND_OPERATORS_MAX = 5;
const std::string COMMAND_FILENAME = "Command";

struct CommandGroup {
    std::string Name;
    short RankShow;
};

class Command {
    public:
        std::string Id;
        std::string Name;
        std::string Group;
        std::string Description;
        short Rank;
        short RankShow;
        std::string Plugin;
        std::function<void()> Function;
        bool Internal;
        bool Hidden;
};

class CommandMain : TaskItem {
    public:

        bool SaveFile;
        time_t FileDateLast;
        int CommandClientId;
        std::string ParsedCommand;
        std::string ParsedOperator[COMMAND_OPERATORS_MAX];
        std::string ParsedText0;
        std::string ParsedText1;
        std::string ParsedText2;
        std::vector<CommandGroup> CommandGroups;
        std::vector<Command> Commands;
        
        static CommandMain* GetInstance();
        static CommandMain* Instance;

        CommandMain();
        void Init();
        void Load();
        void Save();
        void MainFunc();
        void CommandDo(const std::shared_ptr<NetworkClient> client, std::string input);
        // --
        void CommandCommands();
        void CommandHelp();
        void CommandPlayers();
        void CommandPlayerInfo();
        void CommandGlobal();
        void CommandPing();
        void CommandChangeMap();
        void CommandChangeRank();
    private:
};


#endif