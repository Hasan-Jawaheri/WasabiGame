#pragma once
#include "RTBServer/Main.hpp"


namespace RTBServer {
    
    class ServerApplication;
	class ServerConnectedClient;

    enum class SERVER_CELL_TYPE : uint8_t {
        CELL_TYPE_LOGIN = 0,
        CELL_TYPE_MATCHMAKING = 1,
        CELL_TYPE_MATCH = 2,
    };

    /**
     * ALL FUNCTION CALLS OF THIS INTERFACE MUST TAKE CARE OF SYNCHRONIZATION, THEY CAN ALL HAPPEN
     * IN ANY ORDER IN ANY THREAD.
     */
    class ServerCell {

        static std::atomic<uint32_t> ATOMIC_ID;

    protected:
        std::weak_ptr<ServerApplication> m_app;
        uint32_t m_id;

    public:
        ServerCell(std::weak_ptr<ServerApplication> app) : m_app(app) { m_id = ++ATOMIC_ID; }
        virtual ~ServerCell() {}

        uint32_t GetId() { return m_id; }

        virtual SERVER_CELL_TYPE GetType() const = 0;
        virtual void Initialize() {}
        virtual bool Update() = 0;
        virtual void Cleanup() {}

        virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {}
        virtual void OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) {}
    };

};
