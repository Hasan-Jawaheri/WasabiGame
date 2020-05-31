#pragma once
#include "RTBServer/Cells/ServerCell.hpp"
#include "WasabiGame/Networking/Selectable.hpp"
#include "WasabiGame/Networking/Data.hpp"


namespace RTBServer {

    class MatchmakingCell : public ServerCell {

    public:
        MatchmakingCell(std::shared_ptr<ServerApplication> app);
        virtual ~MatchmakingCell();

        virtual SERVER_CELL_TYPE GetType() const override { return RTBServer::SERVER_CELL_TYPE::CELL_TYPE_MATCHMAKING; }
        virtual bool Update() override;

        virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) override;
    };

};
