#pragma once
#include "RTBServer/Cells/ServerCell.hpp"
#include "RTBServer/Game/GameModes/GameMode.hpp"


namespace RTBServer {

    class MatchCell : public ServerCell, public std::enable_shared_from_this<MatchCell> {
        std::shared_ptr<GameMode> m_gameMode;

    public:
        MatchCell(std::weak_ptr<ServerApplication> app, GAME_MODE gameMode);
        virtual ~MatchCell();

        virtual SERVER_CELL_TYPE GetType() const override { return RTBServer::SERVER_CELL_TYPE::CELL_TYPE_MATCH; }
        virtual void Initialize() override;
        virtual bool Update() override;
        virtual void Cleanup() override;

        virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) override;
        virtual void OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) override;
        virtual bool OnReceivedNetworkUpdate(std::shared_ptr<ServerConnectedClient> client, WasabiGame::NetworkUpdate update) override;
    };

};
