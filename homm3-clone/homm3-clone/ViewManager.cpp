#include "ViewManager.h"
#include "GameLogic.h"
#include "CombatLogic.h"
#include "DebugParameters.h"
#include "MOCastle.h"
#include "UICreatureContainerDouble.h"
#include "TopMenu.h"

ViewManager::ViewManager() {
	memset(isActive, 0, sizeof isActive);

	mapCamera = Camera(INIT_MAP_CAMERA);
	topMenuCamera = Camera(INIT_TOPMENU_CAMERA, vec3(0.6, 0.3, 0.3));
	rightMenuCamera = Camera(INIT_RIGHTPANEL_CAMERA, vec3(0.8, 0.8, 0.8));
	minimapCamera = Camera(INIT_MINIMAP_CAMERA, vec3(0, 0, 0));
	modelCamera = Camera(INIT_MODEL_CAMERA, vec3(1, 1, 1));
	promptCamera = Camera(INIT_PROMPT_CAMERA, vec3(0.3, 0.3, 0.3));
	combatCamera = Camera(INIT_COMBAT_CAMERA, vec3(0, 0, 0));
	buildingCamera = Camera(INIT_BUILDING_CAMERA, vec3(0, 0, 0));
	totalCamera = Camera(INIT_TOTAL_CAMERA, vec3(0, 0, 0));

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

	mapCamera.setFov(0.05);

	mapUnit = 0.2f;
	combatUnit = 0.05f;
	selectedMapTile = intp(-1, -1);

	/*
	// set up the menu buttons
	menuPositions.clear();
	menuButtons.clear();
	UIButton* button = new UIButton(vec2(70, 20), std::string("Quit"), &GameLogic::quitGame);
	UIButton* button2 = new UIButton(vec2(70, 20), std::string("End Turn"), &GameLogic::endTurn);
	menuButtons.push_back(button);
	menuButtons.push_back(button2);
	menuPositions.push_back(vec2(40, 5));
	menuPositions.push_back(vec2(130, 5));
	*/
	topMenuButtons = new TopMenu();
}

