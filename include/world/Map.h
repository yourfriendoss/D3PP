//
// Created by unknown on 4/2/21.
//
// Time for the biggest module in the server boys. 2242 lines of PB code.
//

#ifndef D3PP_MAP_H
#define D3PP_MAP_H
#define GLF __FILE__, __LINE__, __FUNCTION__
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <memory>
#include <filesystem>

#include "world/IMapProvider.h"
#include "common/TaskScheduler.h"
#include "world/MapActions.h"
#include "common/MinecraftLocation.h"
#include "common/Vectors.h"

class IMinecraftClient;
class Entity;

namespace D3PP::world {
    class Teleporter;

    enum MapAction {
        SAVE = 0,
        LOAD,
        RESIZE = 5,
        FILL,
        DELETE = 10,
    };

    struct MapActionItem {
        int ID;
        int ClientID;
        int MapID;
        MapAction Action;
        std::string FunctionName;
        std::string Directory;
        unsigned short X;
        unsigned short Y;
        unsigned short Z;
        std::string ArgumentString;
    };

    struct MapBlockDo { // -- Physics Queue Item
        std::chrono::time_point<std::chrono::steady_clock> time;
        unsigned short X;
        unsigned short Y;
        unsigned short Z;
    };

    struct MapBlockChanged {
        unsigned short X;
        unsigned short Y;
        unsigned short Z;
        unsigned char Priority;
        short OldMaterial;

        bool operator()(const MapBlockChanged &a, const MapBlockChanged &b) {
            return a.Priority < b.Priority;
        }
    };

    struct UndoStep {
        short PlayerNumber;
        int MapId;
        short X;
        short Y;
        short Z;
        time_t Time;
        char TypeBefore;
        short PlayerNumberBefore;
    };

    const std::string MAP_LIST_FILE = "Map_List";
    const std::string MAP_SETTINGS_FILE = "Map_Settings";
    const std::string MAP_HTML_FILE = "Map_HTML";

    const int MAP_BLOCK_ELEMENT_SIZE = 4;

    class Map {
        friend class MapMain;

    public:
        int ID;
        time_t SaveTime;

        std::vector<unsigned char> PhysicData;
        std::vector<unsigned char> BlockchangeData;
        std::vector<Teleporter> Portals;

        std::mutex physicsQueueMutex;
        std::mutex bcMutex;

        std::vector<MapBlockDo> PhysicsQueue;
        std::priority_queue<MapBlockChanged, std::vector<MapBlockChanged>, MapBlockChanged> ChangeQueue;
        std::vector<UndoStep> UndoCache;

        bool BlockchangeStopped, PhysicsStopped, loading, loaded;
        std::string filePath;
        time_t LastClient;
        int Clients;

        Map();
        std::string Name() { return m_mapProvider->MapName; }
        Common::Vector3S GetSize() { return m_mapProvider->GetSize(); }
        MinecraftLocation GetSpawn() { return m_mapProvider->GetSpawn(); }
        void SetSpawn(MinecraftLocation location);

        bool Resize(short x, short y, short z);

        void Fill(const std::string &functionName, const std::string& paramString);

        void BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                       unsigned short Z1, bool undo, bool physic, unsigned char priority);

        void BlockChange(const std::shared_ptr<IMinecraftClient> &client, unsigned short X, unsigned short Y,
                         unsigned short Z, unsigned char mode, unsigned char type);

        void BlockChange(short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type,
                         bool undo, bool physic, bool send, unsigned char priority);

        void ProcessPhysics(unsigned short X, unsigned short Y, unsigned short Z);

        bool Save(const std::string& directory);

        void Load(const std::string& directory);

        unsigned char GetBlockType(unsigned short X, unsigned short Y, unsigned short Z);

        unsigned short GetBlockPlayer(unsigned short X, unsigned short Y, unsigned short Z);

        int BlockGetRank(unsigned short X, unsigned short Y, unsigned short Z);
        void SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                        unsigned short Z1, short rank);

        void Reload();
        void Unload();

        void Send(int clientId);
        void Resend();

        void
        AddTeleporter(std::string id, MinecraftLocation start, MinecraftLocation end, MinecraftLocation destination,
                      std::string destMapUniqueId, int destMapId);
        void DeleteTeleporter(std::string id);
        const Teleporter GetTeleporter(std::string id);

        void MapExport(MinecraftLocation start, MinecraftLocation end, std::string filename);

        void MapImport(std::string filename, MinecraftLocation location, short scaleX, short scaleY, short scaleZ);

        void SetEnvColors(int red, int green, int blue, int type);
        void SetMapAppearance(std::string url, int sideblock, int edgeblock, int sidelevel);
        void
        SetHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight);

        MapPermissions GetMapPermissions();
        void SetMapPermissions(const MapPermissions& perms);

        MapEnvironment GetMapEnvironment() { return m_mapProvider->GetEnvironment(); }

        std::vector<int> GetEntities();
        void RemoveEntity(std::shared_ptr<Entity> e);
        void AddEntity(std::shared_ptr<Entity> e);

        std::mutex BlockChangeMutex;
    protected:
        std::unique_ptr<IMapProvider> m_mapProvider;
    private:
        MapActions m_actions;

        void QueueBlockPhysics(unsigned short X, unsigned short Y, unsigned short Z);

        void QueueBlockChange(unsigned short X, unsigned short Y, unsigned short Z, unsigned char priority,
                              unsigned char oldType);
    };

    class MapMain : TaskItem {
    public:
        MapMain();

        std::shared_ptr<Map> GetPointer(int id);

        std::shared_ptr<Map> GetPointer(std::string name);

        std::shared_ptr<Map> GetPointerUniqueId(std::string uniqueId);

        int GetMapId();

        int Add(int id, short x, short y, short z, const std::string &name);

        void Delete(int id);

        static MapMain *GetInstance();

        static std::string GetMapMOTDOverride(int mapId);

        static int GetMapSize(int x, int y, int z, int blockSize) { return (x * y * z) * blockSize; }

        static int GetMapOffset(int x, int y, int z, int sizeX, int sizeY, int sizeZ, int blockSize) {
            return (x + y * sizeX + z * sizeX * sizeY) * blockSize;
        }

        static Common::Vector3S GetMapExportSize(const std::string &filename);

        void MainFunc();

        void AddSaveAction(int clientId, int mapId, const std::string &directory);

        void AddLoadAction(int clientId, int mapId, const std::string &directory);

        void AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z);

        void AddFillAction(int clientId, int mapId, std::string functionName, std::string argString);

        void AddDeleteAction(int clientId, int mapId);

        bool SaveFile;
        std::map<int, std::shared_ptr<Map>> _maps;
    private:
        static MapMain *Instance;
        std::thread BlockchangeThread;
        std::thread PhysicsThread;
        bool mbcStarted;
        bool phStarted;

        time_t SaveFileTimer;
        std::string TempFilename;
        int TempId;
        std::string TempOverviewFilename;
        long LastWriteTime;

        std::vector<MapActionItem> _mapActions;

        // --
        long mapSettingsLastWriteTime;
        int mapSettingsTimerFileCheck;
        int mapSettingsMaxChangesSec;

        void MapListSave();

        void MapListLoad();

        void MapSettingsSave();

        void MapSettingsLoad();

        void MapBlockChange();

        void MapBlockPhysics();
    };
}

#endif //D3PP_MAP_H
