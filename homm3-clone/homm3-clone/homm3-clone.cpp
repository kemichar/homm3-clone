// homm3-clone.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <cstdio>
#include <algorithm>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <queue>
#include <stack>
#include <cmath>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <stdlib.h>
#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/glew.h>
#include <GL/glut.h>
#include "objglm/objglm.h"
#include "Utility.h"
#include "Constants.h"
#include "Model.h"
#include "UIButton.h"
#include "Camera.h"
#include "Map.h"
#include "ViewManager.h"
#include "Player.h"
#include "GameLogic.h"
#include "CombatLogic.h"
#include "Resources.h"
#include "objglm/objglm.h"

using namespace std;
using namespace glm;

int windowWidth = INITIAL_WIDTH;
int windowHeight = INITIAL_HEIGHT;

// DEBUG
bool debugShowGrid = true;
bool debugShowCoordinates = true;
bool debugFreeCamera = true;
//

// CHECK
string ViewManager::promptText = "";
bool ViewManager::isPromptUp = false;
float ViewManager::testPromptTimer = 0;
//

// TEMP
vector<UIButton*> menuButtons;
vector<vec2> menuPositions;
void tempCreateButtons() {
	UIButton* button = new UIButton(vec2(70, 20), string("Quit"), &GameLogic::quitGame);
	UIButton* button2 = new UIButton(vec2(70, 20), string("End Turn"), &GameLogic::endTurn);
	menuButtons.push_back(button);
	menuButtons.push_back(button2);
	menuPositions.push_back(vec2(40, 5));
	menuPositions.push_back(vec2(130, 5));
}

bool minimapNavigation = false;
//

int frameCount;
int curr, last;
bool paused;

const int rowCount = 50, colCount = 50;
float testDim = 0.2f;
vec3 selectedPoint(0, 0, 0);
vec2 selectedTile(0, 0);
bool isTileSelected = false;

// hero movement (TODO refactor ofc)
int heroMoving = -1;
vector<vec2> movingPath;
float movingProgress = 0;
float movingSpeed = 0.03f;
//

GameLogic& gameLogic = GameLogic::instance();
CombatLogic* combatLogic = new CombatLogic();

// TODO refactor; Camera viewports etc. 
float INIT_MAP_CAMERA[4] = { 0, 0, 8.f / 10, 29.f / 30 };
Camera mapCamera(INIT_MAP_CAMERA);
float INIT_TOPMENU_CAMERA[4] = { 0, 29.f / 30, 8.f / 10, 1 };
Camera topMenuCamera(INIT_TOPMENU_CAMERA, vec3(0.6, 0.3, 0.3));
float INIT_RIGHTPANEL_CAMERA[4] = { 8.f / 10, 1.f / 3, 1, 2.f / 3 };
Camera rightMenuCamera(INIT_RIGHTPANEL_CAMERA, vec3(0.8, 0.8, 0.8));
float INIT_MINIMAP_CAMERA[4] = { 8.f / 10, 0, 1, 1.f / 3 };
Camera minimapCamera(INIT_MINIMAP_CAMERA, vec3(0, 0, 0));
// TODO recalculate minimap offset on resize so it fits perfectly
float INIT_MODEL_CAMERA[4] = { 8.f / 10, 2.f / 3, 1, 1 };
Camera modelCamera(INIT_MODEL_CAMERA, vec3(1, 1, 1));
float INIT_TROOPS_CAMERAS[HERO_UNIT_SLOTS][4];
Camera troopCamera[HERO_UNIT_SLOTS];

float INIT_PROMPT_CAMERA[4] = { 3.f / 10, 9.f / 20, 5.f / 10, 11.f / 20 };
Camera promptCamera(INIT_PROMPT_CAMERA, vec3(0.3, 0.3, 0.3));

float INIT_COMBAT_CAMERA[4] = { 0.25f, 0.25f, 0.75f, 0.75f };
Camera combatCamera(INIT_COMBAT_CAMERA, vec3(0, 0, 0));
//

void tempInitCameras() {
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

	//mapCamera.addRotation(vec4(-45, 1, 0, 0));
}

