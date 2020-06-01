#include "RTBServer/Cells/MatchmakingCell.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::MatchmakingCell::MatchmakingCell(std::shared_ptr<ServerApplication> app) : ServerCell(app), std::enable_shared_from_this<MatchmakingCell>() {
    m_updatePeriod = m_app.lock()->Config->Get<float>("MatchmakingCellUpdatePeriodS");
    m_lastUpdateTime = 0.0f;
}

RTBServer::MatchmakingCell::~MatchmakingCell() {
}

bool RTBServer::MatchmakingCell::Update() {
    std::shared_ptr<ServerApplication> app = m_app.lock();
    std::shared_ptr<MatchmakingCell> sharedThis = shared_from_this();

    float curTime = app->Timer.GetElapsedTime();
    if (m_lastUpdateTime + m_updatePeriod < curTime) {
        m_lastUpdateTime = curTime;
        ServerCellClientsMap* clientsMap = nullptr;
        app->ClientsRepository->LockCellClients(sharedThis, &clientsMap);
        while (clientsMap->size() >= 2) {
            // create a new match cell
            std::shared_ptr<ServerCell> newMatchCell = nullptr;
            app->RegisterCell(newMatchCell);

            auto iter = clientsMap->begin();
            std::shared_ptr<ServerConnectedClient> client1 = (iter++)->second;
            std::shared_ptr<ServerConnectedClient> client2 = (iter++)->second;
            app->ClientsRepository->MoveClientToCell(client1, newMatchCell);
            app->ClientsRepository->MoveClientToCell(client2, newMatchCell);
        }
        app->ClientsRepository->UnlockCellClients(sharedThis, &clientsMap);
    }

    return true;
}

void RTBServer::MatchmakingCell::OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {
}