void ViewManager::displayMapWindow() {
	mapCamera.activateView();

	glPushMatrix();

	GameLogic &gameLogic = GameLogic::instance();
	Player* currPlayer = gameLogic.getCurrentPlayer();

	// TEMP draw the background
	if (debugShowBackground) {
		glPushMatrix();
		glTranslatef(0, 0, -0.001);
		displayTexture(backgroundTexture, mapUnit * gameLogic.colCount, mapUnit * gameLogic.rowCount, vec3(0.75, 0.75, 0.75));
		glPopMatrix();
	}
	else {
		glDisable(GL_LIGHTING);
		glColor3f(1, 1, 1);
		glRectf(0, 0, mapUnit * gameLogic.colCount, mapUnit * gameLogic.rowCount);
		glEnable(GL_LIGHTING);
	}

	glDisable(GL_LIGHTING);
	if (currPlayer->getCurrentHero() != nullptr && debugDistanceDisplay > 0) {
		std::vector<intp> reachable =
			currPlayer->pf.getReachableTiles(
				currPlayer->getCurrentHero()->hero->movementPoints,
				debugDistanceDisplay == 2
			);
		for (intp reachablePos : reachable) {
			glColor3f(0.5, 0.5, 1);
			glRectf(reachablePos.x * mapUnit, reachablePos.y * mapUnit,
				(reachablePos.x + 1) * mapUnit, (reachablePos.y + 1) * mapUnit);
		}
	}
	glEnable(GL_LIGHTING);

	// color the currently selected tile and the path to that tile (if one exists)
	glDisable(GL_LIGHTING);
	if (isMapTileSelected() || ~heroMoving) {
		glPushMatrix();
		if (gameLogic.getCurrentPlayer()->pf.distSolid[selectedMapTile.x][selectedMapTile.y] != -1) {
			glColor3f(0, 1, 0);

			MOHero* movingHero = gameLogic.getCurrentPlayer()->getCurrentHero();

			for (int i = 0; i < (int)movingPath.size(); i++) {
				// TODO modify costs? see displayHeroes
				if (!movingHero->hero->canMove((i + 1))) {
					glColor3f(1, 0, 0);
				}
				int currX = movingPath[i].x;
				int currY = movingPath[i].y;

				glRectf(currX * mapUnit, currY * mapUnit,
					(currX + 1) * mapUnit, (currY + 1) * mapUnit);
			}
		}
		else {
			glColor3f(1, 0, 0);
		}

		glRectf(selectedMapTile.x * mapUnit, selectedMapTile.y * mapUnit,
			(selectedMapTile.x + 1) * mapUnit, (selectedMapTile.y + 1) * mapUnit);
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);

	// draw the map elements
	Pathfinder &pf =
		(debugWhoseFogToShow == -1) ?
		gameLogic.getCurrentPlayer()->pf :
		gameLogic.getPlayerByIndex(debugWhoseFogToShow)->pf;

	float dimThird = mapUnit / 3;
	for (int i = 0; i < gameLogic.colCount; i++) {
		for (int j = 0; j < gameLogic.rowCount; j++) {
			if (pf.tileVisible[i][j] || !debugShowFogOfWar) {
				// TEMP change this asap
				glPushMatrix();
				glTranslatef(i * mapUnit + mapUnit / 2, j * mapUnit + mapUnit / 2, mapUnit * 2 / 5);
				gameLogic.map->getObject(intp(i, j))->draw(mapUnit);
				glPopMatrix();
			}
			else {
				// TODO modify to show accurately when models are on the fog border
				glDisable(GL_LIGHTING);
				glColor3f(0.3, 0.3, 0.3);
				glRectf(i * mapUnit + dimThird, j * mapUnit + dimThird,
					i* mapUnit + 2 * dimThird, j * mapUnit + 2 * dimThird);
				for (int k = 0; k < 8; k++) {
					int ni = i + COMBAT_MOVE_DIR[k].x;
					int nj = j + COMBAT_MOVE_DIR[k].y;
					bool dispFog = false;
					if (ni < 0 || nj < 0 || ni >= gameLogic.colCount || nj >= gameLogic.rowCount) {
						dispFog = true;
					}
					else if ((!pf.tileVisible[ni][nj] && k < 4) ||
						(k >= 4 && !pf.tileVisible[ni][nj] && !pf.tileVisible[i][nj] && !pf.tileVisible[ni][j])) {
						dispFog = true;
					}
					if (dispFog) {
						glRectf(
							i * mapUnit + dimThird + COMBAT_MOVE_DIR[k].x * dimThird,
							j * mapUnit + dimThird + COMBAT_MOVE_DIR[k].y * dimThird,
							i * mapUnit + 2 * dimThird + COMBAT_MOVE_DIR[k].x * dimThird,
							j * mapUnit + 2 * dimThird + COMBAT_MOVE_DIR[k].y * dimThird);
					}
				}

				glEnable(GL_LIGHTING);
			}
		}
	}

	// draw the map grid
	glDisable(GL_LIGHTING);
	if (debugShowGrid) {
		glPushMatrix();
		glColor3f(0, 0, 0);
		for (int i = 0; i <= gameLogic.rowCount; i++) {
			glBegin(GL_LINES);
			glVertex3f(0, i * mapUnit, 0);
			glVertex3f(gameLogic.colCount * mapUnit, i * mapUnit, 0);
			glEnd();
		}
		for (int j = 0; j <= gameLogic.colCount; j++) {
			glBegin(GL_LINES);
			glVertex3f(j * mapUnit, 0, 0);
			glVertex3f(j * mapUnit, gameLogic.rowCount * mapUnit, 0);
			glEnd();
		}
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);

	// write some map coordinates every here and there
	if (debugShowCoordinates) {
		glPushMatrix();
		for (int i = 0; i < gameLogic.rowCount; i += 5) {
			for (int j = 0; j < gameLogic.colCount; j += 5) {
				glRasterPos2f(i * mapUnit + mapUnit / 7, j * mapUnit + mapUnit / 7);
				drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(i) + ", " + std::to_string(j));
			}
		}
		glPopMatrix();
	}

	// draw the heroes
	// TODO move movement to a more appropriate location
	for (int i = 0; i < (int)gameLogic.players.size(); i++) {
		Player* player = gameLogic.players[i];
		for (int j = 0; j < (int)player->heroObjects.size(); j++) {
			MOHero* heroObject = player->heroObjects[j];
			vec2 currentCoords = vec2(heroObject->pos.x + 0.5f, heroObject->pos.y + 0.5f) * mapUnit;

			if (gameLogic.currentPlayer == i && heroMoving == j) {
				// CHECK 5 - 10 == -5  ok?
				if (!heroObject->hero->canMove(1)) {
					movingPath.clear();
				}
				if (!movingPath.empty()) {
					currentCoords = lerp(currentCoords, coordsFromMap(movingPath[0], mapUnit), movingProgress);
					if (movingProgress >= 1) {
						movingProgress = 0;
						heroObject->pos = movingPath[0];
						movingPath.erase(movingPath.begin());

						// TODO add different movement and costs?
						heroObject->hero->move(1);
						player->pf.makeVisibleAround(heroObject->pos, 5);
					}
				}
				else {
					// TODO refactor animation
					heroMoving = -1;
					selectedMapTile = intp(-1, -1);
					player->pf.findPaths(player->getCurrentHero()->pos);

					// TODO add tile activation stuff
					gameLogic.interact(heroObject);
				}
			}

			// draw the hero model
			glPushMatrix();
			glTranslatef(currentCoords.x, currentCoords.y, 0);
			vec3 tempColor = COLORS[player->getFactionId()];
			glColor3f(tempColor.r, tempColor.g, tempColor.b);
			heroObject->draw(mapUnit);
			glPopMatrix();
		}
	}

	glPopMatrix();
}

