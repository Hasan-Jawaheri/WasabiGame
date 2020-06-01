#include "RTBServer/Cells/LoginCell.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"

#include "RollTheBall/GameModes/GameModes.hpp"


RTBServer::LoginCell::LoginCell(std::shared_ptr<ServerApplication> app) : ServerCell(app) {
	// login update callback
	app->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN, [this](std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& loginUpdate) {
        return this->OnClientLoginUpdate(_client, loginUpdate);
	});

    app->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SELECT_GAME_MODE, [this](std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& gameModeUpdate) {
        return this->OnClientSelectedGameMode(_client, gameModeUpdate);
    });
}

RTBServer::LoginCell::~LoginCell() {
}

bool RTBServer::LoginCell::Update() {
    return true;
}

bool RTBServer::LoginCell::OnClientLoginUpdate(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& loginUpdate) {
    std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
    std::shared_ptr<ServerApplication> app = m_app.lock();
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
    return false;
}

bool RTBServer::LoginCell::OnClientSelectedGameMode(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& gameModeUpdate) {
    std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
    std::shared_ptr<ServerApplication> app = m_app.lock();
    uint32_t gameMode;
    if (RollTheBall::UpdateBuilders::ReadSelectGameMode(gameModeUpdate, gameMode)) {
        switch (gameMode) {
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
