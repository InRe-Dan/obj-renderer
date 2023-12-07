# UoB Computer Graphics | .obj renderer
Mihai Daniel Dodoc
## Instructions
Compilation and execution can be done with `make speedy`. Upon execution, the program starts with rendering a scene of a generally unmodified Cornell Box via rasterization. You will see a debug display on the top left of the screen which details information about the current scene.
## Controls
- ARROW KEYS: Camera look controls
- W/A: Camera forward/backward
- A/D: Camera left/right
- Q/E: Camera up/down
- N: Toggle target look
- Z: Increase focal length
- X: Decrease focal length
- L/K: Next/Previous camera
- NUMPAD 1: Wireframe mode
- NUMPAD 2: Rasterization mode
- NUMPAD 3: Raytracing mode
- NUMPAD 4: Toggle lighting (Raytracing only)
- NUMPAD 5: Toggle textures (Raytracing only)
- NUMPAD 6: Toggle normalmaps (Raytracing only)
- NUMPAD 7: Toggle light position view
- NUMPAD 8: Toggle animation
- NUMPAD 9: Enable smoothing (Raytracing only)
- NUMPAD /: Change smoothing mode
- NUMPAD +/-: Change light selected
- NUMPAD *: Toggle current light
- T/G: Change current light X
- F/H: Change current light Y
- R/Y: Change current light Z
- O/P: Change current camera resolution
- C/V: Change current scene
- ESC: Quit program

## Features
**In order to achieve a bare pass mark (40%) you must _convincingly_ implement ALL of the following**:
- [X] OBJ geometry file loading
- [X] Wireframe 3D scene rendering
- [X] Export of individual frames and creation of animated video
- [?] Clear incremental evolution of codebase over 3 week assessment period (determined by GitHub analysis)

**For a mark in the 40s you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [X] OBJ material file loading
- [X] Flat colour 3D scene rasterising
- [X] Surface texture mapping
- [X] Keyboard control of camera orientation (using orientation matrix)

**For a mark in the 50s you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [X] Hard Shadow (without soft edges)
- [X] Ambient lighting (minimum threshold or universal supplement)
- [X] Diffuse lighting (proximity and angle-of-incidence)
- [X] Simple animation (e.g. orbit, lookAt, fly-through, tracking/panning, simple transformation of model elements etc.)

**For a mark in the 60s you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [ ]Gouraud shading
- [ ]Rough attempt at soft shadows (using multi-point light sources)
- [ ]Reflective materials (e.g. mirrors, metal etc.)
- [X] Specular lighting (must be a clearly visible specular "spot")

**For a mark in the low 70s you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [ ] Phong Shading
- [ ] Smooth and elegant soft shadows
- [ ] Refractive materials (e.g. glass, water etc.)
- [ ] Complex animation (e.g. complex camera movement, rigging and articulation of model elements etc.)

**To get above 75% you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [X] Normal maps
- [ ] Environment maps (used with reflective materials)
- [ ] Photon maps and caustics
- [ ] VERY creative and sophisticated animation (needs to have the "wow" factor !)

## Disclaimer
This repository and my work is a few weeks older than most, as I was offered a copy of the 2021 repository by another student during the summer. I did some work on basic rasterization of triangles before the unit officially started, and I completed most of the work in November, before the coursework period. All of the code is my own, and any online resources used to write it are marked in comments in relevant areas.