void displayTexture(GLuint id, float size) {
	glBindTexture(GL_TEXTURE_2D, id);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex3f(-size, -size, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-size, size, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(size, size, 0.0);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(size, -size, 0.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, NULL);
}

int bfsDistance[MAP_MAX_ROWS][MAP_MAX_COLS];
int bfsDirection[MAP_MAX_ROWS][MAP_MAX_COLS];

void bfs(intp origin) {
	memset(bfsDistance, -1, sizeof bfsDistance);
	bfsDistance[origin.x][origin.y] = 0;
	bfsDirection[origin.x][origin.y] = -1;

	queue<intp> bfsQueue;
	bfsQueue.push(origin);
	while (!bfsQueue.empty()) {
		intp current = bfsQueue.front();
		bfsQueue.pop();

		if (current != origin && (gameLogic.map->getObject(current)->isHolding() || gameLogic.map->isThreatened(current))) {
			continue;
		}

		for (int i = 0; i < HERO_MOVE_DIRS; i++) {
			intp next = current + HERO_MOVE_DIR[i];
			
			if (next.x >= 0 && next.y >= 0 && next.x < colCount && next.y < rowCount && !gameLogic.map->getObject(next)->isBlocking() && !~bfsDistance[(int)next.x][(int)next.y]) { // ~(-1) == 0
				bfsQueue.push(next);
				bfsDistance[next.x][next.y] = bfsDistance[current.x][current.y] + 1;
				bfsDirection[next.x][next.y] = (i + 2) % 4; // storing the reverse direction for backtracking
			}
		}
	}
}

void displayHeroes() {
	for (int i = 0; i < (int)gameLogic.players.size(); i++) {
		Player* player = gameLogic.players[i];
		for (int j = 0; j < (int)player->heroes.size(); j++) {
			Hero* hero = player->heroes[j];
			vec2 currentCoords = vec2(hero->pos.x + 0.5f, hero->pos.y + 0.5f) * testDim;

			if (gameLogic.currentPlayer == i && heroMoving == j) {
				// CHECK 5 - 10 == -5  ok?
				if (!hero->canMove(HERO_TILE_MOVE_COST)) {
					movingPath.clear();
				}
				if (!movingPath.empty()) {
					currentCoords = lerp(currentCoords, coordsFromMap(movingPath[movingPath.size() - 1], testDim), movingProgress);
					if (movingProgress >= 1) {
						movingProgress = 0;
						hero->pos = movingPath[movingPath.size() - 1];
						movingPath.pop_back();

						// TODO add different movement and costs?
						hero->move(HERO_TILE_MOVE_COST);
					}
				}
				else {
					// TODO refactor animation
					heroMoving = -1;
					isTileSelected = false;

					// TODO add tile activation stuff
					if (gameLogic.map->isThreatened(hero->pos)) {
						MapObject* threat = gameLogic.map->getFirstThreat(hero->pos);
						combatLogic->setupCombat(hero, threat);
					}
					else {
						gameLogic.map->getObject(hero->pos)->interact();
					}
				}
			}

			// draw the hero model
			glPushMatrix();
			glTranslatef(currentCoords.x, currentCoords.y, 0);
			glColor3f(COLORS[i].x, COLORS[i].y, COLORS[i].z);
			hero->draw(testDim);
			glPopMatrix();
		}
	}
}

void displayMapWindow() {
	glPushMatrix();

	// color the currently selected tile and the path to that tile (if one exists)
	glDisable(GL_LIGHTING);
	if (isTileSelected || ~heroMoving) {
		glPushMatrix();
		if (~bfsDistance[(int)selectedTile.x][(int)selectedTile.y]) {
			glColor3f(0, 1, 0);

			Hero* movingHero = gameLogic.getCurrentPlayer()->getCurrentHero();

			for (int i = 0; i < (int)movingPath.size(); i++) {
				// TODO modify costs? see displayHeroes
				if (!movingHero->canMove((i + 1) * HERO_TILE_MOVE_COST)) {
					glColor3f(1, 0, 0);
				}
				int currX = movingPath[movingPath.size() - i - 1].x;
				int currY = movingPath[movingPath.size() - i - 1].y;

				glRectf(currX * testDim, currY * testDim,
					(currX + 1) * testDim, (currY + 1) * testDim);
			}
		}
		else {
			glColor3f(1, 0, 0);
		}

		glRectf(selectedTile.x * testDim, selectedTile.y * testDim, (selectedTile.x + 1) * testDim, (selectedTile.y + 1) * testDim);
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);
	
	// draw the map elements
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			glPushMatrix();
			glTranslatef(i * testDim + testDim / 2, j * testDim + testDim / 2, testDim * 2 / 5);
			gameLogic.map->getObject(intp(i, j))->draw(testDim);
			glPopMatrix();
		}
	}

	// draw the map grid
	glDisable(GL_LIGHTING);
	if (debugShowGrid) {
		glPushMatrix();
		glColor3f(0, 0, 0);
		for (int i = 0; i <= rowCount; i++) {
			glBegin(GL_LINES);
			glVertex3f(0, i * testDim, 0);
			glVertex3f(colCount * testDim, i * testDim, 0);
			glEnd();
		}
		for (int j = 0; j <= colCount; j++) {
			glBegin(GL_LINES);
			glVertex3f(j * testDim, 0, 0);
			glVertex3f(j * testDim, rowCount * testDim, 0);
			glEnd();
		}
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);

	// write some map coordinates every here and there
	if (debugShowCoordinates) {
		glPushMatrix();
		for (int i = 0; i < rowCount; i += 5) {
			for (int j = 0; j < colCount; j += 5) {
				glRasterPos2f(i * testDim + testDim / 7, j * testDim + testDim / 7);
				drawText(GLUT_BITMAP_HELVETICA_12, to_string(i) + ", " + to_string(j));
			}
		}
		glPopMatrix();
	}
	
	// draw the heroes
	displayHeroes();

	glPopMatrix();
}

