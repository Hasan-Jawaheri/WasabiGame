#include "RTBServer/Repositories/ClientsRepository.hpp"


void RTBServer::ServerClientsRepository::SetClientConnected(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> initialCell, void* arg) {
    std::shared_ptr<CELL_DATA> cellData;

    {
        std::lock_guard<std::recursive_mutex> lock(m_clientsDataByCellMutex);
        auto clientsIter = m_clientsDataByCell.find(initialCell);
        if (clientsIter == m_clientsDataByCell.end()) {
            cellData = std::make_shared<CELL_DATA>();
            m_clientsDataByCell.insert(std::make_pair(initialCell, cellData));
        } else
            cellData = clientsIter->second;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(cellData->mutex);
        cellData->clientsMap.insert(std::make_pair(client->m_id, client));
    }

    initialCell->OnClientAdded(client, arg);
}

void RTBServer::ServerClientsRepository::SetClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
    std::shared_ptr<ServerCell> cell;

    {
        std::lock_guard<std::recursive_mutex> lock(m_clientsDataByCellMutex);
        for (auto clientsIter : m_clientsDataByCell) {
            std::lock_guard<std::recursive_mutex> lock(clientsIter.second->mutex);
            if (clientsIter.second->clientsMap.erase(client->m_id) > 0) {
                if (clientsIter.second->clientsMap.size() == 0)
                    m_clientsDataByCell.erase(cell);
                cell = clientsIter.first;
                break;
            }
        }
    }

    if (cell != nullptr)
        cell->OnClientRemoved(client);
}

void RTBServer::ServerClientsRepository::MoveClientToCell(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> newCell, void* arg) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsDataByCellMutex);

    SetClientDisconnected(client);
    SetClientConnected(client, newCell, arg);
}

void RTBServer::ServerClientsRepository::LockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsDataByCellMutex);
    auto clientsIter = m_clientsDataByCell.find(cell);
    if (clientsIter == m_clientsDataByCell.end()) {
        *clientsMap = nullptr;
    } else {
        clientsIter->second->mutex.lock();
        *clientsMap = &clientsIter->second->clientsMap;
    }
}

void RTBServer::ServerClientsRepository::UnlockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap) {
    std::lock_guard<std::recursive_mutex> lock(m_clientsDataByCellMutex);
    auto clientsIter = m_clientsDataByCell.find(cell);
    if (clientsIter != m_clientsDataByCell.end())
        clientsIter->second->mutex.unlock();
    *clientsMap = nullptr;
}
