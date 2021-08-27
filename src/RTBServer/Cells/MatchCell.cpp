#include "RTBServer/Cells/MatchCell.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::MatchCell::MatchCell(std::weak_ptr<ServerApplication> app, GAME_MODE gameMode)
        : ServerCell(app), std::enable_shared_from_this<MatchCell>() {
    m_gameMode = CreateGameMode(gameMode, m_app);
}

RTBServer::MatchCell::~MatchCell() {
}

void RTBServer::MatchCell::Initialize() {
    m_gameMode->Initialize(weak_from_this());
}

bool RTBServer::MatchCell::Update() {
    std::shared_ptr<ServerApplication> app = m_app.lock();
    std::shared_ptr<MatchCell> sharedThis = shared_from_this();
    bool isMoreThanOnePlayer = false;
    ServerCellClientsMap* clientsMap = nullptr;
    app->ClientsRepository->LockCellClients(sharedThis, &clientsMap);

    // cell stays alive as long as there are clients
    if (clientsMap != nullptr && clientsMap->size() > 0) {
        isMoreThanOnePlayer = true;
        m_gameMode->Update();
    }

    app->ClientsRepository->UnlockCellClients(sharedThis, &clientsMap);

    return isMoreThanOnePlayer;
}

void RTBServer::MatchCell::Cleanup() {
    m_gameMode->Cleanup();
}

void RTBServer::MatchCell::OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {
    m_gameMode->OnClientAdded(client);
}

void RTBServer::MatchCell::OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) {
    m_gameMode->OnClientRemoved(client);
}