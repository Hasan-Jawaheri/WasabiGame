# WasabiGame
Game showcase for the Wasabi Engine. This is a WIP port for [this older version, written in HasX11](https://github.com/Hasan-Jawaheri/RPG). 

# Building
This project uses the CMake build system.
- Clone and build [Wasabi](https://github.com/Hasan-Jawaheri/Wasabi) and make sure Vulkan is installed.
- Run
```bash
mkdir build
cd build
cmake -DWASABI_ROOT="<path to Wasabi build/dist folder>" ..
```

# Progress
## Stage 1: Running client
<img src="https://github.com/Hasan-Jawaheri/WasabiGame/raw/master/gitstuff/client-1.png" /> 

# Bugs
- Client disconnects after ~30 seconds, reconnects fail quicker.
- When client disconnects and reconnects, it is supplied with a new unit as the player (duplicates).
    - The server should indicate that a supplied unit is the player's own. The client should be able to only read new updates from that server message.
    - The server will tell the client that it has a new unit id.
    - A better solution is to not immediately delete the player on the server, but give a window where the player can reconnect. If a reocnneciton occurs during that window, everything continues as normal (no authentication required). Otherwise, a permanent disconnect occurs.
