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

## Demo Scenes
Using C and V, you can flip between some hard-coded Scenes. Resolutions and rendering settings vary per-scene.
### Scene 1: Cornell Box
This scene closely mimics what is expected from the lab work and can be used to look at basic lighting, soft shadows, specular reflections as well as some reflections. The boxes have a higher specular exponent than other materials, but I suggest looking for specular spots on the second scene, where there is finer camera control. The camera is able to lock on to the middle of the scene, and will orbit around it if animations are enabled.
### Scene 2: Cornell Box 2
This scene was used during the development of the program to test that all of the implemented features work well together. It features near-full texture and normal-mapping, some reflections and an assortment of animated lights. In this scene, the camera is also fixed on one of the orbiting lights, which is an easy way to get a headache with animations enabled.
### Scene 3: Sphere
This scene was only used to test smooth shading. Phong is partly implemented, but clearly has some kinks to work out.
### Scene 4: Animiation
This is the scene used to render the final ident video to showcase the project. Upon switching to this scene, animation will immediately begin, albeit at a much lower resolution than is available in `output.mp4`. At any point during or after the "15 seconds" of animation, you may press `NUMPAD 8` to regain control of the lights and the camera. Recording is not enabled.
## Features
**In order to achieve a bare pass mark (40%) you must _convincingly_ implement ALL of the following**:
- [X] OBJ geometry file loading
- [X] Wireframe 3D scene rendering
- [X] Export of individual frames and creation of animated video
- [X] Clear incremental evolution of codebase over ~~3~~ 6 week assessment period (determined by GitHub analysis)

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
- [ ] Gouraud shading
- [X] Rough attempt at soft shadows (using multi-point light sources)
- [X] Reflective materials (e.g. mirrors, metal etc.)
- [X] Specular lighting (must be a clearly visible specular "spot")

**For a mark in the low 70s you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [?] Phong Shading (Attempted.)
- [ ] Smooth and elegant soft shadows
- [ ] Refractive materials (e.g. glass, water etc.)
- [ ] Complex animation (e.g. complex camera movement, rigging and articulation of model elements etc.)

**To get above 75% you must _convincingly_ implement ALL above features, as well as SOME of the following**:
- [X] Normal maps
- [ ] Environment maps (used with reflective materials)
- [ ] Photon maps and caustics
- [ ] VERY creative and sophisticated animation (needs to have the "wow" factor !)

## Additional Features
These are features that are not listed in the project specification, and only some of them can be directly seen in the final build of the project due to the fact that I wasn't able to easily add runtime controls for them. For features inaccessible at runtime but visible in code, I have added code locations:
- Various postprocessing effects in `postprocessing.cpp`
  - Convolution application: Blurring, sharpening, Sobel filters
  - Black and White filters with correction for human vision
  - Compositing, to be used in conjunction with edge detection to create outlines
  - Arbitrary nearest-neighbour upscaling (Downscaling not included.)
    - Consequently, a fairly flexible resolution system
- Light quantization for less realistic renders in `vecutil.cpp:quantize`
- Light location approximations, which are toggleable in release
- A multiple camera and multiple scene system
- Textured rasterized triangles can be seen at some point in the git history, but they have long been deprecated

## Disclaimers and attribution
Most of the extended material library comes from [https://3dtextures.me/](https://3dtextures.me/).

8x8 font for debugging purposes provided by [dhepper](https://github.com/dhepper/font8x8).

Most of the code, libraries aside, is completely original. If a snippet was derived from a resource, such as a guide or a paper, it will be stated in comments (assuming that the method or algorithm is not from lesson material).

Homogenous coordinates were "implemented", but I realised far too late that I was using them wrong and not to their full extent.