void displayMenuWindow() {
	for (int i = 0; i < (int)menuPositions.size(); i++) {
		glPushMatrix();
		glColor3f(0.8, 0.8, 0.8);
		glTranslatef(menuPositions[i].x, menuPositions[i].y, 0);
		menuButtons[i]->draw();
		glPopMatrix();
	}
}

float minimapTestDim = 1;
void displayMinimap() {
	minimapTestDim = mmin(minimapCamera.getViewport()[2] / (float)colCount, minimapCamera.getViewport()[3] / (float)rowCount);
	glPushMatrix();

	// draw the map grid
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.2);
	for (int i = 0; i <= rowCount; i++) {
		glBegin(GL_LINES);
		glVertex3f(0, i * minimapTestDim, 0);
		glVertex3f(colCount * minimapTestDim, i * minimapTestDim, 0);
		glEnd();
	}
	for (int j = 0; j <= colCount; j++) {
		glBegin(GL_LINES);
		glVertex3f(j * minimapTestDim, 0, 0);
		glVertex3f(j * minimapTestDim, rowCount * minimapTestDim, 0);
		glEnd();
	}
	glPopMatrix();

	// draw the simplified heroes
	for (int i = 0; i < (int)gameLogic.players.size(); i++) {
		glColor3f(COLORS[i].x, COLORS[i].y, COLORS[i].z);
		for (int j = 0; j < (int)gameLogic.players[i]->heroes.size(); j++) {
			intp heroPos = gameLogic.players[i]->heroes[j]->pos;
			glBegin(GL_TRIANGLES);
			glVertex2f((heroPos.x + 0.8f) * minimapTestDim, (heroPos.y + 0.9f) * minimapTestDim);
			glVertex2f((heroPos.x + 0.2f) * minimapTestDim, (heroPos.y + 0.9f) * minimapTestDim);
			glVertex2f((heroPos.x + 0.5f) * minimapTestDim, (heroPos.y + 0.1f) * minimapTestDim);
			glEnd();
		}
	}

	// draw walls
	for (int i = 0; i < rowCount; i++) {
		for (int j = 0; j < colCount; j++) {
			if (gameLogic.map->getObject(vec2(i, j))->isBlocking()) {
				glPushMatrix();
				glColor3f(0.8, 0.8, 0.8);
				glRectf(i * minimapTestDim, j * minimapTestDim,
					(i + 1) * minimapTestDim, (j + 1) * minimapTestDim);
				glPopMatrix();
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
	glVertex2f(pointSW.x / testDim * minimapTestDim, pointSW.y / testDim * minimapTestDim);
	glVertex2f(pointNE.x / testDim * minimapTestDim, pointSW.y / testDim * minimapTestDim);
	glVertex2f(pointNE.x / testDim * minimapTestDim, pointNE.y / testDim * minimapTestDim);
	glVertex2f(pointSW.x / testDim * minimapTestDim, pointNE.y / testDim * minimapTestDim);
	glEnd();
	glPopMatrix();

	glPopMatrix();
}

void displayPromptWindow() {
	// TODO
	glRasterPos2f(0.5, 
		(INIT_PROMPT_CAMERA[3] - INIT_PROMPT_CAMERA[1]) / 2 * windowHeight);
	drawText(GLUT_BITMAP_HELVETICA_18, ViewManager::promptText);
}

float testModelDim = 1.f;
float testRotPerc = 0;
void displayModelPreview() {
	glPushMatrix();

	testRotPerc += 0.001f;
	if (testRotPerc >= 1) {
		testRotPerc = 0;
	}
	glRotatef(-80, 1, 0, 0);
	glRotatef(testRotPerc * 360, 0, 0, 1);

	if (isTileSelected) {
		gameLogic.map->getObject(selectedTile)->draw(testModelDim);
	}
	else if (gameLogic.getCurrentPlayer()->getCurrentHero() != nullptr) {
		vec3 factionColor = COLORS[gameLogic.getCurrentPlayer()->getFactionId()];
		glColor3f(factionColor.r, factionColor.g, factionColor.b);
		gameLogic.getCurrentPlayer()->getCurrentHero()->draw(testModelDim);
	}

	glPopMatrix();
}

float combatTestDim = 0.05f;
Creature* combatTooltipTarget;
void displayCombat() {
	glPushMatrix();

	// highlight the available moves
	//cout << combatLogic.creatures.size() << endl;
	glDisable(GL_LIGHTING);
	glColor3f(0.2, 0.2, 0.2);
	for (vec2 validMove: combatLogic->validMoves) {
		glRectf(validMove.x * combatTestDim, validMove.y * combatTestDim,
			(validMove.x + 1) * combatTestDim, (validMove.y + 1) * combatTestDim);
	}
	glEnable(GL_LIGHTING);

	// highlight the active creature
	glDisable(GL_LIGHTING);
	glColor3f(0, 1, 0);
	vec2 activePos = combatLogic->getActiveCreature()->combatPos;
	glRectf(activePos.x * combatTestDim, activePos.y * combatTestDim,
		(activePos.x + 1) * combatTestDim, (activePos.y + 1) * combatTestDim);
	glEnable(GL_LIGHTING);

	// draw the combat grid
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glColor3f(1, 1, 1);
	for (int i = 0; i <= COMBAT_ROWS; i++) {
		glBegin(GL_LINES);
		glVertex3f(0, i * combatTestDim, 0);
		glVertex3f(COMBAT_COLS * combatTestDim, i * combatTestDim, 0);
		glEnd();
	}
	for (int j = 0; j <= COMBAT_COLS; j++) {
		glBegin(GL_LINES);
		glVertex3f(j * combatTestDim, 0, 0);
		glVertex3f(j * combatTestDim, COMBAT_ROWS * combatTestDim, 0);
		glEnd();
	}
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	// draw the creatures
	for (Creature* creature : combatLogic->creatures) {
		if (creature != nullptr && creature->count > 0) {
			vec3 tempColor = COLORS[creature->getFactionId()];
			glColor3f(tempColor.r, tempColor.g, tempColor.b);
			glPushMatrix();
			glTranslatef(creature->combatPos.x * combatTestDim + combatTestDim / 2,
				creature->combatPos.y * combatTestDim + combatTestDim / 2, 0);
			creature->draw(combatTestDim);
			glPopMatrix();
		}
	}

	glPopMatrix();
}

GLMmodel* testModel;

void myDisplay() {
	glClear(GL_DEPTH_BUFFER_BIT);

	// display the player-map portion of the window
	mapCamera.activateView();
	displayMapWindow();

	// display the model preview
	modelCamera.activateView();
	displayModelPreview();

	// display the right portion of the window
	rightMenuCamera.activateView();
	//displayStatusWindow();

	// display the minimap
	minimapCamera.activateView(Camera::ORTHO2D);
	displayMinimap();

	// display the menu bar at the top
	topMenuCamera.activateView(Camera::ORTHO2D);
	displayMenuWindow();

	// display the creature stacks on the right
	MapObject* creatureContainer = nullptr;
	if (isTileSelected) {
		if (gameLogic.map->getObject(selectedTile)->hasCreatures()) {
			creatureContainer = gameLogic.map->getObject(selectedTile);
		}
	}
	else {
		creatureContainer = gameLogic.getCurrentPlayer()->getCurrentHero();
	}
	if (creatureContainer != nullptr) {
		for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
			Creature* creature = creatureContainer->creatures[i];
			if (creature != nullptr && creature->count > 0) {
				troopCamera[i].activateView();

				glPushMatrix();
				glTranslatef(0, -0.2, 0);
				glRotatef(-80, 1, 0, 0);
				int tempFaction = creature->getFactionId();
				glColor3f(COLORS[tempFaction].r, COLORS[tempFaction].g, COLORS[tempFaction].b);
				creature->draw(1);
				glPopMatrix();

				troopCamera[i].activateView(Camera::ORTHO2D, false);

				glRasterPos2f((INIT_TROOPS_CAMERAS[i][2] - INIT_TROOPS_CAMERAS[i][0]) / 2 * windowWidth, 10);
				drawText(GLUT_BITMAP_HELVETICA_18, to_string(creature->count));
			}
		}
	}
	//

	if (ViewManager::isPromptUp) {
		promptCamera.activateView(Camera::ORTHO2D);
		displayPromptWindow();
	}

	// dislay the combat window
	if (combatLogic->combatActive) {
		combatCamera.activateView(Camera::NORMAL);
		displayCombat();

		// display the creature counts
		combatCamera.activateView(Camera::ORTHO2D, false);
		// TODO see Camera.h worldToViewPoint() (precalculate)
		for (Creature* creature : combatLogic->creatures) {
			if (creature != nullptr && creature->count > 0) {
				vec3 worldPoint = vec3(creature->combatPos.x * combatTestDim, creature->combatPos.y * combatTestDim, 0);
				// the stack count position depends on whether the stack is of the left/right faction
				if (creature->getFactionId() == combatLogic->attackerFaction) {
					worldPoint.x += combatTestDim;
				}
				else {
					worldPoint.x -= combatTestDim / 4;
					worldPoint.y += combatTestDim * 0.667f;
				}
				vec2 screenPos = combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL);
				if (creature->getFactionId() == combatLogic->attackerFaction) {
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
				drawText(GLUT_BITMAP_HELVETICA_12, to_string(creature->count));
			}
		}

		// display the creature tooltip if visible (if hovering over a creature)
		// TODO modify to adjust to font size, currently always using helvetica 12
		if (combatTooltipTarget != nullptr) {
			vec3 worldPoint = vec3(combatTooltipTarget->combatPos.x * combatTestDim + combatTestDim / 2,
				combatTooltipTarget->combatPos.y * combatTestDim + combatTestDim / 2, 0);
			vec2 screenPos = combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL);

			// move the tooltip if needed (so it doesn't display outside the view)
			vector<string> description = combatTooltipTarget->getDescription();
			if (screenPos.x >= combatCamera.getViewport()[2] / 2.f) {
				screenPos.x -= 100;
			}
			if (screenPos.y >= combatCamera.getViewport()[3] / 2.f) {
				screenPos.y -= description.size() * 15 + 30;
			}
			glColor3f(0.4f, 0.4f, 0.4f);
			glRectf(screenPos.x, screenPos.y, screenPos.x + 100, screenPos.y + description.size() * 15 + 30);
			glColor3f(0.7f, 0.7f, 0.7f);
			glRectf(screenPos.x + 5, screenPos.y + 5, screenPos.x + 95, screenPos.y + description.size() * 15 + 25);
			glColor3f(0, 0, 0);
			for (string descLine : combatTooltipTarget->getDescription()) {
				glRasterPos2d(screenPos.x + 15, screenPos.y + description.size() * 15);
				drawText(GLUT_BITMAP_HELVETICA_12, descLine);
				screenPos.y -= 15;
			}
		}
	}

	glutSwapBuffers();
}

