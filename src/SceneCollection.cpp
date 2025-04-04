#include "SceneCollection.h"

#include "Scene.h"
#include "Camera.h"

#include <filesystem>

using namespace glm;

SceneCollection::SceneCollection()
{
	sceneVect.push_back(makeCornellScene());
	sceneVect.push_back(makeTexturedCornellScene());
	sceneVect.push_back(makeSphereScene());
	sceneVect.push_back(makeLogoScene());
}

Scene& SceneCollection::getCurrent()
{
	return sceneVect.at(index);
}

void SceneCollection::next()
{
	if (index == sceneVect.size() - 1)
		index = 0;
	else
		index++;
}

void SceneCollection::prev()
{
	if (index == 0)
		index = sceneVect.size() - 1;
	else
		index--;
}
Scene SceneCollection::makeCornellScene()
{
	Camera::RenderSettings settings;

	settings.lightingEnabled = true;
	settings.texturesEnabled = true;
	settings.normalMapsEnabled = true;
	settings.lightPositionPreview = false;
	settings.mode = Camera::RenderSettings::Mode::Raster;

	Camera camera(settings, vec3(0, 0, 0));
	Camera camera2(settings, vec3(1, 1, 3));
	Scene scene(camera);
	Light light
	{
		.pos = {0.4, 0.4, 0.4},
		.colour={1.0, 1.0, 1.0, 1.0},
		.strength=0.5,
		.radius=0.05,
		.on=true 
	};
	Light light2
	{
		.pos = {-0.4, 0.4, 0.4},
		.colour = {1.0, 1.0, 1.0, 1.0},
		.strength = 0.5,
		.radius = 0.00,
		.on = true
	};

	camera.setPosition(vec3(0, 0, 4));
	camera.changeF(-1.0f);
	scene.addLight(light);
	scene.addLight(light2);
	scene.addObjectFile(std::make_unique<ObjectFile>("assets/obj/cornell-box.obj"));
	return scene;
}

Scene SceneCollection::makeTexturedCornellScene()
{
	Camera::RenderSettings settings;

	settings.lightingEnabled = true;
	settings.texturesEnabled = true;
	settings.normalMapsEnabled = true;
	settings.mode = Camera::RenderSettings::Mode::Raytracing;

	Camera camera(settings, vec3(0, 0, 7));
	Scene scene(camera);
	Light whiteLight
	{
		.pos=vec3(0, -0.6, 0.6),
		.strength=5
	};
	scene.getCamera().lookAt(&whiteLight.pos);
	scene.addLight(whiteLight);
	scene.addLight({ .pos = { -2.2, 2.2, 3 }, .colour = {1.0, 0.0, 0.0, 1.0}, .strength = 5 });
	scene.addLight({ .pos = { 2.2, -2.2, 3 }, .colour = {0.0, 1.0, 0.0, 1.0}, .strength = 5 });
	scene.addLight({ .pos = { -2.2, -2.2, 3 }, .colour = {0.0, 0.0, 1.0, 1.0}, .strength = 5 });
	scene.addObjectFile(std::make_unique<ObjectFile>(std::filesystem::path() / "assets" / "obj" / "textured-cornell-box.obj"));
	scene.addObjectFile(std::make_unique<ObjectFile>(std::filesystem::path() / "assets" / "obj" / "mirrorsphere.obj"));
	return scene;
}

Scene SceneCollection::makeSphereScene()
{
	Camera::RenderSettings settings;
	settings.lightingEnabled = true;
	settings.mode = Camera::RenderSettings::Mode::Raytracing;
	Scene scene(Camera(settings, vec3(0.05, 0.05, 3)));
	Light whiteLight{
		.pos=vec3(0.4, 0.4, 1.5),
		.strength=5,
	};
	scene.addLight(whiteLight);
	scene.addObjectFile(std::make_unique<ObjectFile>(std::filesystem::path() / "assets" / "obj" / "sphere.obj"));
	return scene;
}

Scene SceneCollection::makeLogoScene()
{
	Camera::RenderSettings settings;
	settings.lightingEnabled = true;
	settings.texturesEnabled = true;
	settings.lightPositionPreview = false;
	settings.normalMapsEnabled = true;
	settings.mode = Camera::RenderSettings::Mode::Raytracing;
	Camera camera(settings, vec3(0.0, 0.0, 5));
	Scene scene(camera);
	Light whiteLight{
		.pos = vec3(-1.5, 0, 0.9),
		.strength=7,
		.radius=0.1
	};
	Light red{ 
		.pos=vec3(1, 0, 2), 
		.colour = Colour{1.0, 0.0, 0.0, 1.0},
		.strength=2
	};
	Light blue
	{
		.pos = vec3(-1, 0, 2),
		.colour = Colour{ 0.0, 1.0, 0.0, 0.0 },
		.strength = 2
	};
	scene.addLight(whiteLight);
	scene.addLight(red);
	scene.addLight(blue);
	scene.addObjectFile(std::make_unique<ObjectFile>(std::filesystem::path() / "assets" / "obj" / "logo.obj"));
	scene.addObjectFile(std::make_unique<ObjectFile>(std::filesystem::path() / "assets" / "obj" / "submission-box.obj"));
	scene.toggleAnimation();
	return scene;
}