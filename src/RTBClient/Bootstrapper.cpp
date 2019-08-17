#include <WinSock2.h>
#include "RTBClient/Main.hpp"

Wasabi* WInitialize() {
	return new RTBClient();
}