void ViewManager::displayModelPreview() {
	modelCamera.activateView();

	float testModelDim = 1.f;
	float testRotPerc = 0;

	glPushMatrix();

	testRotPerc += 0.001f;
	if (testRotPerc >= 1) {
		testRotPerc = 0;
	}
	glRotatef(-80, 1, 0, 0);
	glRotatef(testRotPerc * 360, 0, 0, 1);

	GameLogic &gameLogic = GameLogic::instance();
	if (isMapTileSelected()) {
		gameLogic.map->getObject(selectedMapTile)->draw(testModelDim, false);
	}
	else if (gameLogic.getCurrentPlayer() != nullptr
		&& gameLogic.getCurrentPlayer()->getCurrentHero() != nullptr) {
		vec3 factionColor = COLORS[gameLogic.getCurrentPlayer()->getFactionId()];
		glColor3f(factionColor.r, factionColor.g, factionColor.b);
		gameLogic.getCurrentPlayer()->getCurrentHero()->draw(testModelDim);
	}

	glPopMatrix();
}

void ViewManager::displayMinimap() {
	minimapCamera.activateView(Camera::ORTHO2D);

	GameLogic &gameLogic = GameLogic::instance();

	// draw the map grid
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.2);
	for (int i = 0; i <= gameLogic.rowCount; i++) {
		glBegin(GL_LINES);
		glVertex3f(0, i * minimapCamera.unit, 0);
		glVertex3f(gameLogic.colCount * minimapCamera.unit, i * minimapCamera.unit, 0);
		glEnd();
	}
	for (int j = 0; j <= gameLogic.colCount; j++) {
		glBegin(GL_LINES);
		glVertex3f(j * minimapCamera.unit, 0, 0);
		glVertex3f(j * minimapCamera.unit, gameLogic.rowCount * minimapCamera.unit, 0);
		glEnd();
	}
	glPopMatrix();

	// draw the simplified heroes
	for (int i = 0; i < (int)gameLogic.players.size(); i++) {
		glColor3f(COLORS[i].x, COLORS[i].y, COLORS[i].z);
		for (int j = 0; j < (int)gameLogic.players[i]->heroObjects.size(); j++) {
			intp heroPos = gameLogic.players[i]->heroObjects[j]->pos;
			glBegin(GL_TRIANGLES);
			glVertex2f((heroPos.x + 0.8f) * minimapCamera.unit, (heroPos.y + 0.9f) * minimapCamera.unit);
			glVertex2f((heroPos.x + 0.2f) * minimapCamera.unit, (heroPos.y + 0.9f) * minimapCamera.unit);
			glVertex2f((heroPos.x + 0.5f) * minimapCamera.unit, (heroPos.y + 0.1f) * minimapCamera.unit);
			glEnd();
		}
	}

	// draw walls
	Pathfinder &pf =
		(debugWhoseFogToShow == -1) ?
		gameLogic.getCurrentPlayer()->pf :
		gameLogic.getPlayerByIndex(debugWhoseFogToShow)->pf;
	for (int i = 0; i < gameLogic.rowCount; i++) {
		for (int j = 0; j < gameLogic.colCount; j++) {
			if (pf.tileVisible[i][j] || !debugShowFogOfWar) {
				if (gameLogic.map->getObject(vec2(i, j))->isBlocking()) {
					glPushMatrix();
					glColor3f(0.8, 0.8, 0.8);
					glRectf(i * minimapCamera.unit, j * minimapCamera.unit,
						(i + 1) * minimapCamera.unit, (j + 1) * minimapCamera.unit);
					glPopMatrix();
				}
			}
		}
	}

	// draw the area visible in the main view
	glPushMatrix();
	// TODO remove duplicate code for SW - NE calculation
	vec3 pointSW = mapCamera.viewToWorldPoint(windowWidth * INIT_MAP_CAMERA[0], windowHeight * INIT_MAP_CAMERA[1]);
	vec3 pointNE = mapCamera.viewToWorldPoint(windowWidth * INIT_MAP_CAMERA[2], windowHeight * INIT_MAP_CAMERA[3]);
	//
	glColor3f(0, 0.9, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(pointSW.x / mapUnit * minimapCamera.unit,
		pointSW.y / mapUnit * minimapCamera.unit);
	glVertex2f(pointNE.x / mapUnit * minimapCamera.unit,
		pointSW.y / mapUnit * minimapCamera.unit);
	glVertex2f(pointNE.x / mapUnit * minimapCamera.unit,
		pointNE.y / mapUnit * minimapCamera.unit);
	glVertex2f(pointSW.x / mapUnit * minimapCamera.unit,
		pointNE.y / mapUnit * minimapCamera.unit);
	glEnd();
	glPopMatrix();
}

