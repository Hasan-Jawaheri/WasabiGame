# WasabiGame
Game showcase for the Wasabi Engine. [This was previously a port for an older version, written in HasX11](https://github.com/Hasan-Jawaheri/RPG), but has evolved into a networked game.

## Overview
The repository contains multiple projects that together form a game called RollTheBall(2) (RollTheBall was a project I made a long time ago to compete in some Intel game competition, it wasn't a great success). The game consists of a client and a server that utilize the [Wasabi Engine](https://github.com/Hasan-Jawaheri/Wasabi) to render. The game is planned to have an open-world RPG where the player controls a role-playing ball that will do cool stuff (TBA).

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
Track the progress in the [current project plan](https://github.com/Hasan-Jawaheri/WasabiGame/projects/1).

## Stage 1: Running client
<img src="https://github.com/Hasan-Jawaheri/WasabiGame/raw/master/gitstuff/client-1.png" />

## Stage 2: Client & server communicating
<img src="https://github.com/Hasan-Jawaheri/WasabiGame/raw/master/gitstuff/client-server.png" />

