#include "RTBServer/Repositories/ClientsRepository.hpp"


void RTBServer::ServerClientsRepository::SetClientConnected(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> initialCell, void* arg) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsMutex);

    auto clientsIter = m_clients.find(initialCell);
    if (clientsIter == m_clients.end()) {
        clientsIter = m_clients.insert(m_clients.begin(), std::make_pair(initialCell, std::unordered_map<uint32_t, std::shared_ptr<RTBServer::ServerConnectedClient>>()));
    }
    clientsIter->second.insert(std::make_pair(client->m_id, client));
    initialCell->OnClientAdded(client, arg);
}

void RTBServer::ServerClientsRepository::SetClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsMutex);

    for (auto clientsIter : m_clients) {
        if (clientsIter.second.erase(client->m_id) > 0) {
            clientsIter.first->OnClientRemoved(client);
            if (clientsIter.second.size() == 0)
                m_clients.erase(clientsIter.first);
            break;
        }
    }
}

void RTBServer::ServerClientsRepository::MoveClientToCell(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> newCell, void* arg) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsMutex);

    SetClientDisconnected(client);
    SetClientConnected(client, newCell, arg);
}
