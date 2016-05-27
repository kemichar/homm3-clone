#pragma once

#include <string>
#include "Constants.h"
#include "Camera.h"

using namespace std;

class ViewManager {

public:
	static void showPrompt(string _promptText, int miliDuration = 3000);
	static void hidePrompt();
	void mapCameraLookAt(intp objectPos);

	static string promptText;
	static bool isPromptUp;
	static float testPromptTimer;

	Camera mapCamera;
	Camera topMenuCamera;
	Camera rightMenuCamera;
	Camera minimapCamera;
	Camera modelCamera;
	Camera troopCamera[HERO_UNIT_SLOTS];
	Camera promptCamera;
	Camera combatCamera;

	float INIT_MAP_CAMERA[4] = { 0, 0, 8.f / 10, 29.f / 30 };
	float INIT_TOPMENU_CAMERA[4] = { 0, 29.f / 30, 8.f / 10, 1 };
	float INIT_RIGHTPANEL_CAMERA[4] = { 8.f / 10, 1.f / 3, 1, 2.f / 3 };
	float INIT_MINIMAP_CAMERA[4] = { 8.f / 10, 0, 1, 1.f / 3 };
	// TODO recalculate minimap offset on resize so it fits perfectly
	float INIT_MODEL_CAMERA[4] = { 8.f / 10, 2.f / 3, 1, 1 };
	float INIT_TROOPS_CAMERAS[HERO_UNIT_SLOTS][4];
	float INIT_PROMPT_CAMERA[4] = { 3.f / 10, 9.f / 20, 5.f / 10, 11.f / 20 };
	float INIT_COMBAT_CAMERA[4] = { 0.25f, 0.25f, 0.75f, 0.75f };

	float mapUnit;

	// singleton things
	static ViewManager& instance();
	ViewManager(ViewManager const&) = delete;
	void operator = (ViewManager const &) = delete;

private:
	ViewManager();
};
