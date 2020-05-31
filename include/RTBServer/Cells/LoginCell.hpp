#pragma once
#include "RTBServer/Cells/ServerCell.hpp"
#include "WasabiGame/Networking/Selectable.hpp"
#include "WasabiGame/Networking/Data.hpp"


namespace RTBServer {

    class LoginCell : public ServerCell {
        bool OnClientLoginUpdate(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& loginUpdate);
        bool OnClientSelectedGameMode(std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& gameModeUpdate);

    public:
        LoginCell(std::shared_ptr<ServerApplication> app);
        virtual ~LoginCell();

        virtual SERVER_CELL_TYPE GetType() const override { return RTBServer::SERVER_CELL_TYPE::CELL_TYPE_LOGIN; }
        virtual bool Update() override;
        
		bool Authenticate(WasabiGame::ClientIdentity& identity);
    };

};
