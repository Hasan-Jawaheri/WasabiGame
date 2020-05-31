#pragma once
#include "RTBServer/Main.hpp"
#include "RTBServer/Cells/ServerCellType.hpp"


namespace RTBServer {
    
    class ServerApplication;
	class ServerConnectedClient;

    /**
     * ALL FUNCTION CALLS OF THIS INTERFACE MUST TAKE CARE OF SYNCHRONIZATION, THEY CAN ALL HAPPEN
     * IN ANY ORDER IN ANY THREAD.
     */
    class ServerCell {
    protected:
        std::weak_ptr<ServerApplication> m_app;

    public:
        ServerCell(std::shared_ptr<ServerApplication> app) : m_app(app) {}
        virtual ~ServerCell() {}

        virtual SERVER_CELL_TYPE GetType() const = 0;
        virtual bool Update() = 0;

        virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {}
        virtual void OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) {}
    };

};
