#include "lua/system.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "Utils.h"
#include "common/Files.h"
#include "network/Network_Functions.h"
#include "EventSystem.h"
#include "plugins/LuaPlugin.h"

const struct luaL_Reg LuaSystemLib::lib[] = {
        {"msgAll", &LuaMessageToAll},
        {"msg", &LuaMessage},
        {"getfile", &LuaFileGet},
        {"getfolder", &LuaFolderGet},
        {"addEvent", &LuaEventAdd},
        {"deleteEvent", &LuaEventDelete},
        {NULL, NULL}
};

int LuaSystemLib::openLib(lua_State* L)
{
    lua_getglobal(L, "System");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaSystemLib::lib, 0);
    lua_setglobal(L, "System");
    return 1;
}

int LuaSystemLib::LuaFileGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_File_Get called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string fileName(lua_tostring(L, 1));
    Files* f = Files::GetInstance();
    lua_pushstring(L, f->GetFile(fileName).c_str());
    return 1;
}


int LuaSystemLib::LuaFolderGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_Folder_Get called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string fileName(lua_tostring(L, 1));
    Files* f = Files::GetInstance();
    lua_pushstring(L, f->GetFolder(fileName).c_str());
    return 1;
}



int LuaSystemLib::LuaEventAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 6) {
        Logger::LogAdd("Lua", "LuaError: Event_Add called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string eventId(lua_tostring(L, 1));
    std::string function(lua_tostring(L, 2));
    std::string type(lua_tostring(L, 3));
    int setOrCheck = luaL_checkinteger(L, 4);
    int timed = luaL_checkinteger(L, 5);
    int mapId = luaL_checkinteger(L, 6);

    if (!Dispatcher::hasdescriptor(type)) {
        Logger::LogAdd("Lua", "LuaError: Invalid event type: " + type + ".", LogType::WARNING,GLF);
        return 0;
    }

    auto typeAsEvent = Dispatcher::getDescriptor(type);

    LuaEvent newEvent{
            function,
            typeAsEvent,
            clock(),
            timed
    };
    LuaPlugin* lp = LuaPlugin::GetInstance();

    if (setOrCheck == 1) {
        if (lp->events.find(typeAsEvent) != lp->events.end()) {
            lp->events[typeAsEvent].push_back(newEvent);
        }
        else {
            lp->events.insert(std::make_pair(typeAsEvent, std::vector<LuaEvent>()));
            lp->events[typeAsEvent].push_back(newEvent);
        }
    }
    else {
        bool eventExists = false;

        if (lp->events.find(typeAsEvent) != lp->events.end()) {
            for (const auto& i : lp->events[typeAsEvent]) {
                if (i.type == typeAsEvent && i.functionName == function) {
                    eventExists = true;
                    break;
                }
            }
        }

        lua_pushboolean(L, eventExists);
        return 1;
    }

    return 0;
}

int LuaSystemLib::LuaEventDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Event_Delete called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string eventId(lua_tostring(L, 1));
    // -- TODO:
    return 0;
}


int LuaSystemLib::LuaMessageToAll(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send_2_All called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = luaL_checkinteger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend2All(mapId, message, messageType);
    return 0;
}

int LuaSystemLib::LuaMessage(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = luaL_checkinteger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend(clientId, message, messageType);
    return 0;
}