#include "RTBServer/Cells/MatchmakingCell.hpp"
#include "RTBServer/Main.hpp"


RTBServer::MatchmakingCell::MatchmakingCell(std::shared_ptr<ServerApplication> app) : ServerCell(app) {
}

RTBServer::MatchmakingCell::~MatchmakingCell() {
}

bool RTBServer::MatchmakingCell::Update() {
    return true;
}

void RTBServer::MatchmakingCell::OnClientAdded(std::shared_ptr<ServerConnectedClient> client, void* arg) {
}
