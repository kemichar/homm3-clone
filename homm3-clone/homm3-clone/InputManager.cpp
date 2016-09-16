#include "InputManager.h"
#include "ViewManager.h"
#include "FactionSetup.h"
#include "CombatLogic.h"
#include "GameLogic.h"
#include "DebugParameters.h"
#include "MOCastle.h"

void InputManager::windowResized(int width, int height){
	windowWidth = width;
	windowHeight = height;
}

void InputManager::processClick(int button, int state, int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	ViewManager &viewManager = ViewManager::instance();

	if (viewManager.isActive[CamId::BUILDING]) {
		if (rectContains(vec2(viewManager.INIT_BUILDING_CAMERA[0], viewManager.INIT_BUILDING_CAMERA[1]),
			vec2(viewManager.INIT_BUILDING_CAMERA[2] - viewManager.INIT_BUILDING_CAMERA[0], viewManager.INIT_BUILDING_CAMERA[3] - viewManager.INIT_BUILDING_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			processBuildingClick(
				button,
				state,
				screenX - viewManager.buildingCamera.getViewport()[0],
				screenY - viewManager.buildingCamera.getViewport()[1]);
		}
		else if (state == GLUT_UP){
			viewManager.isActive[CamId::BUILDING] = false;
		}
	}
	else if (viewManager.isActive[CamId::COMBAT]) {
		if (rectContains(vec2(viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[1]),
			vec2(viewManager.INIT_COMBAT_CAMERA[2] - viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[3] - viewManager.INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			processCombatClick(
				button,
				state,
				screenX - viewManager.combatCamera.getViewport()[0],
				screenY - viewManager.combatCamera.getViewport()[1]);
		}
	}
	else if (rectContains(vec2(viewManager.INIT_MAP_CAMERA[0], viewManager.INIT_MAP_CAMERA[1]),
		vec2(viewManager.INIT_MAP_CAMERA[2] - viewManager.INIT_MAP_CAMERA[0], viewManager.INIT_MAP_CAMERA[3] - viewManager.INIT_MAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		processAdventureClick(
			button,
			state,
			screenX - viewManager.mapCamera.getViewport()[0],
			screenY - viewManager.mapCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2(viewManager.INIT_TOPMENU_CAMERA[2] - viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[3] - viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		processTopMenuClick(
			button,
			state,
			screenX - viewManager.topMenuCamera.getViewport()[0],
			screenY - viewManager.topMenuCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[1]),
		vec2(viewManager.INIT_MINIMAP_CAMERA[2] - viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[3] - viewManager.INIT_MINIMAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		processMinimapClick(
			button,
			state,
			screenX - viewManager.minimapCamera.getViewport()[0],
			screenY - viewManager.minimapCamera.getViewport()[1]);
	}
}

void InputManager::processHover(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	ViewManager &viewManager = ViewManager::instance();

	if (viewManager.isActive[CamId::COMBAT]) {
		if (rectContains(vec2(viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[1]),
			vec2(viewManager.INIT_COMBAT_CAMERA[2] - viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[3] - viewManager.INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			x = screenX - viewManager.combatCamera.getViewport()[0];
			y = screenY - viewManager.combatCamera.getViewport()[1];

			vec3 tempSelected = viewManager.combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

			vec2 tempTile =
				vec2(int(tempSelected.x / viewManager.combatUnit),
					int(tempSelected.y / viewManager.combatUnit));

			if (tempSelected.x < 0 || tempSelected.y < 0 ||
				tempSelected.x > COMBAT_COLS * viewManager.combatUnit ||
				tempSelected.y > COMBAT_ROWS * viewManager.combatUnit) {
				return;
			}

			CombatLogic &combatLogic = CombatLogic::instance();
			bool hoveredCreature = false;
			for (Creature* creature : combatLogic.creatures) {
				if (creature->combatPos == tempTile) {
					vec3 worldPoint = vec3(
						tempTile.x * viewManager.combatUnit + viewManager.combatUnit / 2,
						tempTile.y * viewManager.combatUnit + viewManager.combatUnit / 2,
						0
					);

					viewManager.showTooltip(
						viewManager.combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL),
						creature->getDescription(),
						&viewManager.combatCamera,
						1000
					);
					hoveredCreature = true;
					break;
				}
			}
			if (!hoveredCreature && (viewManager.tooltip.refCamera == &viewManager.combatCamera)) {
				viewManager.tooltip.duration = 0;
			}
		}
	}
	else if (rectContains(vec2(viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2(viewManager.INIT_TOPMENU_CAMERA[2] - viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[3] - viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		viewManager.topMenuButtons->processHover(
			(screenX - viewManager.topMenuCamera.getViewport()[0])
				/ (float)viewManager.topMenuCamera.getViewport()[2],
			(screenY - viewManager.topMenuCamera.getViewport()[1])
				/ (float)viewManager.topMenuCamera.getViewport()[3]
		);
	}
}

void InputManager::processDrag(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	ViewManager &viewManager = ViewManager::instance();

	if (minimapNavigation) {
		if (!rectContains(vec2(viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[1]),
			vec2(viewManager.INIT_MINIMAP_CAMERA[2] - viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[3] - viewManager.INIT_MINIMAP_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
			minimapNavigation = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		else {
			processMinimapDrag(screenX - viewManager.minimapCamera.getViewport()[0], screenY - viewManager.minimapCamera.getViewport()[1]);
		}
	}
}

void InputManager::processKeyDown(unsigned char key, int x, int y) {
	ViewManager &viewManager = ViewManager::instance();
	if (key == 'w') {
		viewManager.mapCamera.speed.y += 1;
	}
	else if (key == 's') {
		viewManager.mapCamera.speed.y -= 1;
	}
	else if (key == 'a') {
		viewManager.mapCamera.speed.x -= 1;
	}
	else if (key == 'd') {
		viewManager.mapCamera.speed.x += 1;
	}
	else if (key == 'q') {
		viewManager.mapCamera.speed.z -= 1;
	}
	else if (key == 'e') {
		viewManager.mapCamera.speed.z += 1;
	}
	else if (key == ' ') {
		MOHero* hero = GameLogic::instance().getCurrentPlayer()->getCurrentHero();
		if (hero != nullptr) {
			viewManager.mapCameraLookAt(hero->pos);
		}
	}
	else if (key == 27) {
		GameLogic::instance().quitGame();
	}
	else if (key == 'x') {
		debugTiltedMap = !debugTiltedMap;
		if (debugTiltedMap) {
			viewManager.mapCamera.addRotation(vec4(-30, 1, 0, 0));
		}
		else {
			viewManager.mapCamera.rotations.clear();
		}
	}
}

void InputManager::processKeyUp(unsigned char key, int x, int y) {
	GameLogic &gameLogic = GameLogic::instance();
	ViewManager &viewManager = ViewManager::instance();

	if (key == 'w') {
		viewManager.mapCamera.speed.y -= 1;
	}
	else if (key == 's') {
		viewManager.mapCamera.speed.y += 1;
	}
	else if (key == 'a') {
		viewManager.mapCamera.speed.x += 1;
	}
	else if (key == 'd') {
		viewManager.mapCamera.speed.x -= 1;
	}
	else if (key == 'q') {
		viewManager.mapCamera.speed.z += 1;
	}
	else if (key == 'e') {
		viewManager.mapCamera.speed.z -= 1;
	}
	else if (key >= '1' && key <= '9') {
		gameLogic.getCurrentPlayer()->setHeroByIndex(key - '1');
		viewManager.selectedMapTile = intp(-1, -1);
	}
	else if (key == 'c') {
		viewManager.showBuilding(gameLogic.getCurrentPlayer()->getSelectedCastle());
	}
	else if (key == '+') {
		if (!gameLogic.isAiActive()) {
			MOHero* heroObject = gameLogic.getCurrentPlayer()->getCurrentHero();
			if (heroObject != nullptr) {
				heroObject->hero->movementPoints += 5;
				gameLogic.getCurrentPlayer()->pf.findPaths(heroObject->pos);
				viewManager.selectedMapTile = intp(-1, -1);
			}
		}
	}
	else if (key == '0') {
		if (!gameLogic.isAiActive()) {
			gameLogic.getCurrentPlayer()->wallet[GOLD] += 500;
		}
	}
}

void InputManager::processKeySpecial(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT) {
	}
	else if (key == GLUT_KEY_RIGHT) {
	}
	else if (key == GLUT_KEY_DOWN) {
	}
	else if (key == GLUT_KEY_UP) {
	}
	else if (key == GLUT_KEY_F1) {
		debugBotsSlowed = !debugBotsSlowed;
		//paused = !paused;
	}
	else if (key == GLUT_KEY_F2) {
		GameLogic &gameLogic = GameLogic::instance();
		gameLogic.map->clearMap();
		//	gameLogic.map->fillMap();
		gameLogic.map->testFillMap(5);
	}
	else if (key == GLUT_KEY_F3) {
		debugCameraAutoFocus = !debugCameraAutoFocus;
	}
	else if (key == GLUT_KEY_F4) {
		debugShowCoordinates = !debugShowCoordinates;
	}
	else if (key == GLUT_KEY_F5) {
		debugShowGrid = !debugShowGrid;
	}
	else if (key == GLUT_KEY_F6) {
		debugShowBackground = !debugShowBackground;
	}
	else if (key == GLUT_KEY_F7) {
		debugShowFogOfWar = !debugShowFogOfWar;
	}
	else if (key == GLUT_KEY_F8) {
		debugWhoseFogToShow++;
		if (debugWhoseFogToShow == GameLogic::instance().players.size()) {
			debugWhoseFogToShow = -1; // will show the current player's fog
		}
	}
	else if (key == GLUT_KEY_F9) {
		debugDistanceDisplay = (debugDistanceDisplay + 1) % 3;
	}
	else if (key == GLUT_KEY_F10) {
		debugFreeCamera = !debugFreeCamera;
	}
	else if (key == GLUT_KEY_F11) {
	}
}

InputManager & InputManager::instance() {
	static InputManager INPUTMANAGER_INSTANCE;

	return INPUTMANAGER_INSTANCE;
}

void InputManager::processAdventureClick(int button, int state, int x, int y) {
	// assuming (0, 0) to be bottom left

	ViewManager &viewManager = ViewManager::instance();
	GameLogic &gameLogic = GameLogic::instance();

	// DEBUG hero teleportation
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		vec3 tempPoint = viewManager.mapCamera.viewToWorldPoint(x, y, Camera::NORMAL);

		intp tempTile = intp(int(tempPoint.x / viewManager.mapUnit), int(tempPoint.y / viewManager.mapUnit));

		if (tempPoint.x < 0 || tempPoint.y < 0 ||
			tempPoint.x > gameLogic.colCount * viewManager.mapUnit || tempPoint.y > gameLogic.rowCount * viewManager.mapUnit) {
			return;
		}
		MOHero* heroObject = GameLogic::instance().getCurrentPlayer()->getCurrentHero();
		if (heroObject != nullptr) {
			heroObject->pos = tempTile;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		if (~viewManager.heroMoving) {
			return; // we want to finish the current movement first
		}

		Player* currPlayer = gameLogic.getCurrentPlayer();

		MOHero* activeHero = currPlayer->getCurrentHero();
		if (FactionSetup::instance().isAI[currPlayer->getFactionId()] || activeHero == nullptr) {
			return;
		}

		vec3 selectedPoint = viewManager.mapCamera.viewToWorldPoint(x, y, Camera::NORMAL);

		// tile selection and player movement update
		intp tempTile = intp(int(selectedPoint.x / viewManager.mapUnit), int(selectedPoint.y / viewManager.mapUnit));

		if (selectedPoint.x < 0 || selectedPoint.y < 0 ||
			selectedPoint.x > gameLogic.colCount * viewManager.mapUnit || selectedPoint.y > gameLogic.rowCount * viewManager.mapUnit
			|| !currPlayer->pf.tileVisible[tempTile.x][tempTile.y]) {
			return;
		}

		// if the clicked tile is already selected the player wants to move there
		if (tempTile == viewManager.selectedMapTile) {
			viewManager.heroMoving = -1;
			if (currPlayer->pf.distSolid[tempTile.x][tempTile.y] != -1) {
				viewManager.movingProgress = 0;
				viewManager.heroMoving = currPlayer->getCurrentHeroIndex();
			}
		}
		// select the clicked tile and find the shortest path to that tile
		else {
			viewManager.selectedMapTile = tempTile;

			viewManager.movingPath = currPlayer->pf.getPathTo(tempTile);
			if (!viewManager.movingPath.empty()) {
				viewManager.movingPath.erase(viewManager.movingPath.begin());
			}
		}
	}
}

void InputManager::processTopMenuClick(int button, int state, int x, int y) {
	ViewManager &viewManager = ViewManager::instance();

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		viewManager.topMenuButtons->processClick(
			x / (float)(viewManager.topMenuCamera.getViewport()[2]),
			y / (float)(viewManager.topMenuCamera.getViewport()[3])
		);
		/*
		for (int i = 0; i < (int)viewManager.menuButtons.size(); i++) {
			if (rectContains(viewManager.menuPositions[i], viewManager.menuButtons[i]->getDimension(), vec2(x, y))) {
				viewManager.menuButtons[i]->activate();
				break;
			}
		}*/
	}
}

void InputManager::processMinimapClick(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		minimapNavigation = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		minimapNavigation = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void InputManager::processCombatClick(int button, int state, int x, int y) {
	ViewManager &viewManager = ViewManager::instance();
	CombatLogic &combatLogic = CombatLogic::instance();

	// assuming (0, 0) to be bottom left

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		// TODO if (~animating) { // we want to finish the current movement first

		vec3 tempSelected = viewManager.combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

		// tile selection and player movement update
		vec2 tempTile =
			vec2(int(tempSelected.x / viewManager.combatUnit),
				int(tempSelected.y / viewManager.combatUnit));

		if (tempSelected.x < 0 || tempSelected.y < 0 ||
			tempSelected.x > COMBAT_COLS * viewManager.combatUnit ||
			tempSelected.y > COMBAT_ROWS * viewManager.combatUnit) {
			return;
		}
		else {
			vec2 dirOffset = vec2(tempSelected.x, tempSelected.y) - tempTile * viewManager.combatUnit;
			int dirCode = 0;
			if (dirOffset.x > 0.25f * viewManager.combatUnit)
				dirCode += 3;
			if (dirOffset.x > 0.75 * viewManager.combatUnit)
				dirCode += 3;
			if (dirOffset.y > 0.25f * viewManager.combatUnit)
				dirCode += 1;
			if (dirOffset.y > 0.75 * viewManager.combatUnit)
				dirCode += 1;

			// try moving to the clicked tile
			if (!combatLogic.isAiActive()) {
				combatLogic.move(tempTile, dirCode);
			}
		}
	}
}

void InputManager::processBuildingClick(int button, int state, int x, int y) {
	ViewManager &viewManager = ViewManager::instance();

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (GameLogic::instance().isAiActive()) {
			return;
		}

		// TEMP brutish castle UI interaction
		MapObject* object = GameLogic::instance().map->getObject(viewManager.displayedBuilding);
		if (object->objectType == MOType::CASTLE) {
			MOCastle* castle = ((MOCastle*)object);
			for (int i = 0; i < (int)viewManager.buildingSlots.size(); i++) {
				BuildingType type = static_cast<BuildingType>(viewManager.buildingSlots[i].type);
				if (rectContains(viewManager.buildingSlots[i].pos, viewManager.buildingSlots[i].dim, vec2(x, y))) {
					if (!castle->hasBuilding(type)) {
						castle->build(type);
					}
					else {
						Buildings::Building* building = castle->getBuilding(type);
						building->processClick(
							(x - viewManager.buildingSlots[i].pos.x) / (float)viewManager.buildingSlots[i].dim.x,
							(y - viewManager.buildingSlots[i].pos.y) / (float)viewManager.buildingSlots[i].dim.y);
					}
					return;
				}
			}
			viewManager.troopSlots->processClick(
				(x - viewManager.troopSlotsPos.x) / (float)viewManager.troopSlotsDim.x,
				(y - viewManager.troopSlotsPos.y) / (float)viewManager.troopSlotsDim.y
			);
		}
	}
}

void InputManager::processMinimapDrag(int x, int y) {
	ViewManager &viewManager = ViewManager::instance();
	GameLogic &gameLogic = GameLogic::instance();

	if (minimapNavigation) {
		// TEMP until the UI minimap becomes square (todo)
		float minDim = mmin(viewManager.minimapCamera.getViewport()[2], viewManager.minimapCamera.getViewport()[3]);
		float xTrue = x / minDim, yTrue = y / minDim;

		// TEMP until we change test dimensions
		viewManager.mapCamera.pos =
			vec3(xTrue * gameLogic.colCount * viewManager.mapUnit,
				yTrue * gameLogic.rowCount * viewManager.mapUnit,
				viewManager.mapCamera.pos.z);
	}
}

InputManager::InputManager() {
}