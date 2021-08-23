#include "RTBServer/Cells/MatchmakingCell.hpp"
#include "RTBServer/Cells/MatchCell.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::MatchmakingCell::MatchmakingCell(std::weak_ptr<ServerApplication> app) : ServerCell(app), std::enable_shared_from_this<MatchmakingCell>() {
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
        std::scoped_lock<std::mutex> lock(m_clientsChosenGameModeMutex);
        for (auto gameModeIter = m_clientsChosenGameMode.begin(); gameModeIter != m_clientsChosenGameMode.end(); gameModeIter++) {
            GAME_MODE_MATCHMAKING_PROPERTIES matchmakingProps = GetGameModeMatchmakingProperties(gameModeIter->first);
            while (gameModeIter->second.size() >= matchmakingProps.minPlayers && gameModeIter->second.size()) {
                uint32_t numClientsToTake = std::min(matchmakingProps.maxPlayers, (uint32_t)gameModeIter->second.size());

                // create a new match cell
                std::shared_ptr<MatchCell> newMatchCell = std::make_shared<MatchCell>(m_app, gameModeIter->first);

                // move all clients (registered in the repository) from this cell to the new match cell
                for (int i = 0; i < numClientsToTake; i++) {
                    app->ClientsRepository->MoveClientToCell(gameModeIter->second.at(i), newMatchCell);
                }

                // register cell only after we've moved clients to make sure when cell initialization happens, clients have been moved
                app->RegisterCell(newMatchCell);

                // remove clients from matchmaking map
                gameModeIter->second.erase(gameModeIter->second.begin(), gameModeIter->second.begin() + numClientsToTake);
            }
        }
    }

    return true;
}

void RTBServer::MatchmakingCell::OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {
    GAME_MODE gameMode = (GAME_MODE)((size_t)arg);
    std::scoped_lock<std::mutex> lock(m_clientsChosenGameModeMutex);
    auto iter = m_clientsChosenGameMode.find(gameMode);
    if (iter == m_clientsChosenGameMode.end()) {
        m_clientsChosenGameMode.insert(std::make_pair(gameMode, std::vector<std::shared_ptr<ServerConnectedClient>>()));
        iter = m_clientsChosenGameMode.find(gameMode);
    }
    iter->second.push_back(client);
}
