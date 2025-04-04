#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "TextureMap.h"
#include "ModelTriangle.h"
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

constexpr auto WIDTH = 640;
constexpr auto HEIGHT = 480;
static int framelimit = 0;

using glm::vec2;
using glm::vec3;
using std::vector;

SceneCollection scenes = SceneCollection();

vector<vector<uint32_t>> upscaledFrameBuffer;

std::string debugString;
std::chrono::duration<double> frameTime = std::chrono::duration<double>(1);

// Ran when starting program. Initializes buffers.
static void initialize()
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
static void renderDebugString(std::string str)
{
	size_t yOffset = 8;
	size_t xOffset = 8;
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
		for (uint8_t i = 0; i < 8; i++)
		{
			for (uint8_t j = 0; j < 8; j++)
			{
				if ((font8x8_basic[character][i] >> j) & 1)
				{
					upscaledFrameBuffer[yOffset + i][xOffset + j] = 0xffffffff;
				}
			}
		}
		xOffset += 8;
	}
}

// Called every frame. Fills frame buffer using scene information, and sends to
// SDL wrapper.
void draw(DrawingWindow& window, Scene& scene)
{
	Camera& camera = scene.getCamera();
	const Camera::RenderSettings settings = camera.getSettings();
	for (size_t y = 0; y < HEIGHT; y++)
	{
		for (size_t x = 0; x < WIDTH; x++)
		{
			upscaledFrameBuffer.at(y).at(x) = 0;
		}
	}

	// Update camera frame buffer
	auto& result = camera.render(scene);

	// Upscale the camera's frame buffer into the global one
	arbitraryUpscale(result, upscaledFrameBuffer);

	int resx = static_cast<int>(camera.getSettings().resolution.x);
	int resy = static_cast<int>(camera.getSettings().resolution.y);

	// Generate debug information and write into a strings
	// Get mouse state
	float xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);
	float upscaleFactor = WIDTH / static_cast<float>(resx);
	int mouseCanvasX =
		glm::min(resx - 1, roundI(xMouse / upscaleFactor));
	int mouseCanvasY =
		glm::min(resy - 1, roundI(yMouse / upscaleFactor));

	/// TODO rewrite debugstring

	// Add generic information
	debugString += "Mouse        : " + std::to_string(xMouse) + ", " +
				   std::to_string(yMouse) + "\n";
	debugString += "Resolution   : " + std::to_string(resx) +
				   "x" + std::to_string(resy) + "\n";
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
	switch (scene.getCamera().getSettings().mode)
	{
	case Camera::RenderSettings::Mode::Wireframe:
			debugString += "Mode         : Wireframe \n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.getDepth()[mouseCanvasY][mouseCanvasX]) +
				"\n";
			break;
	case Camera::RenderSettings::Mode::Raster:
		debugString += "Mode         : Rasterization \n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.getDepth()[mouseCanvasY][mouseCanvasX]) +
				"\n";
			break;
	case Camera::RenderSettings::Mode::Raytracing:
		debugString += "Mode         : Raytracing\n";
			debugString +=
				"  Depth      : " +
				std::to_string(
					1 / camera.getDepth()[mouseCanvasY][mouseCanvasX]) +
				"\n";
			debugString +=
				"  Threads    : " + std::to_string(settings.threadCount) + "\n";
			debugString += "  Lighting   : " +
						   std::string((settings.lightingEnabled) ? "ON\n" : "OFF\n");
			debugString += "  Textures   : " +
						   std::string((settings.texturesEnabled) ? "ON\n" : "OFF\n");
			debugString += "  Normals    : " +
						   std::string((settings.normalMapsEnabled) ? "ON\n" : "OFF\n");
			debugString +=
				"  Light view : " +
				std::string((settings.lightPositionPreview) ? "ON\n" : "OFF\n");
			debugString +=
				"  Cameras    : " + std::to_string(scene.cameraCount()) + "\n";
			debugString +=
				"  Lights     : " + std::to_string(scene.getLights().size()) +
				"\n";
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

static void handleEvent(SDL_Event event, DrawingWindow& window)
{
	// TODO input event handling on the classes themselves, and mouse inputs for looking
	Scene& scene = scenes.getCurrent();
	Camera& camera = scene.getCamera();
	Camera::RenderSettings settings = camera.getSettings();

	if (event.type == SDL_EVENT_KEY_DOWN)
	{
		SDL_Keycode sym = event.key.key;
		// CAMERA CONTROLS
		if (sym == SDLK_RIGHT)
			scene.getCamera().lookRight(2);
		if (sym == SDLK_LEFT)
			scene.getCamera().lookLeft(2);
		if (sym == SDLK_UP)
			scene.getCamera().lookUp(2);
		if (sym == SDLK_DOWN)
			scene.getCamera().lookDown(2);
		if (sym == SDLK_W)
			scene.getCamera().moveForward(0.2);
		if (sym == SDLK_S)
			scene.getCamera().moveBack(0.2);
		if (sym == SDLK_A)
			scene.getCamera().moveLeft(0.2);
		if (sym == SDLK_D)
			scene.getCamera().moveRight(0.2);
		if (sym == SDLK_Q)
			scene.getCamera().moveUp(0.2);
		if (sym == SDLK_E)
			scene.getCamera().moveDown(0.2);
		if (sym == SDLK_N)
			scene.getCamera().lookAt(nullptr);
		if (sym == SDLK_Z)
			scene.getCamera().changeF(0.1);
		if (sym == SDLK_X)
			scene.getCamera().changeF(-0.1);
		if (sym == SDLK_L)
			scene.nextCamera();
		if (sym == SDLK_K)
			scene.prevCamera();

		// MODE CONTROLS
		if (sym == SDLK_KP_1)
			settings.mode = Camera::RenderSettings::Mode::Wireframe;
		if (sym == SDLK_KP_2)
			settings.mode = Camera::RenderSettings::Mode::Raster;
		if (sym == SDLK_KP_3)
			settings.mode = Camera::RenderSettings::Mode::Raytracing;
		if (sym == SDLK_KP_4)
			settings.lightingEnabled = !settings.lightingEnabled;
		if (sym == SDLK_KP_5)
			settings.texturesEnabled = !settings.texturesEnabled;
		if (sym == SDLK_KP_6)
			settings.normalMapsEnabled = !settings.normalMapsEnabled;
		if (sym == SDLK_KP_7)
			settings.lightPositionPreview = !settings.lightPositionPreview;
		if (sym == SDLK_KP_8)
			scene.toggleAnimation();
		if (sym == SDLK_KP_9)
			// TODO settings.smoothing = !settings.smoothingEnabled;
		if (sym == SDLK_KP_DIVIDE)
			// TODO settings.usingGouraudSmoothing = !settings.usingGouraudSmoothing;

		// LIGHT CONTROLS
		if (sym == SDLK_G)
			scene.getLight().pos += vec3(0, -0.2, 0);
		if (sym == SDLK_T)
			scene.getLight().pos += vec3(0, 0.2, 0);
		if (sym == SDLK_F)
			scene.getLight().pos += vec3(-0.2, 0, 0);
		if (sym == SDLK_H)
			scene.getLight().pos += vec3(0.2, 0, 0);
		if (sym == SDLK_R)
			scene.getLight().pos += vec3(0, 0, 0.2);
		if (sym == SDLK_Y)
			scene.getLight().pos += vec3(0, 0, -0.2);
		if (sym == SDLK_KP_PLUS)
			scene.nextLight();
		if (sym == SDLK_KP_MINUS)
			scene.prevLight();
		if (sym == SDLK_KP_MULTIPLY)
			scene.getLight().on =
				!scene.getLight().on;
		// GENERAL CONTROLS
		if (sym == SDLK_O)
			camera.changeResolutionBy(-32, -24);
		if (sym == SDLK_P)
			camera.changeResolutionBy(32, 24);
		if (sym == SDLK_C) scenes.next();
		if (sym == SDLK_V) scenes.prev();
	}
	else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		if (event.button.button == SDL_BUTTON_RIGHT)
		{
			window.savePPM("output.ppm");
			window.saveBMP("output.bmp");
		}
	}

	camera.updateSettings(settings);

}

int main(int argc, char* argv[])
{
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT);
	SDL_Event event;
	initialize();
	while (true)
	{
		Scene& scene = scenes.getCurrent();
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
		scene.getCamera().update();
		scene.update();
		draw(window, scene);
		window.renderFrame();
		frameTime = std::chrono::system_clock::now() - start;
		if (framelimit > 0)
		{
			double desiredFrametime = 1.0f / framelimit;
			double difference = desiredFrametime - frameTime.count();
			if (difference > 0)
			{
				SDL_Delay((Uint32) (difference * 1000));
			}
		}
		frameTime = std::chrono::system_clock::now() - start;
	}
}