void ViewManager::displayMenuWindow() {
	topMenuCamera.activateView(Camera::ORTHO2D);

	topMenuButtons->draw(
		intp(0, 0),
		intp(topMenuCamera.getViewport()[2], topMenuCamera.getViewport()[3])
	);

	/*for (int i = 0; i < (int)menuPositions.size(); i++) {
		glPushMatrix();
		glColor3f(0.8, 0.8, 0.8);
		glTranslatef(menuPositions[i].x, menuPositions[i].y, 0);
		menuButtons[i]->draw();
		glPopMatrix();
	}*/

	// TODO make ui text classes etc.

	glPushMatrix();
	Currency curr = GameLogic::instance().getCurrentPlayer()->wallet;
	std::string resourceText =
		"Gold: " + std::to_string(curr[GOLD]) + "     Wood: " + std::to_string(curr[WOOD]) +
		"     Ore: " + std::to_string(curr[ORE]) + "     Crystal: " + std::to_string(curr[CRYSTAL]);
	glColor3f(0, 0, 0);
	glRasterPos2d(500, 15);
	glDisable(GL_DEPTH_TEST);
	drawText(GLUT_BITMAP_HELVETICA_12, resourceText);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void ViewManager::displayControlledTroops() {
	GameLogic &gameLogic = GameLogic::instance();

	Hero* creatureContainer = nullptr;
	if (isMapTileSelected()) {
		creatureContainer = gameLogic.map->getObject(selectedMapTile)->hero;
		if (creatureContainer == nullptr && gameLogic.getHeroAt(selectedMapTile) != nullptr) {
			creatureContainer = gameLogic.getHeroAt(selectedMapTile)->hero;
		}
	}
	if (creatureContainer == nullptr && gameLogic.getCurrentPlayer()->getCurrentHero() != nullptr) {
		creatureContainer = gameLogic.getCurrentPlayer()->getCurrentHero()->hero;
	}
	if (creatureContainer != nullptr) {
		for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
			Creature* creature = creatureContainer->creatures[i];
			troopCamera[i].activateView();
			if (creature != nullptr && creature->count > 0) {

				// display the creature model
				glPushMatrix();
				glTranslatef(0, -0.2, 0);
				glRotatef(-80, 1, 0, 0);
				int tempFaction = creature->getFactionId();
				glColor3f(COLORS[tempFaction].r, COLORS[tempFaction].g, COLORS[tempFaction].b);
				creature->draw(1);
				glPopMatrix();

				// display the creature information
				troopCamera[i].activateView(Camera::ORTHO2D, false);
				//glRasterPos2f((INIT_TROOPS_CAMERAS[i][2] - INIT_TROOPS_CAMERAS[i][0]) / 2 * windowWidth, 25);
				glRasterPos2f(25, 25);
				drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(creature->count));
				glRasterPos2f(15, 10);
				if (creature->count == 1) {
					drawText(GLUT_BITMAP_HELVETICA_12, creature->name);
				}
				else {
					drawText(GLUT_BITMAP_HELVETICA_12, creature->plural);
				}
			}
		}
	}
}

