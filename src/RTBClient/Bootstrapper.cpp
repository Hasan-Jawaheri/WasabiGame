#include "RTBClient/Main.hpp"

Wasabi* WInitialize() {
	std::shared_ptr<RTBClient::ClientApplication> app = std::make_shared<RTBClient::ClientApplication>();
	RunWasabi(app.get());
	return nullptr;
}