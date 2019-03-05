#include "Wasabi.h"

class MyApplication : public Wasabi {
public:
	WError Setup() {
		// start the engine
		WError status = StartEngine(640, 480);
		if (!status) {
			// Failed to start the engine...
			return status;
		}
		return status;
	}
	bool Loop(float fDeltaTime) {
		return true; // return true to continue to next frame
	}
	void Cleanup() {
	}
};

Wasabi* WInitialize() {
	return new MyApplication();
}