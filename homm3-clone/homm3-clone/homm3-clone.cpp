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
#include <SOIL.h>
#include "objglm/objglm.h"
#include "Utility.h"
#include "Constants.h"
#include "FactionSetup.h"
#include "Model.h"
#include "UIButton.h"
#include "Camera.h"
#include "Map.h"
#include "ViewManager.h"
#include "Player.h"
#include "GameLogic.h"
#include "CombatLogic.h"
#include "Resources.h"
#include "DebugParameters.h"

using namespace std;
using namespace glm;

int windowWidth = INITIAL_WIDTH;
int windowHeight = INITIAL_HEIGHT;

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

GLuint backgroundTexture;
GLuint combatBackgroundTexture;

const int rowCount = 100, colCount = 100;
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
FactionSetup& factionSetup = FactionSetup::instance();
CombatLogic& combatLogic = CombatLogic::instance();
ViewManager& viewManager = ViewManager::instance();

void displayTexture(GLuint id, float width, float height, vec3 colorMult = vec3(1, 1, 1)) {
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
		for (int j = 0; j < (int)player->heroObjects.size(); j++) {
			MOHero* heroObject = player->heroObjects[j];
			vec2 currentCoords = vec2(heroObject->pos.x + 0.5f, heroObject->pos.y + 0.5f) * testDim;

			if (gameLogic.currentPlayer == i && heroMoving == j) {
				// CHECK 5 - 10 == -5  ok?
				if (!heroObject->hero->canMove(1)) {
					movingPath.clear();
				}
				if (!movingPath.empty()) {
					currentCoords = lerp(currentCoords, coordsFromMap(movingPath[movingPath.size() - 1], testDim), movingProgress);
					if (movingProgress >= 1) {
						movingProgress = 0;
						heroObject->pos = movingPath[movingPath.size() - 1];
						movingPath.pop_back();

						// TODO add different movement and costs?
						heroObject->hero->move(1);
						player->pf.makeVisibleAround(heroObject->pos, 5);
					}
				}
				else {
					// TODO refactor animation
					heroMoving = -1;
					isTileSelected = false;

					// TODO add tile activation stuff
					gameLogic.interact(heroObject);
				}
			}

			// draw the hero model
			glPushMatrix();
			glTranslatef(currentCoords.x, currentCoords.y, 0);
			vec3 tempColor = factionSetup.getFactionColor(player);
			glColor3f(tempColor.r, tempColor.g, tempColor.b);
			heroObject->draw(testDim);
			glPopMatrix();
		}
	}
}

