#include "ViewManager.h"

ViewManager::ViewManager() {
	mapCamera = Camera(INIT_MAP_CAMERA);
	topMenuCamera = Camera(INIT_TOPMENU_CAMERA, vec3(0.6, 0.3, 0.3));
	rightMenuCamera = Camera(INIT_RIGHTPANEL_CAMERA, vec3(0.8, 0.8, 0.8));
	minimapCamera = Camera(INIT_MINIMAP_CAMERA, vec3(0, 0, 0));
	modelCamera = Camera(INIT_MODEL_CAMERA, vec3(1, 1, 1));
	promptCamera = Camera(INIT_PROMPT_CAMERA, vec3(0.3, 0.3, 0.3));
	combatCamera = Camera(INIT_COMBAT_CAMERA, vec3(0, 0, 0));

	int unitSlotsPerRow = 3;
	float offsetX = (INIT_RIGHTPANEL_CAMERA[2] - INIT_RIGHTPANEL_CAMERA[0]) / unitSlotsPerRow;
	float offsetY = (INIT_RIGHTPANEL_CAMERA[1] - INIT_RIGHTPANEL_CAMERA[3]) / HERO_UNIT_SLOTS * unitSlotsPerRow;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		INIT_TROOPS_CAMERAS[i][0] = INIT_RIGHTPANEL_CAMERA[0] + offsetX * (i % unitSlotsPerRow);
		INIT_TROOPS_CAMERAS[i][1] = INIT_RIGHTPANEL_CAMERA[3] + offsetY * (i / unitSlotsPerRow + 1);
		INIT_TROOPS_CAMERAS[i][2] = INIT_RIGHTPANEL_CAMERA[0] + offsetX * (i % unitSlotsPerRow + 1);
		INIT_TROOPS_CAMERAS[i][3] = INIT_RIGHTPANEL_CAMERA[3] + offsetY * (i / unitSlotsPerRow);

		troopCamera[i] = Camera(INIT_TROOPS_CAMERAS[i], vec3(0, 0, 0));
	}

	combatCamera.addRotation(vec4(-45, 1, 0, 0));
	combatCamera.setFov(0.05);

	//mapCamera.addRotation(vec4(-30, 1, 0, 0));
	mapCamera.setFov(0.05);

	mapUnit = 0.2f;
}

void ViewManager::showPrompt(string _promptText, int miliDuration) {
	isPromptUp = true;
	promptText = _promptText;

	testPromptTimer = miliDuration;
}

void ViewManager::hidePrompt() {
	isPromptUp = false;
	promptText = "";
	testPromptTimer = 0;
}

void ViewManager::mapCameraLookAt(intp objectPos){
	mapCamera.pos = vec3(objectPos.x * mapUnit, objectPos.y * mapUnit, mapCamera.pos.z);
}

ViewManager& ViewManager::instance() {
	static ViewManager VIEWMANAGER_INSTANCE;

	return VIEWMANAGER_INSTANCE;
}
