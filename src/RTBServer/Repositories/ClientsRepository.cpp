#include "RTBServer/Repositories/ClientsRepository.hpp"


void RTBServer::ServerClientsRepository::SetClientConnected(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> initialCell, void* arg) {
    std::shared_ptr<CELL_DATA> cellData;

    {
        std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);
        auto clientsIter = m_clientsDataByCellId.find(initialCell->GetId());
        if (clientsIter == m_clientsDataByCellId.end()) {
            cellData = std::make_shared<CELL_DATA>();
            cellData->cell = initialCell;
            m_clientsDataByCellId.insert(std::make_pair(initialCell->GetId(), cellData));
        } else
            cellData = clientsIter->second;
    }

    {
        std::scoped_lock<std::recursive_mutex> lock(cellData->mutex);
        cellData->clientsMap.insert(std::make_pair(client->m_id, client));
    }

    initialCell->OnClientAdded(client, arg);
}

void RTBServer::ServerClientsRepository::SetClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
    std::shared_ptr<ServerCell> cell;

    {
        std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);
        for (auto clientsIter : m_clientsDataByCellId) {
            std::scoped_lock<std::recursive_mutex> lock(clientsIter.second->mutex);
            if (clientsIter.second->clientsMap.erase(client->m_id) > 0) {
                if (clientsIter.second->clientsMap.size() == 0)
                    m_clientsDataByCellId.erase(clientsIter.second->cell->GetId());
                cell = clientsIter.second->cell;
                break;
            }
        }
    }

    if (cell != nullptr)
        cell->OnClientRemoved(client);
}

void RTBServer::ServerClientsRepository::MoveClientToCell(std::shared_ptr<RTBServer::ServerConnectedClient> client, std::shared_ptr<ServerCell> newCell, void* arg) {
    std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);

    SetClientDisconnected(client);
    SetClientConnected(client, newCell, arg);
}

void RTBServer::ServerClientsRepository::LockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap) {
    std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);
    auto clientsIter = m_clientsDataByCellId.find(cell->GetId());
    if (clientsIter == m_clientsDataByCellId.end()) {
        *clientsMap = nullptr;
    } else {
        clientsIter->second->mutex.lock();
        *clientsMap = &clientsIter->second->clientsMap;
    }
}

void RTBServer::ServerClientsRepository::UnlockCellClients(std::shared_ptr<ServerCell> cell, ServerCellClientsMap** clientsMap) {
    std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);
    auto clientsIter = m_clientsDataByCellId.find(cell->GetId());
    if (clientsIter != m_clientsDataByCellId.end())
        clientsIter->second->mutex.unlock();
    *clientsMap = nullptr;
}
