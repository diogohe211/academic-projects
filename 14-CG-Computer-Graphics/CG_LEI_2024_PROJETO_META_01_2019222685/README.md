# Frogger 3D

A Frogger-inspired game written in C++ with openFrameworks and OpenGL. Developed by Diogo Henriques for Milestone 1 of the Computer Graphics course at the University of Coimbra (2024/25).

Guide the frog across a busy road and a river to reach the far side of the board. Dodge moving cars and cross the river on logs or lily pads. Logs carry the frog as they move. A car collision or a fall into the river ends the run and triggers a particle effect.

## Graphics and game logic

- The frog, cars and logs are built from OpenGL primitives. Matrix transformations position and scale the parts of each model.
- Three views share the same scene and gameplay. The camera can switch between top-down, angled perspective and the frog’s first-person view during play.
- Axis-aligned bounding boxes detect overlap with cars, logs and lily pads in the XY plane.

## Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move the frog one grid cell |
| V | Cycle through the three camera views |
| R | Restart the game |
| G | Switch to wireframe rendering |
| F | Switch back to filled rendering |
| B | Toggle car hitbox overlays |

## Build and run on Windows

Use Visual Studio 2022 with C++ support and the Visual Studio distribution of openFrameworks 0.12.0.

1. Use the openFrameworks Project Generator to create a C++ project named **Frogger** under the openFrameworks directory, for example in `apps/myApps/Frogger`.
2. Copy the `.cpp` and `.h` files from this folder into the generated project’s `src` directory, replacing the template files.
3. Update the generated project so Visual Studio includes all three `.cpp` files, then open `Frogger.sln` and build and run it.

The rendering code uses legacy OpenGL calls such as `glBegin` and `glMatrixMode`. Keep the default openFrameworks context settings used in [main.cpp](main.cpp).

## Source files

| File | Role |
| --- | --- |
| [main.cpp](main.cpp) | Creates the window and starts the app |
| [ofApp.cpp](ofApp.cpp) and [ofApp.h](ofApp.h) | Game state, movement, collision checks, drawing and keyboard input |
| [cg_cam_extras.h](cg_cam_extras.h) and [cg_drawing_extras.h](cg_drawing_extras.h) | Camera, projection and primitive-drawing helpers |
| [cg_game_particle.cpp](cg_game_particle.cpp) and [cg_game_particle.h](cg_game_particle.h) | Particle effect used when the player loses |
| [cg_extras.h](cg_extras.h) | Course utilities credited in the source to André Perrotta and Evgheni Polisciuc |