double ZOOM_MIN = 0.5;
double ZOOM_MAX = 10;
void myIdle() {
	curr = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = curr - last;

	if (elapsed > 10) {
		last = curr;

		if (paused)
			return;

		if (ViewManager::isPromptUp) {
			ViewManager::testPromptTimer -= elapsed;
			if (ViewManager::testPromptTimer <= 0){
				ViewManager::isPromptUp = false;
			}
		}
		else {
			// update the camera location
			double m = sqrt(mapCamera.speed.x * mapCamera.speed.x + mapCamera.speed.y * mapCamera.speed.y);
			if (m > eps) {
				vec3 pointSW = mapCamera.viewToWorldPoint(windowWidth * INIT_MAP_CAMERA[0], windowHeight * INIT_MAP_CAMERA[1]);
				vec3 pointNE = mapCamera.viewToWorldPoint(windowWidth * INIT_MAP_CAMERA[2], windowHeight * INIT_MAP_CAMERA[3]);

				double newX = mapCamera.pos.x + mapCamera.speed.x * elapsed * 0.004 / m;
				double newY = mapCamera.pos.y + mapCamera.speed.y * elapsed * 0.004 / m;
				if ((debugFreeCamera) ||
					(newX < mapCamera.pos.x && pointSW.x + testDim >= 0) ||
					(newX > mapCamera.pos.x && pointNE.x - testDim <= colCount * testDim)) {
					mapCamera.pos.x = newX;
				}
				if ((debugFreeCamera) ||
					(newY < mapCamera.pos.y && pointSW.y + testDim >= 0) ||
					(newY > mapCamera.pos.y && pointNE.y - testDim <= rowCount * testDim)) {
					mapCamera.pos.y = newY;
				}
			}
			mapCamera.pos.z = clamp(ZOOM_MIN, ZOOM_MAX, mapCamera.pos.z + mapCamera.speed.z * elapsed * 0.004);

			// update the hero movement animation
			if (~heroMoving) {
				movingProgress += movingSpeed * elapsed;
			}
		}

		// display the scene
		myDisplay();

		frameCount++;
	}
}

