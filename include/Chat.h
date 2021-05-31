//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_CHAT_H
#define D3PP_CHAT_H
#include <iostream>
#include <string>
#include <regex>

#include "Network.h"
#include "Entity.h"
#include "Network_Functions.h"

const std::regex AllowedRegexp("[^A-Za-z0-9!\\^\\~$%&/()=?{}\t\\[\\]\\\\ ,\\\";.:\\-_#'+*<>|@]|&.$|&.(&.)");
class NetworkClient;

class Chat {
public:
    static std::string StringMultiline(std::string input);
    static bool StringIV(std::string input);
    static std::string StringGV(std::string input);
    static void NetworkSend2All(int entityId, std::string message);
    static void HandleIncomingChat(const shared_ptr<NetworkClient> client, std::string input, char playerId);
};


#endif //D3PP_CHAT_H
