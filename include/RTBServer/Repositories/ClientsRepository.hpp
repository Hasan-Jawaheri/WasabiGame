#pragma once
#include "RTBServer/Main.hpp"
#include "RTBServer/Cells/ServerCell.hpp"
#include "RTBServer/Cells/ServerCellType.hpp"


namespace RTBServer {
    
	class ServerConnectedClient;
    class ServerCell;

    class ServerClientsRepository {
        std::recursive_mutex m_clientsMutex;
        std::unordered_map<std::shared_ptr<ServerCell>, std::unordered_map<uint32_t, std::shared_ptr<ServerConnectedClient>>> m_clients;

    public:

        void SetClientConnected(std::shared_ptr<ServerConnectedClient> client, std::shared_ptr<ServerCell> initialCell, void* arg = nullptr);
        void SetClientDisconnected(std::shared_ptr<ServerConnectedClient> client);

        void MoveClientToCell(std::shared_ptr<ServerConnectedClient> client, std::shared_ptr<ServerCell> newCell, void* arg = nullptr);
    };

};
