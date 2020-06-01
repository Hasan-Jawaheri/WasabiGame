#pragma once
#include "RTBServer/Main.hpp"
#include "RTBServer/Cells/ServerCell.hpp"
#include "RTBServer/Cells/ServerCellType.hpp"


namespace RTBServer {
    
	class ServerConnectedClient;
    class ServerCell;

    typedef std::unordered_map<uint32_t, std::shared_ptr<ServerConnectedClient>> ServerCellClientsMap;

    class ServerClientsRepository {
        struct CELL_DATA {
            std::recursive_mutex mutex;
            ServerCellClientsMap clientsMap;
        };

        std::recursive_mutex m_clientsDataByCellMutex;
        std::unordered_map<std::shared_ptr<ServerCell>, std::shared_ptr<CELL_DATA>> m_clientsDataByCell;

    public:

        void SetClientConnected(std::shared_ptr<ServerConnectedClient> client, std::shared_ptr<ServerCell> initialCell, void* arg = nullptr);
        void SetClientDisconnected(std::shared_ptr<ServerConnectedClient> client);
        void MoveClientToCell(std::shared_ptr<ServerConnectedClient> client, std::shared_ptr<ServerCell> newCell, void* arg = nullptr);

        void LockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap);
        void UnlockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap);
    };

};