void myReshape(int width, int height);
void myPassiveMouseMovement(int x, int y);
void myActiveMouseMovement(int x, int y);
void myMouse(int button, int state, int x, int y);
void mySpecial(int key, int x, int y);
void myKeyboard(unsigned char key, int x, int y);
void myKeyboardUp(unsigned char key, int x, int y);
void loadTexture(string texturePath, GLuint &saveId);

int main(int argc, char ** argv) {
	srand(time(NULL));

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(INITIAL_POS_X, INITIAL_POS_Y);
	glutInit(&argc, argv);

	glutCreateWindow("Homm3 \\:D/");
	glutReshapeFunc(myReshape);
	glutIdleFunc(myIdle);
	glutDisplayFunc(myDisplay);
	glutSpecialFunc(mySpecial); 
	glutPassiveMotionFunc(myPassiveMouseMovement);
	glutMotionFunc(myActiveMouseMovement);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);
	glutKeyboardUpFunc(myKeyboardUp);

	glutIgnoreKeyRepeat(true);
	
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 20.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); :( newer version
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glewInit(); // BEFORE loading the models

	Resources::loadCreatures();
	Resources::loadMapBlocks();
	Resources::loadModels();

	gameLogic.map = new Map(rowCount, colCount);
	gameLogic.map->fillMap();
	mapCamera.pos = vec3(testDim * colCount / 2, testDim * rowCount / 2, 1);
	// TEMP TODO change the y offset
	combatCamera.pos = vec3(COMBAT_COLS * combatTestDim / 2.f, COMBAT_ROWS * combatTestDim / 2.f - combatTestDim * 2, 1);

	tempCreateButtons();
	tempInitCameras();

	Player* player = new Player(1);
	gameLogic.players.push_back(player);
	player->addHero(new Hero(player));
	player->addHero(new Hero(player));
	Player* player2 = new Player(2);
	gameLogic.players.push_back(player2);
	player2->addHero(new Hero(player2));
	player2->addHero(new Hero(player2));

	player->getCurrentHero()->creatures[0] = new Creature("Monster", 15, player);
	player->getCurrentHero()->creatures[3] = new Creature("Monster", 5, player);
	player2->getCurrentHero()->creatures[1] = new Creature("Monster", 6, player2);
	//combatLogic->setupCombat(dynamic_cast<MapObject*>(player->getCurrentHero()),
	//	dynamic_cast<MapObject*>(player2->getCurrentHero()));

	glutMainLoop();
	return 0;
}

