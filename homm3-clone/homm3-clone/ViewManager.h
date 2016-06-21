#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Constants.h"
#include "Camera.h"
#include "UIButton.h"
#include "Creature.h"
#include "UIClickableContainer.h"

enum CamId {
	CAM_NULL, MAP, TOPMENU, MINIMAP, MODEL, TROOPS, COMBAT, BUILDING, _CAMID_END
};

struct BuildingSlot {
	BuildingSlot(glm::vec2 _pos, glm::vec2 _dim, int _type);

	glm::vec2 pos;
	glm::vec2 dim;
	int type;
};

class ViewManager {

public:
	void mapCameraLookAt(intp objectPos);

	void displayActiveWindows();
	void displayTexture(GLuint id, float width, float height, vec3 colorMult = vec3(1, 1, 1));
	void showBuilding(intp location);

	bool isActive[_CAMID_END];
	Camera mapCamera;
	Camera topMenuCamera;
	Camera rightMenuCamera;
	Camera minimapCamera;
	Camera modelCamera;
	Camera troopCamera[HERO_UNIT_SLOTS];
	Camera promptCamera;
	Camera combatCamera;
	Camera buildingCamera;

	float INIT_MAP_CAMERA[4] = { 0, 0, 8.f / 10, 29.f / 30 };
	float INIT_TOPMENU_CAMERA[4] = { 0, 29.f / 30, 8.f / 10, 1 };
	float INIT_RIGHTPANEL_CAMERA[4] = { 8.f / 10, 1.f / 3, 1, 2.f / 3 };
	float INIT_MINIMAP_CAMERA[4] = { 8.f / 10, 0, 1, 1.f / 3 };
	// TODO recalculate minimap offset on resize so it fits perfectly
	float INIT_MODEL_CAMERA[4] = { 8.f / 10, 2.f / 3, 1, 1 };
	float INIT_TROOPS_CAMERAS[HERO_UNIT_SLOTS][4];
	float INIT_PROMPT_CAMERA[4] = { 3.f / 10, 9.f / 20, 5.f / 10, 11.f / 20 };
	float INIT_COMBAT_CAMERA[4] = { 0.25f, 0.25f, 0.75f, 0.75f };
	float INIT_BUILDING_CAMERA[4] = { 0.25f, 0.25f, 0.75f, 0.75f };

	float mapUnit;
	float combatUnit;

	int windowWidth;
	int windowHeight;

	// specific camera data
	GLuint combatBackgroundTexture;
	GLuint backgroundTexture;
	intp selectedMapTile;
	std::vector<UIButton*> menuButtons;
	std::vector<glm::vec2> menuPositions;
	Creature* combatTooltipTarget;
	intp displayedBuilding;
	std::vector<BuildingSlot> buildingSlots;
	UIClickableContainer* troopSlots;
	// TEMP
	intp troopSlotsPos;
	intp troopSlotsDim;

	// TEMP hero movement (TODO refactor ofc)
	int heroMoving = -1;
	std::vector<intp> movingPath;
	float movingProgress = 0;
	float movingSpeed = 0.03f;
	//

	// singleton things
	static ViewManager& instance();
	ViewManager(ViewManager const&) = delete;
	void operator = (ViewManager const &) = delete;

private:
	// singleton things
	ViewManager();

	void displayMapWindow();
	void displayModelPreview();
	void displayMinimap();
	void displayMenuWindow();
	void displayControlledTroops();
	void displayCombat();
	void displayBuilding();

	inline bool isMapTileSelected();
};
