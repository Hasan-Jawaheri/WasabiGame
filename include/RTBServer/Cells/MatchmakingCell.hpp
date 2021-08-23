#pragma once
#include "RTBServer/Cells/ServerCell.hpp"
#include "WasabiGame/Networking/Selectable.hpp"
#include "WasabiGame/Networking/Data.hpp"
#include "RTBServer/Game/GameModes/GameMode.hpp"


namespace RTBServer {

    class MatchmakingCell : public ServerCell, public std::enable_shared_from_this<MatchmakingCell> {

        float m_lastUpdateTime;
        float m_updatePeriod;

        std::mutex m_clientsChosenGameModeMutex;
        std::unordered_map<GAME_MODE, std::vector<std::shared_ptr<ServerConnectedClient>>> m_clientsChosenGameMode;

    public:
        MatchmakingCell(std::weak_ptr<ServerApplication> app);
        virtual ~MatchmakingCell();

        virtual SERVER_CELL_TYPE GetType() const override { return RTBServer::SERVER_CELL_TYPE::CELL_TYPE_MATCHMAKING; }
        virtual bool Update() override;

        virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) override;
    };

};