void ViewManager::displayCombat() {
	if (!isActive[CamId::COMBAT]) {
		return;
	}

	CombatLogic &combatLogic = CombatLogic::instance();

	// display the 3D models etc.
	combatCamera.activateView(Camera::NORMAL);
	glPushMatrix();
	{
		// TEMP draw the background
		glPushMatrix();
		glColor3f(1, 1, 1);
		glTranslatef(-combatUnit * COMBAT_COLS, -combatUnit * COMBAT_ROWS, -0.001);
		displayTexture(combatBackgroundTexture, combatUnit * COMBAT_COLS * 3, combatUnit * COMBAT_ROWS * 3);
		glPopMatrix();

		// highlight the available moves
		if (!combatLogic.isAiActive()) {
			glDisable(GL_LIGHTING);
			glColor3f(0.2, 0.2, 0.2);
			for (intp validMove : combatLogic.validMoves) {
				glRectf(validMove.x * combatUnit, validMove.y * combatUnit,
					(validMove.x + 1) * combatUnit, (validMove.y + 1) * combatUnit);
			}
			glEnable(GL_LIGHTING);
		}

		// highlight the active creature
		glDisable(GL_LIGHTING);
		glColor3f(0, 1, 0);
		intp activePos = combatLogic.getActiveCreature()->combatPos;
		glRectf(activePos.x * combatUnit, activePos.y * combatUnit,
			(activePos.x + 1) * combatUnit, (activePos.y + 1) * combatUnit);
		glEnable(GL_LIGHTING);

		// draw the combat grid
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glColor3f(1, 1, 1);
		for (int i = 0; i <= COMBAT_ROWS; i++) {
			glBegin(GL_LINES);
			glVertex3f(0, i * combatUnit, 0);
			glVertex3f(COMBAT_COLS * combatUnit, i * combatUnit, 0);
			glEnd();
		}
		for (int j = 0; j <= COMBAT_COLS; j++) {
			glBegin(GL_LINES);
			glVertex3f(j * combatUnit, 0, 0);
			glVertex3f(j * combatUnit, COMBAT_ROWS * combatUnit, 0);
			glEnd();
		}
		glPopMatrix();
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		// draw the creatures
		for (Creature* creature : combatLogic.creatures) {
			if (creature != nullptr && creature->count > 0) {
				glPushMatrix();
				glTranslatef(creature->combatPos.x * combatUnit + combatUnit / 2,
					creature->combatPos.y * combatUnit + combatUnit / 2, 0);
				creature->draw(combatUnit);
				glPopMatrix();
			}
		}
	}
	glPopMatrix();

	// display the creature counts and other 2D info
	combatCamera.activateView(Camera::ORTHO2D, false);
	// TODO see Camera.h worldToViewPoint() (precalculate!!)
	std::vector<Creature*> safeCopy = combatLogic.creatures;
	for (Creature* creature : safeCopy) {
		if (creature != nullptr && creature->count > 0) {
			vec3 worldPoint = vec3(creature->combatPos.x * combatUnit, creature->combatPos.y * combatUnit, 0);
			// the stack count position depends on whether the stack is of the left/right faction
			if (creature->getFactionId() == combatLogic.attackerFaction) {
				worldPoint.x += combatUnit;
			}
			else {
				worldPoint.x -= combatUnit / 4;
				worldPoint.y += combatUnit * 0.667f;
			}
			vec2 screenPos = combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL);
			if (creature->getFactionId() == combatLogic.attackerFaction) {
				screenPos.x += 5;
				screenPos.y += 5;
			}
			else {
				screenPos.x -= 5;
				screenPos.y -= 5;
			}
			glColor3f(1, 1, 1);
			glRectf(screenPos.x, screenPos.y, screenPos.x + 30, screenPos.y + 15);
			glColor3f(0, 0, 0);
			glRasterPos2d(screenPos.x + 10, screenPos.y + 5);
			drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(creature->count));
		}
	}
}

