#include "RTBServer/Cells/LoginCell.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"

#include "RollTheBall/GameModes/GameModes.hpp"


RTBServer::LoginCell::LoginCell(std::weak_ptr<ServerApplication> app) : ServerCell(app), std::enable_shared_from_this<LoginCell>() {
}

RTBServer::LoginCell::~LoginCell() {
}

bool RTBServer::LoginCell::Update() {
    return true;
}

bool RTBServer::LoginCell::OnReceivedNetworkUpdate(std::shared_ptr<ServerConnectedClient> client, WasabiGame::NetworkUpdate update) {
    // first message from the client must be a login message
    if (static_cast<RollTheBall::NetworkUpdateTypeEnum>(update.type) != RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN && client->Identity.accountName[0] == 0)
        return false;

    switch (static_cast<RollTheBall::NetworkUpdateTypeEnum>(update.type)) {
    case RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN:
        return OnClientLoginUpdate(client, update);
    case RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SELECT_GAME_MODE:
        return OnClientSelectedGameMode(client, update);
    }

    // unknown message for this cell
    return false;
}

bool RTBServer::LoginCell::OnClientLoginUpdate(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& loginUpdate) {
    std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
    std::shared_ptr<ServerApplication> app = m_app.lock();
    std::shared_ptr<ServerCell> sharedThis = shared_from_this();
    ServerCellClientsMap* clientsMap;
    bool clientIsInLoginCell = false;
    app->ClientsRepository->LockCellClients(sharedThis, &clientsMap);
    clientIsInLoginCell = clientsMap && clientsMap->find(client->m_id) != clientsMap->end();
    app->ClientsRepository->UnlockCellClients(sharedThis, &clientsMap);

    if (clientIsInLoginCell) {
        WasabiGame::ClientIdentity identity;
        if (RollTheBall::UpdateBuilders::ReadLoginPacket(loginUpdate, identity)) {
            bool authenticated = Authenticate(identity);
            if (authenticated) {
                memcpy(&client->Identity, &identity, sizeof(WasabiGame::ClientIdentity));
                LOG_F(INFO, "AUTHENTICATED: id=%d, account=%s", client->m_id, client->Identity.accountName);
            }

            WasabiGame::NetworkUpdate loginStatusUpdate;
            RollTheBall::UpdateBuilders::LoginStatus(loginStatusUpdate, authenticated);
            app->Networking->SendUpdate(client, loginStatusUpdate);
            return authenticated;
        }
    }
    return false;
}

bool RTBServer::LoginCell::OnClientSelectedGameMode(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& gameModeUpdate) {
    std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
    std::shared_ptr<ServerApplication> app = m_app.lock();
    uint32_t gameMode;
    if (RollTheBall::UpdateBuilders::ReadSelectGameModePacket(gameModeUpdate, gameMode)) {
        switch (static_cast<RollTheBall::RTB_GAME_MODE>(gameMode)) {
        case RollTheBall::RTB_GAME_MODE::GAME_MODE_ONE_VS_ONE:
            app->ClientsRepository->MoveClientToCell(client, app->GetMatchmakingCell(), (void*)(size_t)gameMode);
            break;
        default:
            LOG_F(WARNING, "Client id=%d, account=%s requested invalid game mode: %d", client->m_id, client->Identity.accountName, gameMode);
            return false;
        }
        return true;
        
    }
    return false;
}

bool RTBServer::LoginCell::Authenticate(WasabiGame::ClientIdentity& identity) {
	return std::string(identity.accountName).find("ghandi") == 0 && std::string(identity.passwordHash) == "123456";
}
