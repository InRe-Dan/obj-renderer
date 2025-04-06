#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"
#include "ObjectFile.h"
#include "Camera.h"
#include "Scene.h"
#include "SceneCollection.h"

#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <chrono>
#include <ctime>
#include <functional>
#include <font8x8/font8x8_basic.h>
#include <format>

constexpr auto WIDTH = 640;
constexpr auto HEIGHT = 480;
static int framelimit = 0;

SceneCollection scenes = SceneCollection();

std::string debugString;
std::chrono::duration<double> frameTime = std::chrono::duration<double>(1);

// Ran when starting program
static void initialize()
{
	std::cout << 
		"====================================================\n"
		"Wavefront .obj Model Renderer (C++)\n"
		"University of Bristol COMS30020: Computer Graphics\n"
		"Author: Mihai Daniel Dodoc\n"
		"Find controls and specifications in readme.md!\n"
		"====================================================\n";
}

// Renders a string to the top left corner of the global frame buffer.
static void renderDebugString(std::string str, std::vector<std::vector<glm::vec4>>& buffer)
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
					buffer[yOffset + i][xOffset + j] = glm::vec4(1.0);
				}
			}
		}
		xOffset += 8;
	}
}

// Called every frame. Fills frame buffer using scene information, and sends to
// SDL wrapper.
static void draw(DrawingWindow& window, Scene& scene)
{
	Camera& camera = scene.getCamera();
	const Camera::RenderSettings settings = camera.getSettings();

	// Update camera frame buffer
	std::vector<std::vector<glm::vec4>> result = camera.render(scene);

	long resx = static_cast<long>(camera.getSettings().resolution.x);
	long resy = static_cast<long>(camera.getSettings().resolution.y);

	// Generate debug information and write into a strings
	// Get mouse state
	float xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);
	float upscaleFactor = WIDTH / static_cast<float>(resx);
	long mouseCanvasX =
		glm::min(resx - 1, std::lround(xMouse / upscaleFactor));
	long mouseCanvasY =
		glm::min(resy - 1, std::lround(yMouse / upscaleFactor));

	/// TODO rewrite debugstring

	// debugString += std::format("{10:}: {}, {}\n", "Mouse", xMouse, yMouse);
	// debugString += std::format("{10:}: {}, {}\n", "Resolution", resx, resy);
	// debugString += std::format("{10:}: {}\n", "RGBA", unpack(result.at(yMouse).at(xMouse)));
	// debugString += std::format("{10:}: {}\n", "FOV", glm::degrees(2 * glm::atan(camera.getImagePlaneWidth() / (2 * camera.getFocalLength()))));

	// Add mode-specific information
	debugString += "\n";
	switch (scene.getCamera().getSettings().mode)
	{
	case Camera::RenderSettings::Mode::Raytracing:
		debugString += "Mode         : Raytracing\n";
		debugString +=
			"  Depth      : " +
			std::to_string(
				1.f / camera.getDepth()[mouseCanvasY][mouseCanvasX]) +
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
			debugString += std::format("{}", placement[i][j], 5);
		}
		debugString += "\n";
	}

	renderDebugString(debugString, result);

	// Send frame buffer to SDL
	for (size_t y = 0; y < result.size(); y++)
	{
		for (size_t x = 0; x < result[0].size(); x++)
		{
			window.setPixelColour(x, y, pack(result.at(y).at(x)));
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
			scene.getCamera().lookRight(2.f);
		if (sym == SDLK_LEFT)
			scene.getCamera().lookLeft(2.f);
		if (sym == SDLK_UP)
			scene.getCamera().lookUp(2.f);
		if (sym == SDLK_DOWN)
			scene.getCamera().lookDown(2.f);
		if (sym == SDLK_W)
			scene.getCamera().moveForward(0.2f);
		if (sym == SDLK_S)
			scene.getCamera().moveBack(0.2f);
		if (sym == SDLK_A)
			scene.getCamera().moveLeft(0.2f);
		if (sym == SDLK_D)
			scene.getCamera().moveRight(0.2f);
		if (sym == SDLK_Q)
			scene.getCamera().moveUp(0.2f);
		if (sym == SDLK_E)
			scene.getCamera().moveDown(0.2f);
		if (sym == SDLK_N)
			scene.getCamera().lookAt(nullptr);
		if (sym == SDLK_Z)
			scene.getCamera().changeF(0.1f);
		if (sym == SDLK_X)
			scene.getCamera().changeF(-0.1f);
		if (sym == SDLK_L)
			scene.nextCamera();
		if (sym == SDLK_K)
			scene.prevCamera();

		// MODE CONTROLS
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
		if (sym == SDLK_KP_9)
			// TODO settings.smoothing = !settings.smoothingEnabled;
			if (sym == SDLK_KP_DIVIDE)
				// TODO settings.usingGouraudSmoothing = !settings.usingGouraudSmoothing;

			// LIGHT CONTROLS
				if (sym == SDLK_G)
					scene.getLight().pos += glm::vec3(0, -0.2, 0);
		if (sym == SDLK_T)
			scene.getLight().pos += glm::vec3(0, 0.2, 0);
		if (sym == SDLK_F)
			scene.getLight().pos += glm::vec3(-0.2, 0, 0);
		if (sym == SDLK_H)
			scene.getLight().pos += glm::vec3(0.2, 0, 0);
		if (sym == SDLK_R)
			scene.getLight().pos += glm::vec3(0, 0, 0.2);
		if (sym == SDLK_Y)
			scene.getLight().pos += glm::vec3(0, 0, -0.2);
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

int main()
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
		draw(window, scene);
		window.renderFrame();
		frameTime = std::chrono::system_clock::now() - start;
		if (framelimit > 0)
		{
			double desiredFrametime = 1.0f / framelimit;
			double difference = desiredFrametime - frameTime.count();
			if (difference > 0)
			{
				SDL_Delay((Uint32)(difference * 1000));
			}
		}
		frameTime = std::chrono::system_clock::now() - start;
	}
}