void ViewManager::update(int elapsedTime) {
	if (tooltip.duration > 0) {
		tooltip.duration -= elapsedTime;
	}
}

void ViewManager::displayTooltip() {
	tooltip.refCamera->activateView(Camera::ORTHO2D, false);

	glColor3f(0.4f, 0.4f, 0.4f);
	glRectf(tooltip.pos.x, tooltip.pos.y, tooltip.pos.x + tooltip.dim.x, tooltip.pos.y + tooltip.dim.y);
	glColor3f(0.7f, 0.7f, 0.7f);
	glRectf(tooltip.pos.x + 5, tooltip.pos.y + 5, tooltip.pos.x + tooltip.dim.x - 5, tooltip.pos.y + tooltip.dim.y - 5);
	glColor3f(0, 0, 0);
	int ypos = tooltip.pos.y - 30;
	for (std::string descLine : tooltip.description) {
		glRasterPos2d(tooltip.pos.x + 15, ypos + tooltip.dim.y);
		drawText(GLUT_BITMAP_HELVETICA_12, descLine);
		ypos -= 15;
		if (ypos <= 0) {
			break;
		}
	}
}

void ViewManager::showTooltip(intp pos, std::vector<std::string> description, Camera* refCamera, int milliseconds) {
	// calculate the required tooltip dimensions depending on the longest description line and number of lines
	tooltip.dim.x = 0;
	for (std::string descLine : description) {
		tooltip.dim.x = mmax(tooltip.dim.x, descLine.size() * 8);
	}
	tooltip.dim.y = description.size() * 15 + 30;

	// move the tooltip if needed so it doesn't display outside the viewport
	if (pos.x >= refCamera->getViewport()[2] / 2.f) {
		pos.x -= tooltip.dim.x;
	}
	if (pos.y >= refCamera->getViewport()[3] / 2.f) {
		pos.y -= tooltip.dim.y;
	}
	tooltip.pos = pos;

	tooltip.description = description;
	tooltip.refCamera = refCamera;
	tooltip.duration = milliseconds;
}