void displayMapWindow() {
	glPushMatrix();

	// TEMP draw the background
	if (debugShowBackground) {
		glPushMatrix();
		glTranslatef(0, 0, -0.001);
		displayTexture(backgroundTexture, testDim * colCount, testDim * rowCount, vec3(0.75, 0.75, 0.75));
		glPopMatrix();
	}

	// color the currently selected tile and the path to that tile (if one exists)
	glDisable(GL_LIGHTING);
	if (isTileSelected || ~heroMoving) {
		glPushMatrix();
		if (~bfsDistance[(int)selectedTile.x][(int)selectedTile.y]) {
			glColor3f(0, 1, 0);

			MOHero* movingHero = gameLogic.getCurrentPlayer()->getCurrentHero();

			for (int i = 0; i < (int)movingPath.size(); i++) {
				// TODO modify costs? see displayHeroes
				if (!movingHero->hero->canMove((i + 1))) {
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
	Pathfinder &pf =
		(debugWhoseFogToShow == -1) ?
		gameLogic.getCurrentPlayer()->pf :
		gameLogic.getPlayerByIndex(debugWhoseFogToShow)->pf;

	float dimThird = testDim / 3;
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			if (pf.tileVisible[i][j] || !debugShowFogOfWar) {
				glPushMatrix();
				glTranslatef(i * testDim + testDim / 2, j * testDim + testDim / 2, testDim * 2 / 5);
				gameLogic.map->getObject(intp(i, j))->draw(testDim);
				glPopMatrix();
			}
			else {
				// TEMP fog of war, modify to show accurately
				glDisable(GL_LIGHTING);
				glColor3f(0.3, 0.3, 0.3);
				glRectf(i * testDim + dimThird, j * testDim + dimThird,
					i* testDim + 2 * dimThird, j * testDim + 2 * dimThird);
				for (int k = 0; k < 8; k++) {
					int ni = i + COMBAT_MOVE_DIR[k].x;
					int nj = j + COMBAT_MOVE_DIR[k].y;
					bool dispFog = false;
					if (ni < 0 || nj < 0 || ni >= colCount || nj >= rowCount) {
						dispFog = true;
					}
					else if ((!pf.tileVisible[ni][nj] && k < 4) ||
						(k >= 4 && !pf.tileVisible[ni][nj] && !pf.tileVisible[i][nj] && !pf.tileVisible[ni][j])) {
						dispFog = true;
					}
					if (dispFog) {
						glRectf(
							i * testDim + dimThird + COMBAT_MOVE_DIR[k].x * dimThird,
							j * testDim + dimThird + COMBAT_MOVE_DIR[k].y * dimThird,
							i * testDim + 2 * dimThird + COMBAT_MOVE_DIR[k].x * dimThird,
							j * testDim + 2 * dimThird + COMBAT_MOVE_DIR[k].y * dimThird);
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

	// TODO make ui text classes etc.
	glPushMatrix();
	Currency curr = gameLogic.getCurrentPlayer()->wallet;
	string resourceText =
		"Gold: " + to_string(curr[GOLD]) + "     Wood: " + to_string(curr[WOOD]) +
		"     Ore: " + to_string(curr[ORE]) + "     Crystal: " + to_string(curr[CRYSTAL]);
	glColor3f(0, 0, 0);
	glRasterPos2d(500, 15);
	glDisable(GL_DEPTH_TEST);
	drawText(GLUT_BITMAP_HELVETICA_12, resourceText);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

//float minimapTestDim = 1;
void displayMinimap() {
	//minimapTestDim = mmin(viewManager.minimapCamera.getViewport()[2] / (float)colCount, minimapCamera.getViewport()[3] / (float)rowCount);
	glPushMatrix();

	// draw the map grid
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.2);
	for (int i = 0; i <= rowCount; i++) {
		glBegin(GL_LINES);
		glVertex3f(0, i * viewManager.minimapCamera.unit, 0);
		glVertex3f(colCount * viewManager.minimapCamera.unit, i * viewManager.minimapCamera.unit, 0);
		glEnd();
	}
	for (int j = 0; j <= colCount; j++) {
		glBegin(GL_LINES);
		glVertex3f(j * viewManager.minimapCamera.unit, 0, 0);
		glVertex3f(j * viewManager.minimapCamera.unit, rowCount * viewManager.minimapCamera.unit, 0);
		glEnd();
	}
	glPopMatrix();

	// draw the simplified heroes
	for (int i = 0; i < (int)gameLogic.players.size(); i++) {
		glColor3f(COLORS[i].x, COLORS[i].y, COLORS[i].z);
		for (int j = 0; j < (int)gameLogic.players[i]->heroObjects.size(); j++) {
			intp heroPos = gameLogic.players[i]->heroObjects[j]->pos;
			glBegin(GL_TRIANGLES);
			glVertex2f((heroPos.x + 0.8f) * viewManager.minimapCamera.unit, (heroPos.y + 0.9f) * viewManager.minimapCamera.unit);
			glVertex2f((heroPos.x + 0.2f) * viewManager.minimapCamera.unit, (heroPos.y + 0.9f) * viewManager.minimapCamera.unit);
			glVertex2f((heroPos.x + 0.5f) * viewManager.minimapCamera.unit, (heroPos.y + 0.1f) * viewManager.minimapCamera.unit);
			glEnd();
		}
	}

	// draw walls
	Pathfinder &pf =
		(debugWhoseFogToShow == -1) ?
		gameLogic.getCurrentPlayer()->pf :
		gameLogic.getPlayerByIndex(debugWhoseFogToShow)->pf;
	for (int i = 0; i < rowCount; i++) {
		for (int j = 0; j < colCount; j++) {
			if (pf.tileVisible[i][j] || !debugShowFogOfWar) {
				if (gameLogic.map->getObject(vec2(i, j))->isBlocking()) {
					glPushMatrix();
					glColor3f(0.8, 0.8, 0.8);
					glRectf(i * viewManager.minimapCamera.unit, j * viewManager.minimapCamera.unit,
						(i + 1) * viewManager.minimapCamera.unit, (j + 1) * viewManager.minimapCamera.unit);
					glPopMatrix();
				}
			}
		}
	}

	// draw the area visible in the main view
	glPushMatrix();
	// TODO remove duplicate code for SW - NE calculation
	vec3 pointSW = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[0], windowHeight * viewManager.INIT_MAP_CAMERA[1]);
	vec3 pointNE = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[2], windowHeight * viewManager.INIT_MAP_CAMERA[3]);
	//
	glColor3f(0, 0.9, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(pointSW.x / testDim * viewManager.minimapCamera.unit,
		pointSW.y / testDim * viewManager.minimapCamera.unit);
	glVertex2f(pointNE.x / testDim * viewManager.minimapCamera.unit,
		pointSW.y / testDim * viewManager.minimapCamera.unit);
	glVertex2f(pointNE.x / testDim * viewManager.minimapCamera.unit,
		pointNE.y / testDim * viewManager.minimapCamera.unit);
	glVertex2f(pointSW.x / testDim * viewManager.minimapCamera.unit,
		pointNE.y / testDim * viewManager.minimapCamera.unit);
	glEnd();
	glPopMatrix();

	glPopMatrix();
}

void displayPromptWindow() {
	// TODO
	glRasterPos2f(0.5, 
		(viewManager.INIT_PROMPT_CAMERA[3] - viewManager.INIT_PROMPT_CAMERA[1]) / 2 * windowHeight);
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
	else if (gameLogic.getCurrentPlayer() != nullptr && gameLogic.getCurrentPlayer()->getCurrentHero() != nullptr) {
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

	// TEMP draw the background
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(-combatTestDim * COMBAT_COLS, -combatTestDim * COMBAT_ROWS, -0.001);
	//glTranslatef(0, 0, -0.001);
	displayTexture(combatBackgroundTexture, combatTestDim * COMBAT_COLS * 3, combatTestDim * COMBAT_ROWS * 3);
	glPopMatrix();

	// highlight the available moves
	if (!factionSetup.isAI[combatLogic.getActiveFaction()]) {
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		for (vec2 validMove : combatLogic.validMoves) {
			glRectf(validMove.x * combatTestDim, validMove.y * combatTestDim,
				(validMove.x + 1) * combatTestDim, (validMove.y + 1) * combatTestDim);
		}
		glEnable(GL_LIGHTING);
	}

	// highlight the active creature
	glDisable(GL_LIGHTING);
	glColor3f(0, 1, 0);
	vec2 activePos = combatLogic.getActiveCreature()->combatPos;
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
	for (Creature* creature : combatLogic.creatures) {
		if (creature != nullptr && creature->count > 0) {
			//vec3 tempColor = COLORS[creature->getFactionId()];
			//glColor3f(tempColor.r, tempColor.g, tempColor.b);
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
	viewManager.mapCamera.activateView();
	displayMapWindow();

	// display the model preview
	viewManager.modelCamera.activateView();
	displayModelPreview();

	// display the right portion of the window
	viewManager.rightMenuCamera.activateView();
	//displayStatusWindow();

	// display the minimap
	viewManager.minimapCamera.activateView(Camera::ORTHO2D);
	displayMinimap();

	// display the menu bar at the top
	viewManager.topMenuCamera.activateView(Camera::ORTHO2D);
	displayMenuWindow();

	// display the creature stacks on the right
	Hero* creatureContainer = nullptr;
	if (isTileSelected) {
		creatureContainer = gameLogic.map->getObject(selectedTile)->hero;
	}
	else if (gameLogic.getCurrentPlayer()->getCurrentHero() != nullptr){
		creatureContainer = gameLogic.getCurrentPlayer()->getCurrentHero()->hero;
	}
	if (creatureContainer != nullptr) {
		for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
			Creature* creature = creatureContainer->creatures[i];
			if (creature != nullptr && creature->count > 0) {
				viewManager.troopCamera[i].activateView();

				glPushMatrix();
				glTranslatef(0, -0.2, 0);
				glRotatef(-80, 1, 0, 0);
				int tempFaction = creature->getFactionId();
				glColor3f(COLORS[tempFaction].r, COLORS[tempFaction].g, COLORS[tempFaction].b);
				creature->draw(1);
				glPopMatrix();

				viewManager.troopCamera[i].activateView(Camera::ORTHO2D, false);

				//glRasterPos2f((INIT_TROOPS_CAMERAS[i][2] - INIT_TROOPS_CAMERAS[i][0]) / 2 * windowWidth, 25);
				glRasterPos2f(25, 25);
				drawText(GLUT_BITMAP_HELVETICA_12, to_string(creature->count));
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
	//

	if (ViewManager::isPromptUp) {
		viewManager.promptCamera.activateView(Camera::ORTHO2D);
		displayPromptWindow();
	}

	// dislay the combat window
	if (combatLogic.combatActive) {
		viewManager.combatCamera.activateView(Camera::NORMAL);
		displayCombat();

		// display the creature counts
		viewManager.combatCamera.activateView(Camera::ORTHO2D, false);
		// TODO see Camera.h worldToViewPoint() (precalculate)
		for (Creature* creature : combatLogic.creatures) {
			if (creature != nullptr && creature->count > 0) {
				vec3 worldPoint = vec3(creature->combatPos.x * combatTestDim, creature->combatPos.y * combatTestDim, 0);
				// the stack count position depends on whether the stack is of the left/right faction
				if (creature->getFactionId() == combatLogic.attackerFaction) {
					worldPoint.x += combatTestDim;
				}
				else {
					worldPoint.x -= combatTestDim / 4;
					worldPoint.y += combatTestDim * 0.667f;
				}
				vec2 screenPos = viewManager.combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL);
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
				drawText(GLUT_BITMAP_HELVETICA_12, to_string(creature->count));
			}
		}

		// display the creature tooltip if visible (if hovering over a creature)
		// TODO modify to adjust to font size, currently always using helvetica 12
		if (combatTooltipTarget != nullptr) {
			vec3 worldPoint = vec3(combatTooltipTarget->combatPos.x * combatTestDim + combatTestDim / 2,
				combatTooltipTarget->combatPos.y * combatTestDim + combatTestDim / 2, 0);
			vec2 screenPos = viewManager.combatCamera.worldToViewPoint(worldPoint, Camera::NORMAL);

			// move the tooltip if needed (so it doesn't display outside the view)
			vector<string> description = combatTooltipTarget->getDescription();
			if (screenPos.x >= viewManager.combatCamera.getViewport()[2] / 2.f) {
				screenPos.x -= 100;
			}
			if (screenPos.y >= viewManager.combatCamera.getViewport()[3] / 2.f) {
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

double ZOOM_MIN = 1;
double ZOOM_MAX = 20;
void myIdle() {
	curr = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = curr - last;

	if (elapsed > 10) {
		last = curr;

		// archive dead heroes
		for (Player* player : gameLogic.players) {
			for (int i = 0; i < (int)player->heroObjects.size(); ) {
				if (player->heroObjects[i]->hero->isDead) {
					player->archiveHero(i);
				}
				else {
					i++;
				}
			}
		}

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
			double m = sqrt(
				viewManager.mapCamera.speed.x * viewManager.mapCamera.speed.x +
				viewManager.mapCamera.speed.y * viewManager.mapCamera.speed.y);
			if (m > eps) {
				vec3 pointSW = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[0], windowHeight * viewManager.INIT_MAP_CAMERA[1]);
				vec3 pointNE = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[2], windowHeight * viewManager.INIT_MAP_CAMERA[3]);

				double newX = viewManager.mapCamera.pos.x + viewManager.mapCamera.speed.x * elapsed * 0.004 / m;
				double newY = viewManager.mapCamera.pos.y + viewManager.mapCamera.speed.y * elapsed * 0.004 / m;
				if ((debugFreeCamera) ||
					(newX < viewManager.mapCamera.pos.x && pointSW.x + testDim >= 0) ||
					(newX > viewManager.mapCamera.pos.x && pointNE.x - testDim <= colCount * testDim)) {
					viewManager.mapCamera.pos.x = newX;
				}
				if ((debugFreeCamera) ||
					(newY < viewManager.mapCamera.pos.y && pointSW.y + testDim >= 0) ||
					(newY > viewManager.mapCamera.pos.y && pointNE.y - testDim <= rowCount * testDim)) {
					viewManager.mapCamera.pos.y = newY;
				}
			}
			viewManager.mapCamera.pos.z =
				clamp(ZOOM_MIN, ZOOM_MAX, viewManager.mapCamera.pos.z + viewManager.mapCamera.speed.z * elapsed * 0.004);

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

	// TODO move texture(s) to resources
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	loadTexture(INPUT_BACKGROUND_TEXTURE, backgroundTexture);
	loadTexture(INPUT_COMBAT_BACKGROUND_TEXTURE, combatBackgroundTexture);

	gameLogic.map = new Map(rowCount, colCount);
	gameLogic.map->testFillMap();
	viewManager.mapCamera.pos = vec3(testDim * colCount / 2, testDim * rowCount / 2, 1);
	// TEMP TODO change the y offset
	viewManager.combatCamera.pos = vec3(COMBAT_COLS * combatTestDim / 2.f, COMBAT_ROWS * combatTestDim / 2.f - combatTestDim * 2, 1);
	viewManager.minimapCamera.setGrid(intp(colCount, rowCount));

	tempCreateButtons();
	//tempInitCameras();

	// CARE if first player is AI -> thread problems
	Player* player1 = gameLogic.addPlayer(false);
	player1->addNewHero();
	player1->addNewHero();
	player1->getCurrentHero()->hero->creatures[0] = new Creature("Swordsman", 15, player1);
	player1->getCurrentHero()->hero->creatures[3] = new Creature("Swordsman", 5, player1);

	Player* player2 = gameLogic.addPlayer(true);
	player2->addNewHero();
	player2->getCurrentHero()->hero->creatures[1] = new Creature("Griffin", 6, player2);
	player2->getCurrentHero()->hero->creatures[2] = new Creature("Swordsman", 10, player2);

	gameLogic.aiThread = std::thread(&GameLogic::aiThreadJob, &gameLogic);

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

		MOHero* activeHero = gameLogic.getCurrentPlayer()->getCurrentHero();
		if (factionSetup.isAI[gameLogic.getCurrentPlayer()->getFactionId()] || activeHero == nullptr) {
			return;
		}

		selectedPoint = viewManager.mapCamera.viewToWorldPoint(x, y, Camera::NORMAL);

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

	viewManager.mapCamera.windowResized(width, height);
	viewManager.topMenuCamera.windowResized(width, height);
	viewManager.rightMenuCamera.windowResized(width, height);
	viewManager.minimapCamera.windowResized(width, height);
	viewManager.promptCamera.windowResized(width, height); // TODO listen to mapcam?
	viewManager.modelCamera.windowResized(width, height);
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		viewManager.troopCamera[i].windowResized(width, height);
	}
	viewManager.combatCamera.windowResized(width, height);
}

void loadTexture(string texturePath, GLuint &saveId) {
	saveId = SOIL_load_OGL_texture(texturePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	glBindTexture(GL_TEXTURE_2D, saveId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, NULL);
}

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
		vec3 tempSelected = viewManager.combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);
		vec2 test = viewManager.combatCamera.worldToViewPoint(tempSelected, Camera::NORMAL);
		cout << "Mouse (" << x << ", " << y << ")\n World (" << tempSelected.x << ", " << tempSelected.y << ", " <<
			tempSelected.z << ")\n TestMouse (" << test.x << ", " << test.y << ")\n";
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		// TODO if (~animating) { // we want to finish the current movement first

		// TODO change, currently AI = faction 0
		if (combatLogic.getActiveFaction() == 0) {
			return;
		}

		vec3 tempSelected = viewManager.combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

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
			combatLogic.move(tempTile, dirCode);
		}
	}
}

void myMouse(int button, int state, int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (combatLogic.combatActive) {
		if (rectContains(vec2(viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[1]),
			vec2(viewManager.INIT_COMBAT_CAMERA[2] - viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[3] - viewManager.INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			processMouseCombat(
				button,
				state,
				screenX - viewManager.combatCamera.getViewport()[0],
				screenY - viewManager.combatCamera.getViewport()[1]);
		}
	}
	else if (rectContains(vec2(viewManager.INIT_MAP_CAMERA[0], viewManager.INIT_MAP_CAMERA[1]),
		vec2(viewManager.INIT_MAP_CAMERA[2] - viewManager.INIT_MAP_CAMERA[0], viewManager.INIT_MAP_CAMERA[3] - viewManager.INIT_MAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
	
		processMouseW(
			button,
			state,
			screenX - viewManager.mapCamera.getViewport()[0],
			screenY - viewManager.mapCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2(viewManager.INIT_TOPMENU_CAMERA[2] - viewManager.INIT_TOPMENU_CAMERA[0], viewManager.INIT_TOPMENU_CAMERA[3] - viewManager.INIT_TOPMENU_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
		
		processMouseTop(
			button,
			state,
			screenX - viewManager.topMenuCamera.getViewport()[0],
			screenY - viewManager.topMenuCamera.getViewport()[1]);
	}
	else if (rectContains(vec2(viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[1]),
		vec2(viewManager.INIT_MINIMAP_CAMERA[2] - viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[3] - viewManager.INIT_MINIMAP_CAMERA[1]),
		vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

		processMouseMinimap(
			button,
			state,
			screenX - viewManager.minimapCamera.getViewport()[0],
			screenY - viewManager.minimapCamera.getViewport()[1]);
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
		debugFreeCamera = !debugFreeCamera;
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
		if (debugWhoseFogToShow == gameLogic.players.size()) {
			debugWhoseFogToShow = -1; // will show the current player's fog
		}
	}
}

void myKeyboard(unsigned char key, int x, int y) {
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
		MOHero* hero = gameLogic.getCurrentPlayer()->getCurrentHero();
		if (hero != nullptr) {
			viewManager.mapCameraLookAt(hero->pos);
		}
	}
	else if (key == 27) {
		gameLogic.quitGame();
	}
}

void myKeyboardUp(unsigned char key, int x, int y) {
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
		isTileSelected = false; // TODO automatism?
	}
}

void myPassiveMouseMovement(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (combatLogic.combatActive) {
		if (rectContains(vec2(viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[1]),
			vec2(viewManager.INIT_COMBAT_CAMERA[2] - viewManager.INIT_COMBAT_CAMERA[0], viewManager.INIT_COMBAT_CAMERA[3] - viewManager.INIT_COMBAT_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {

			x = screenX - viewManager.combatCamera.getViewport()[0];
			y = screenY - viewManager.combatCamera.getViewport()[1];

			vec3 tempSelected = viewManager.combatCamera.viewToWorldPoint(x, y, Camera::NORMAL);

			// tile selection and player movement update
			vec2 tempTile = vec2(int(tempSelected.x / combatTestDim), int(tempSelected.y / combatTestDim));

			combatTooltipTarget = nullptr;
			if (tempSelected.x < 0 || tempSelected.y < 0 ||
				tempSelected.x > COMBAT_COLS * combatTestDim || tempSelected.y > COMBAT_ROWS * combatTestDim) {
				return;
			}

			for (Creature* creature : combatLogic.creatures) {
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
		float minDim = mmin(viewManager.minimapCamera.getViewport()[2], viewManager.minimapCamera.getViewport()[3]);
		float xTrue = x / minDim, yTrue = y / minDim;

		// TEMP until we change test dimensions
		viewManager.mapCamera.pos = vec3(xTrue * colCount * testDim, yTrue * rowCount * testDim, viewManager.mapCamera.pos.z);
	}
}

void myActiveMouseMovement(int x, int y) {
	int screenX = x;
	int screenY = windowHeight - y;

	if (minimapNavigation) {
		if (!rectContains(vec2(viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[1]),
			vec2(viewManager.INIT_MINIMAP_CAMERA[2] - viewManager.INIT_MINIMAP_CAMERA[0], viewManager.INIT_MINIMAP_CAMERA[3] - viewManager.INIT_MINIMAP_CAMERA[1]),
			vec2((float)screenX / windowWidth, (float)screenY / windowHeight))) {
			minimapNavigation = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		else {
			myActiveMouseMinimap(screenX - viewManager.minimapCamera.getViewport()[0], screenY - viewManager.minimapCamera.getViewport()[1]);
		}
	}
}