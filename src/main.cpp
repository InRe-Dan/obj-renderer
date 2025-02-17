#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include "ObjectFile.h"
#include "Camera.h"
#include "font8x8_basic.h"
#include "postprocessing.h"
#include <chrono>
#include <ctime>
#include "Scene.h"
#include "Camera.h"
#include <functional>
#include "SceneCollection.h"

#define WIDTH  640
#define HEIGHT 480

using glm::vec2;
using glm::vec3;
using std::vector;

SceneCollection scenes = SceneCollection();
Scene scene = *(scenes.getCurrent());

vector<vector<uint32_t>> upscaledFrameBuffer;

std::string debugString;
std::chrono::duration<double> frameTime = std::chrono::duration<double>(1);

// Ran when starting program. Initializes buffers.
void initialize()
{
	std::cout << "====================================================\n"
			"Wavefront .obj Model Renderer (C++)\n"
			"University of Bristol COMS30020: Computer Graphics\n"
			"Author: Mihai Daniel Dodoc\n"
			"Find controls and specifications in readme.md!\n"
			"====================================================\n";
	upscaledFrameBuffer = vector<vector<uint32_t>>();
	for (int i = 0; i < HEIGHT; i++)
	{
		upscaledFrameBuffer.push_back(vector<uint32_t>());
		for (int j = 0; j < WIDTH; j++)
		{
			upscaledFrameBuffer.at(i).push_back(0);
		}
	}
}

// Renders a string to the top left corner of the global frame buffer.
void renderDebugString(std::string str)
{
	int yOffset = 8;
	int xOffset = 8;
	for (unsigned char character : str)
	{
		if (character > 127)
			character = '$';
		if (character == '\n')
		{
			yOffset += 12;
			xOffset = 8;
			continue;
		}
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if ((font8x8_basic[character][i] >> j) & 1)
				{
					upscaledFrameBuffer[yOffset + i][xOffset + j] =
						vec3ToColour(vec3(255, 255, 255), 255);
				}
			}
		}
		xOffset += 8;
	}
}