// TODO move to ~maingamewindowclass
void processMouseW(int button, int state, int x, int y) {
	// assuming (0, 0) to be bottom left

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		if (~heroMoving) {
			return; // we want to finish the current movement first
		}

		Hero* activeHero = gameLogic.getCurrentPlayer()->getCurrentHero();
		if (activeHero == nullptr) {
			return;
		}

		selectedPoint = mapCamera.viewToWorldPoint(x, y, Camera::NORMAL);

		// tile selection and player movement update
		vec2 tempTile = vec2(int(selectedPoint.x / testDim), int(selectedPoint.y / testDim));

		if (selectedPoint.x < 0 || selectedPoint.y < 0 ||
			selectedPoint.x > colCount * testDim || selectedPoint.y > rowCount * testDim) {
			isTileSelected = false;
		}
		else {
			// if the clicked tile is already selected the player wants to move there
			if (isTileSelected && tempTile == selectedTile) {
				heroMoving = -1;
				if (~bfsDistance[(int)selectedTile.x][(int)selectedTile.y]) {
					movingProgress = 0;
					heroMoving = gameLogic.getCurrentPlayer()->getCurrentHeroIndex();
				}
			}
			// select the clicked tile and find the shortest path to that tile
			else {
				isTileSelected = true;
				selectedTile = tempTile;

				bfs(activeHero->pos);

				vec2 current = selectedTile;
				int currentDir = bfsDirection[(int)current.x][(int)current.y];
				movingPath.clear();
				movingPath.push_back(current);
				while (~currentDir) {
					current += HERO_MOVE_DIR[currentDir];
					currentDir = bfsDirection[(int)current.x][(int)current.y];
					if (!~currentDir) {
						break; // we don't need to store the starting point
					}
					movingPath.push_back(current);
				}
			}
		}
	}
}

void myReshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;

	mapCamera.windowResized(width, height);
	topMenuCamera.windowResized(width, height);
	rightMenuCamera.windowResized(width, height);
	minimapCamera.windowResized(width, height);
	promptCamera.windowResized(width, height); // TODO listen to mapcam?
	modelCamera.windowResized(width, height);
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		troopCamera[i].windowResized(width, height);
	}
	combatCamera.windowResized(width, height);
}
/*
void loadTexture(string texturePath, GLuint &saveId) {
	saveId = SOIL_load_OGL_texture(texturePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	glBindTexture(GL_TEXTURE_2D, saveId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, NULL);
}*/

void processMouseTop(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		for (int i = 0; i < (int)menuButtons.size(); i++) {
			if (rectContains(menuPositions[i], menuButtons[i]->getDimension(), vec2(x, y))) {
				menuButtons[i]->activate();
			}
		}
	}
}

void processMouseMinimap(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		minimapNavigation = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		minimapNavigation = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void processMouseCombat(int button, int state, int x, int y) {
	// assuming (0, 0) to be bottom left
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		vec3 tempSelected = combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);
		vec2 test = combatCamera.worldToViewPoint(tempSelected, Camera::NORMAL);
		cout << "Mouse (" << x << ", " << y << ")\n World (" << tempSelected.x << ", " << tempSelected.y << ", " <<
			tempSelected.z << ")\n TestMouse (" << test.x << ", " << test.y << ")\n";
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		// TODO if (~animating) { // we want to finish the current movement first

		// TODO change, currently AI = faction 0
		if (combatLogic->getActiveFaction() == 0) {
			return;
		}

		vec3 tempSelected = combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

		// tile selection and player movement update
		vec2 tempTile = vec2(int(tempSelected.x / combatTestDim), int(tempSelected.y / combatTestDim));

		if (tempSelected.x < 0 || tempSelected.y < 0 ||
			tempSelected.x > COMBAT_COLS * combatTestDim || tempSelected.y > COMBAT_ROWS * combatTestDim) {
			return;
		}
		else {
			vec2 dirOffset = vec2(tempSelected.x, tempSelected.y) - tempTile * combatTestDim;
			int dirCode = 0;
			if (dirOffset.x > 0.25f * combatTestDim)
				dirCode += 3;
			if (dirOffset.x > 0.75 * combatTestDim)
				dirCode += 3;
			if (dirOffset.y > 0.25f * combatTestDim)
				dirCode += 1;
			if (dirOffset.y > 0.75 * combatTestDim)
				dirCode += 1;

			// try moving to the clicked tile
			combatLogic->move(tempTile, dirCode);
		}
	}
}

