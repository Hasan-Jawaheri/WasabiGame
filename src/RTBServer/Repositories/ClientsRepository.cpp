#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::ServerClientsRepository::ServerClientsRepository() {
    int x = 0;
}

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
        std::scoped_lock<std::recursive_mutex> cellLock(cellData->mutex);
        cellData->clientsMap.insert(std::make_pair(client->m_id, client));
        std::scoped_lock<std::recursive_mutex> cellDataByClientIdLock(m_cellDataByClientIdMutex);
        m_cellDataByClientId.insert(std::make_pair(client->m_id, cellData));
    }

    initialCell->OnClientAdded(client, arg);
}

void RTBServer::ServerClientsRepository::SetClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
    std::shared_ptr<CELL_DATA> cell;

    {
        std::scoped_lock<std::recursive_mutex> cellDataByClientIdLock(m_cellDataByClientIdMutex);
        auto iter = m_cellDataByClientId.find(client->m_id);
        if (iter == m_cellDataByClientId.end())
            return; // can't find client
        cell = iter->second;
    }

    // we need to release m_cellDataByClientIdMutex then hold cell mutex before m_cellDataByClientIdMutex to avoid deadlock with SetClientConnected
    {
        std::scoped_lock<std::recursive_mutex> cellLock(cell->mutex);
        cell->clientsMap.erase(client->m_id);
        if (cell->clientsMap.size() == 0) {
            std::scoped_lock<std::recursive_mutex> lock(m_clientsDataByCellMutex);
            m_clientsDataByCellId.erase(cell->cell->GetId());
        }


        std::scoped_lock<std::recursive_mutex> cellDataByClientIdLock(m_cellDataByClientIdMutex);
        m_cellDataByClientId.erase(client->m_id);
    }

    if (cell != nullptr)
        cell->cell->OnClientRemoved(client);
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

std::shared_ptr<RTBServer::ServerCell> RTBServer::ServerClientsRepository::GetClientCell(uint32_t clientId) {
    std::scoped_lock<std::recursive_mutex> lock(m_cellDataByClientIdMutex);
    auto iter = m_cellDataByClientId.find(clientId);
    if (iter != m_cellDataByClientId.end())
        return iter->second->cell;
    return nullptr;
}