void ViewManager::displayBuilding() {
	if (!isActive[CamId::BUILDING]) {
		return;
	}

	MapObject* buildingObject = GameLogic::instance().map->getObject(displayedBuilding);
	if (buildingObject == nullptr || buildingObject->objectType == MOType::EMPTY) {
		isActive[CamId::BUILDING] = false;
		return;
	}

	buildingCamera.activateView(Camera::ORTHO2D);
	if (buildingObject->objectType == MOType::CASTLE) {
		MOCastle* castleObject = (MOCastle*)buildingObject;

		// TEMP a brutish approach to castle UI

		// building slots display
		int toDisplay = 0;
		for (int i = 0; i < BuildingType::_BUILDINGTYPE_END; i++) {
			if (!castleObject->hasBuilding(static_cast<BuildingType>(i))) {
				toDisplay++;
			}
		}

		const int columns = 4;
		const int rows = (toDisplay - 1) / columns + 1;
		const int slotWidth = ((INIT_BUILDING_CAMERA[2] - INIT_BUILDING_CAMERA[0]) * windowWidth - 20) / columns;
		const int slotHeight = ((INIT_BUILDING_CAMERA[3] - INIT_BUILDING_CAMERA[1]) * windowHeight - 20) * 2 / 3 / rows;
		
		buildingSlots.clear();
		for (int i = 0; i < BuildingType::_BUILDINGTYPE_END; i++) {
			vec2 screenPos(
				10 + buildingSlots.size() % columns * slotWidth,
				(INIT_BUILDING_CAMERA[3] - INIT_BUILDING_CAMERA[1]) * windowHeight - 10 - (buildingSlots.size() / columns + 1) * slotHeight
			);


			buildingSlots.push_back(BuildingSlot(screenPos, glm::vec2(slotWidth, slotHeight), i));

			BuildingType type = static_cast<BuildingType>(i);

			if (castleObject->hasBuilding(type)) {
				glColor3f(0.5f, 0.5f, 0.9f);
			}
			else if (castleObject->canBuild(type)) {
				glColor3f(0.4f, 0.9f, 0.4f);
			}
			else {
				glColor3f(0.7f, 0.7f, 0.7f);
			}

			glRectf(screenPos.x, screenPos.y, screenPos.x + slotWidth, screenPos.y + slotHeight);

			if (castleObject->hasBuilding(type)) {
				castleObject->getBuilding(type)->draw(screenPos, intp(slotWidth, slotHeight));
				continue;
			}

			glColor3f(0, 0, 0);
			glRasterPos2d(screenPos.x + 10, screenPos.y + 10);
			drawText(GLUT_BITMAP_HELVETICA_12, Buildings::toString(type));
		}

		// troop display (castle and visiting hero)
		troopSlotsPos = intp(10, 10);
		troopSlotsDim = intp((INIT_BUILDING_CAMERA[2] - INIT_BUILDING_CAMERA[0]) * windowWidth - 20,
			((INIT_BUILDING_CAMERA[3] - INIT_BUILDING_CAMERA[1]) * windowHeight - 20) / 3);
		troopSlots->draw(troopSlotsPos, troopSlotsDim);
	}
	else {
		isActive[BUILDING] = false;
		return;
	}
}

inline bool ViewManager::isMapTileSelected() {
	return selectedMapTile.x >= 0 && selectedMapTile.y >= 0;
}

void ViewManager::mapCameraLookAt(intp objectPos){
	mapCamera.pos = vec3(objectPos.x * mapUnit, objectPos.y * mapUnit, mapCamera.pos.z);
}

void ViewManager::displayActiveWindows() {
	displayMapWindow();
	displayModelPreview();
	displayMinimap();
	displayMenuWindow();
	displayControlledTroops();
	displayCombat();
	displayBuilding();
	if (tooltip.duration > 0) {
		displayTooltip();
	}
}

void ViewManager::displayTexture(GLuint id, float width, float height, vec3 colorMult) {
	glColor3f(colorMult.r, colorMult.g, colorMult.b);

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, id);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(width, 0, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(width, height, 0.0);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0, height, 0.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

void ViewManager::showBuilding(intp location) {
	displayedBuilding = location;
	MapObject* object = GameLogic::instance().map->getObject(location);
	if (object == nullptr) {
		return;
	}
	if (object->objectType == MOType::CASTLE) {
		delete troopSlots;
		MOHero* visitingHero = GameLogic::instance().getHeroAt(location);
		if (visitingHero != nullptr) {
			troopSlots = new UICreatureContainerDouble(object->hero, GameLogic::instance().getHeroAt(location)->hero);
		}
		else {
			troopSlots = new UICreatureContainerDouble(object->hero, nullptr);
		}
	}
	isActive[CamId::BUILDING] = true;
}

ViewManager& ViewManager::instance() {
	static ViewManager VIEWMANAGER_INSTANCE;

	return VIEWMANAGER_INSTANCE;
}

BuildingSlot::BuildingSlot(glm::vec2 _pos, glm::vec2 _dim, int _type)
	: pos(_pos), dim(_dim), type(_type){
}

Tooltip::Tooltip() : duration(0){
	description.clear();
	refCamera = nullptr;
}