void myMouse(int button, int state, int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (combatLogic->combatActive) {
		if (rectContains(vec2(INIT_COMBAT_CAMERA[0], INIT_COMBAT_CAMERA[1]),
			vec2(INIT_COMBAT_CAMERA[2] - INIT_COMBAT_CAMERA[0], INIT_COMBAT_CAMERA[3] - INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			processMouseCombat(button, state, screenX - combatCamera.getViewport()[0], screenY - combatCamera.getViewport()[1]);
		}
	}
	else if (rectContains(vec2(INIT_MAP_CAMERA[0], INIT_MAP_CAMERA[1]),
		vec2(INIT_MAP_CAMERA[2] - INIT_MAP_CAMERA[0], INIT_MAP_CAMERA[3] - INIT_MAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
	
		processMouseW(button, state, screenX - mapCamera.getViewport()[0], screenY - mapCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(INIT_TOPMENU_CAMERA[0], INIT_TOPMENU_CAMERA[1]),
		vec2(INIT_TOPMENU_CAMERA[2] - INIT_TOPMENU_CAMERA[0], INIT_TOPMENU_CAMERA[3] - INIT_TOPMENU_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
		
		processMouseTop(button, state, screenX - topMenuCamera.getViewport()[0], screenY - topMenuCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(INIT_MINIMAP_CAMERA[0], INIT_MINIMAP_CAMERA[1]),
		vec2(INIT_MINIMAP_CAMERA[2] - INIT_MINIMAP_CAMERA[0], INIT_MINIMAP_CAMERA[3] - INIT_MINIMAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		processMouseMinimap(button, state, screenX - minimapCamera.getViewport()[0], screenY - minimapCamera.getViewport()[1]);
	}
}

void mySpecial(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT) {
	}
	else if (key == GLUT_KEY_RIGHT) {
	}
	else if (key == GLUT_KEY_DOWN) {
	}
	else if (key == GLUT_KEY_UP) {
	}
	else if (key == GLUT_KEY_F1) {
		paused = !paused;
	}
	else if (key == GLUT_KEY_F2) {
		gameLogic.map->clearMap();
	//	gameLogic.map->fillMap();
		gameLogic.map->testFillMap(5);
	}
	else if (key == GLUT_KEY_F3) {
		debugShowCoordinates = !debugShowCoordinates;
	}
	else if (key == GLUT_KEY_F4) {
		debugFreeCamera = !debugFreeCamera;
	}
	else if (key == GLUT_KEY_F5) {
		debugShowGrid = !debugShowGrid;
	}
}

void myKeyboard(unsigned char key, int x, int y) {
	if (key == 'w') {
		mapCamera.speed.y += 1;
	}
	else if (key == 's') {
		mapCamera.speed.y -= 1;
	}
	else if (key == 'a') {
		mapCamera.speed.x -= 1;
	}
	else if (key == 'd') {
		mapCamera.speed.x += 1;
	}
	else if (key == 'q') {
		mapCamera.speed.z -= 1;
	}
	else if (key == 'e') {
		mapCamera.speed.z += 1;
	}
	else if (key == 27) {
		exit(0);
	}
}

void myKeyboardUp(unsigned char key, int x, int y) {
	if (key == 'w') {
		mapCamera.speed.y -= 1;
	}
	else if (key == 's') {
		mapCamera.speed.y += 1;
	}
	else if (key == 'a') {
		mapCamera.speed.x += 1;
	}
	else if (key == 'd') {
		mapCamera.speed.x -= 1;
	}
	else if (key == 'q') {
		mapCamera.speed.z += 1;
	}
	else if (key == 'e') {
		mapCamera.speed.z -= 1;
	}
	else if (key >= '1' && key <= '9') {
		gameLogic.getCurrentPlayer()->setHeroByIndex(key - '1');
		isTileSelected = false; // TODO automatism?
	}
}

void myPassiveMouseMovement(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (combatLogic->combatActive) {
		if (rectContains(vec2(INIT_COMBAT_CAMERA[0], INIT_COMBAT_CAMERA[1]),
			vec2(INIT_COMBAT_CAMERA[2] - INIT_COMBAT_CAMERA[0], INIT_COMBAT_CAMERA[3] - INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			x = screenX - combatCamera.getViewport()[0];
			y = screenY - combatCamera.getViewport()[1];

			vec3 tempSelected = combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

			// tile selection and player movement update
			vec2 tempTile = vec2(int(tempSelected.x / combatTestDim), int(tempSelected.y / combatTestDim));

			combatTooltipTarget = nullptr;
			if (tempSelected.x < 0 || tempSelected.y < 0 ||
				tempSelected.x > COMBAT_COLS * combatTestDim || tempSelected.y > COMBAT_ROWS * combatTestDim) {
				return;
			}

			for (Creature* creature : combatLogic->creatures) {
				if (creature->combatPos == tempTile) {
					combatTooltipTarget = creature;
					break;
				}
			}
		}
	}
}

void myActiveMouseMinimap(int x, int y) {
	if (minimapNavigation) {
		// TEMP until the UI minimap becomes square (todo)
		float minDim = mmin(minimapCamera.getViewport()[2], minimapCamera.getViewport()[3]);
		float xTrue = x / minDim, yTrue = y / minDim;

		// TEMP until we change test dimensions
		mapCamera.pos = vec3(xTrue * colCount * testDim, yTrue * rowCount * testDim, mapCamera.pos.z);
	}
}

void myActiveMouseMovement(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (minimapNavigation) {
		if (!rectContains(vec2(INIT_MINIMAP_CAMERA[0], INIT_MINIMAP_CAMERA[1]),
			vec2(INIT_MINIMAP_CAMERA[2] - INIT_MINIMAP_CAMERA[0], INIT_MINIMAP_CAMERA[3] - INIT_MINIMAP_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
			minimapNavigation = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		else {
			myActiveMouseMinimap(screenX - minimapCamera.getViewport()[0], screenY - minimapCamera.getViewport()[1]);
		}
	}
}