// Called every frame. Fills frame buffer using scene information, and sends to
// SDL wrapper.
void draw(DrawingWindow& window)
{
	// Get camera and clear global frame buffer
	Camera camera = *(scene.getCamera());
	for (size_t y = 0; y < HEIGHT; y++)
	{
		for (size_t x = 0; x < WIDTH; x++)
		{
			upscaledFrameBuffer.at(y).at(x) = 0;
		}
	}

	// Draw background first
	camera.drawFancyBackground();
	// Render objects
	switch (scene.renderMode)
	{
		case 1: camera.rasterRender(scene); break;
		case 2: camera.raytraceRender(scene); break;
		default: camera.wireframeRender(scene); break;
	}
	camera.drawLights(scene);
	// Upscale the camera's frame buffer into the global one
	arbitraryUpscale(camera.frameBuffer, upscaledFrameBuffer);
	// vector<vector<uint32_t>> bufferCopy(upscaledFrameBuffer);
	//  Apply effects if desired
	//  blackAndWhite(bufferCopy);
	//  bufferCopy = applyKernel(bufferCopy, boxBlurKernel);
	//  vector<vector<uint32_t>> hEdges = applyKernel(bufferCopy,
	//  edgeDetectionKernelH); vector<vector<uint32_t>> vEdges =
	//  applyKernel(bufferCopy, edgeDetectionKernelV); hypot(bufferCopy, hEdges,
	//  vEdges); threshold(bufferCopy, vec3(230));
	//  composite(upscaledFrameBuffer, bufferCopy);

	// Generate debug information and write into a strings
	// Get mouse state
	int xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);
	float upscaleFactor = WIDTH / camera.canvasWidth;
	int mouseCanvasX =
		glm::min(camera.canvasWidth - 1, roundI(xMouse / upscaleFactor));
	int mouseCanvasY =
		glm::min(camera.canvasHeight - 1, roundI(yMouse / upscaleFactor));

	// Add generic information
	debugString += "Mouse        : " + std::to_string(xMouse) + ", " +
				   std::to_string(yMouse) + "\n";
	debugString += "Resolution   : " + std::to_string(camera.canvasWidth) +
				   "x" + std::to_string(camera.canvasHeight) + "\n";
	uint32_t colour = upscaledFrameBuffer.at(yMouse).at(xMouse);
	debugString += "RGBA         : " + std::to_string((colour >> 16) & 255);
	debugString += ", " + std::to_string((colour >> 8) & 255);
	debugString += ", " + std::to_string(colour & 255);
	debugString += ", " + std::to_string((colour >> 24) & 255) + "\n";
	debugString += "FOV          : " +
				   std::to_string(roundI(glm::degrees(
					   2 * glm::atan(
							   camera.getImagePlaneWidth() /
							   (2 * camera.getFocalLength()))))) +
				   "\n";

	// Add mode-specific information
	debugString += "\n";
	switch (scene.renderMode)
	{
		case 0:
			debugString += "Mode         : Wireframe \n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) +
				"\n";
			break;
		case 1:
			debugString += "Mode         : Rasterization \n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) +
				"\n";
			break;
		case 2:
			debugString += "Mode         : Raytracing\n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) +
				"\n";
			debugString +=
				"  Threads    : " + std::to_string(camera.threadCount) + "\n";
			debugString += "  Lighting   : " +
						   std::string((scene.lightingEnabled) ? "ON\n" : "OFF\n");
			debugString += "  Textures   : " +
						   std::string((scene.texturesEnabled) ? "ON\n" : "OFF\n");
			debugString += "  Normals    : " +
						   std::string((scene.normalMapsEnabled) ? "ON\n" : "OFF\n");
			debugString +=
				"  Light view : " +
				std::string((scene.lightPositionPreview) ? "ON\n" : "OFF\n");
			debugString +=
				"  Smoothing  : " +
				std::string(scene.smoothingEnabled ? "ON / " : "OFF / ") +
				std::string(
					scene.usingGouraudSmoothing ? "Gouraud (Unimplemented!)\n"
												: "Phong\n");
			debugString +=
				"  Cameras    : " + std::to_string(scene.cameraCount()) + "\n";
			debugString +=
				"  Lights     : " + std::to_string(scene.getLights().size()) +
				"\n";
			if ((scene.getLights().size() <= 4))
			{
				for (int i = 0; i < scene.getLights().size(); i++)
				{
					Light* l = scene.getLights().at(i);
					debugString +=
						(l == scene.getControlledLight()) ? "   >" : "    ";
					debugString += (l->state ? "ON : " : "OFF: ") + l->name +
								   " at " + printVec(l->pos) + "\n";
				}
			}
			break;
		default: debugString += "Mode: Unknown\n";
	}

	// Add orientation matrix
	debugString += '\n';
	glm::mat4 placement = camera.getPlacement();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			debugString += formatFloat(placement[i][j], 5) + " ";
		}
		debugString += "\n";
	}
	// Overlay debug string onto frame buffer
	if (!scene.recording)
		renderDebugString(debugString);

	// Send frame buffer to SDL
	for (size_t y = 0; y < HEIGHT; y++)
	{
		for (size_t x = 0; x < WIDTH; x++)
		{
			window.setPixelColour(x, y, upscaledFrameBuffer.at(y).at(x));
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow& window)
{
	if (event.type == SDL_KEYDOWN)
	{
		SDL_Keycode sym = event.key.keysym.sym;
		// CAMERA CONTROLS
		if (sym == SDLK_RIGHT)
			scene.getCamera()->lookRight(2);
		if (sym == SDLK_LEFT)
			scene.getCamera()->lookLeft(2);
		if (sym == SDLK_UP)
			scene.getCamera()->lookUp(2);
		if (sym == SDLK_DOWN)
			scene.getCamera()->lookDown(2);
		if (sym == SDLK_w)
			scene.getCamera()->moveForward(0.2);
		if (sym == SDLK_s)
			scene.getCamera()->moveBack(0.2);
		if (sym == SDLK_a)
			scene.getCamera()->moveLeft(0.2);
		if (sym == SDLK_d)
			scene.getCamera()->moveRight(0.2);
		if (sym == SDLK_q)
			scene.getCamera()->moveUp(0.2);
		if (sym == SDLK_e)
			scene.getCamera()->moveDown(0.2);
		// if (sym == SDLK_m) scene.getCamera()->toggleOrbit();
		if (sym == SDLK_n)
			scene.getCamera()->toggleLookAt();
		if (sym == SDLK_z)
			scene.getCamera()->changeF(0.1);
		if (sym == SDLK_x)
			scene.getCamera()->changeF(-0.1);
		if (sym == SDLK_l)
			scene.nextCamera();
		if (sym == SDLK_k)
			scene.prevCamera();
		// MODE CONTROLS
		if (sym == SDLK_KP_1)
			scene.renderMode = 0;
		if (sym == SDLK_KP_2)
			scene.renderMode = 1;
		if (sym == SDLK_KP_3)
			scene.renderMode = 2;
		if (sym == SDLK_KP_4)
			scene.lightingEnabled = !scene.lightingEnabled;
		if (sym == SDLK_KP_5)
			scene.texturesEnabled = !scene.texturesEnabled;
		if (sym == SDLK_KP_6)
			scene.normalMapsEnabled = !scene.normalMapsEnabled;
		if (sym == SDLK_KP_7)
			scene.lightPositionPreview = !scene.lightPositionPreview;
		if (sym == SDLK_KP_8)
			scene.toggleAnimation();
		if (sym == SDLK_KP_9)
			scene.smoothingEnabled = !scene.smoothingEnabled;
		if (sym == SDLK_KP_DIVIDE)
			scene.usingGouraudSmoothing = !scene.usingGouraudSmoothing;
		// LIGHT CONTROLS
		if (sym == SDLK_g)
			scene.getControlledLight()->pos += vec3(0, -0.2, 0);
		if (sym == SDLK_t)
			scene.getControlledLight()->pos += vec3(0, 0.2, 0);
		if (sym == SDLK_f)
			scene.getControlledLight()->pos += vec3(-0.2, 0, 0);
		if (sym == SDLK_h)
			scene.getControlledLight()->pos += vec3(0.2, 0, 0);
		if (sym == SDLK_r)
			scene.getControlledLight()->pos += vec3(0, 0, 0.2);
		if (sym == SDLK_y)
			scene.getControlledLight()->pos += vec3(0, 0, -0.2);
		if (sym == SDLK_KP_PLUS)
			scene.nextLight();
		if (sym == SDLK_KP_MINUS)
			scene.prevLight();
		if (sym == SDLK_KP_MULTIPLY)
			scene.getControlledLight()->state =
				!scene.getControlledLight()->state;
		// GENERAL CONTROLS
		if (sym == SDLK_o)
			scene.getCamera()->changeResolutionBy(-32, -24);
		if (sym == SDLK_p)
			scene.getCamera()->changeResolutionBy(32, 24);
		if (sym == SDLK_c)
		{
			scenes.next();
			scene = *scenes.getCurrent();
		}
		if (sym == SDLK_v)
		{
			scenes.prev();
			scene = *scenes.getCurrent();
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		if (event.button.button == SDL_BUTTON_RIGHT)
		{
			window.savePPM("output.ppm");
			window.saveBMP("output.bmp");
		}
		else if (event.button.button == SDL_BUTTON_LEFT)
		{
			// Deprecated debug info
		}
	}
}

// Test function for hand-checking outputs of simple functions.
void test()
{
}

int main(int argc, char* argv[])
{
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	initialize();
	test();
	while (true)
	{
		// First clear the debug string and add framerate (more accurate reading
		// here than elsewhere)
		debugString = "";
		debugString += "FPS: " + std::to_string(1 / frameTime.count());
		// Be rude
		if (frameTime.count() > 1.0f)
		{
			debugString += " - Seconds per frame... " +
						   std::to_string(frameTime.count()) + ". Good luck :)";
		}
		debugString += "\n";
		auto start = std::chrono::system_clock::now();
		// Poll for all events
		if (window.pollForInputEvents(event))
			handleEvent(event, window);
		scene.getCamera()->update();
		scene.update();
		draw(window);
		window.renderFrame();
		frameTime = std::chrono::system_clock::now() - start;
		double frameTime24fps = 1.0f / 24.0f;
		double difference = frameTime24fps - frameTime.count();
		if (difference > 0)
		{
			SDL_Delay((Uint32) (difference * 1000));
		}
		frameTime = std::chrono::system_clock::now() - start;
		if (scene.recording)
		{
			std::cout << "Recording:"
				 << formatFloat(
						(((scene.recordFrame - 1000) / (24.0f * 15.0f)) *
						 100.0f),
						6)
				 << "%\n";
			// To piece together frames:
			// ffmpeg -framerate 24 -pattern_type glob -i 'videoframes/*.ppm'
			// -c:v libx264 -pix_fmt yuv420p output.mp4
			window.savePPM(
				"videoframes/" + std::to_string(scene.recordFrame) + ".ppm");
			scene.recordFrame++;
			if (scene.recordFrame > 1000 + 24 * 15)
			{
				scene.recording = false;
			}
		}
	}
}
