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
#include "InputManager.h"
#include "FactionSetup.h"
#include "Map.h"
#include "DebugParameters.h"
#include "ViewManager.h"
#include "Player.h"
#include "GameLogic.h"
#include "CombatLogic.h"
#include "Resources.h"

int windowWidth = INITIAL_WIDTH;
int windowHeight = INITIAL_HEIGHT;

int frameCount;
int curr, last;
bool paused;

GameLogic& gameLogic = GameLogic::instance();
FactionSetup& factionSetup = FactionSetup::instance();
CombatLogic& combatLogic = CombatLogic::instance();
ViewManager& viewManager = ViewManager::instance();
InputManager& inputManager = InputManager::instance();

void myDisplay() {
	glClear(GL_DEPTH_BUFFER_BIT);

	viewManager.displayActiveWindows();

	glutSwapBuffers();
}

double ZOOM_MIN = 1;
double ZOOM_MAX = 20;
void myIdle() {
	curr = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = curr - last;

	if (elapsed > 10) {
		last = curr;

		if (paused)
			return;

		// archive dead heroes
		if (!gameLogic.isAiActive()) {
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
		}

		// update the camera location
		double m = sqrt(
			viewManager.mapCamera.speed.x * viewManager.mapCamera.speed.x +
			viewManager.mapCamera.speed.y * viewManager.mapCamera.speed.y);
		if (m > eps) {
			glm::vec3 pointSW = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[0], windowHeight * viewManager.INIT_MAP_CAMERA[1]);
			glm::vec3 pointNE = viewManager.mapCamera.viewToWorldPoint(windowWidth * viewManager.INIT_MAP_CAMERA[2], windowHeight * viewManager.INIT_MAP_CAMERA[3]);

			double newX = viewManager.mapCamera.pos.x + viewManager.mapCamera.speed.x * elapsed * 0.004 / m;
			double newY = viewManager.mapCamera.pos.y + viewManager.mapCamera.speed.y * elapsed * 0.004 / m;
			if ((debugFreeCamera) ||
				(newX < viewManager.mapCamera.pos.x && pointSW.x + viewManager.mapUnit >= 0) ||
				(newX > viewManager.mapCamera.pos.x && pointNE.x - viewManager.mapUnit <= gameLogic.colCount * viewManager.mapUnit)) {
				viewManager.mapCamera.pos.x = newX;
			}
			if ((debugFreeCamera) ||
				(newY < viewManager.mapCamera.pos.y && pointSW.y + viewManager.mapUnit >= 0) ||
				(newY > viewManager.mapCamera.pos.y && pointNE.y - viewManager.mapUnit <= gameLogic.rowCount * viewManager.mapUnit)) {
				viewManager.mapCamera.pos.y = newY;
			}
		}
		viewManager.mapCamera.pos.z =
			clamp(ZOOM_MIN, ZOOM_MAX, viewManager.mapCamera.pos.z + viewManager.mapCamera.speed.z * elapsed * 0.004);

		// update the hero movement animation
		if (~viewManager.heroMoving) {
			viewManager.movingProgress += viewManager.movingSpeed * elapsed;
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
	//unsigned int seed = time(NULL);
	//srand(seed);
	srand(1465428796);
	//printf("%d\n", seed);

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
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glewInit(); // BEFORE loading the models

	Resources::loadModels();
	Resources::loadCreatures();
	Resources::loadMapBlocks();

	// TODO move texture(s) to resources
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	loadTexture(INPUT_BACKGROUND_TEXTURE, viewManager.backgroundTexture);
	loadTexture(INPUT_COMBAT_BACKGROUND_TEXTURE, viewManager.combatBackgroundTexture);

	gameLogic.map = new Map(gameLogic.rowCount, gameLogic.colCount);
	gameLogic.map->testFillMap();
	viewManager.mapCamera.pos = glm::vec3(viewManager.mapUnit * gameLogic.colCount / 2, viewManager.mapUnit * gameLogic.rowCount / 2, 1);
	// TEMP TODO change the y offset
	viewManager.combatCamera.pos =
		glm::vec3(COMBAT_COLS * viewManager.combatUnit / 2.f,
			COMBAT_ROWS * viewManager.combatUnit / 2.f - viewManager.combatUnit * 2, 1);
	viewManager.minimapCamera.setGrid(intp(gameLogic.colCount, gameLogic.rowCount));

	Player* player1 = gameLogic.addPlayer(false);
	player1->addNewHero();
	player1->getCurrentHero()->hero->creatures[3] = new Creature("Swordsman", 5, player1);
	player1->addNewHero();
	player1->setNextHero();
	player1->getCurrentHero()->hero->creatures[0] = new Creature("Swordsman", 15, player1);
	player1->getCurrentHero()->hero->creatures[3] = new Creature("Swordsman", 5, player1);

	Player* player2 = gameLogic.addPlayer(true);
	player2->addNewHero();
	player2->getCurrentHero()->hero->creatures[1] = new Creature("Griffin", 6, player2);
	player2->getCurrentHero()->hero->creatures[2] = new Creature("Swordsman", 10, player2);
	player2->addNewHero();
	player2->setNextHero();
	player2->getCurrentHero()->hero->creatures[3] = new Creature("Griffin", 15, player2);
	player2->getCurrentHero()->hero->creatures[4] = new Creature("Swordsman", 10, player2);

	/*Player* player3 = gameLogic.addPlayer(true);
	player3->addNewHero();
	player3->getCurrentHero()->hero->creatures[1] = new Creature("Griffin", 6, player3);
	player3->getCurrentHero()->hero->creatures[2] = new Creature("Swordsman", 10, player3);
	player3->addNewHero();
	player3->setNextHero();
	player3->getCurrentHero()->hero->creatures[3] = new Creature("Griffin", 15, player3);
	player3->getCurrentHero()->hero->creatures[4] = new Creature("Swordsman", 10, player3);*/

	gameLogic.aiThread = std::thread(&GameLogic::aiThreadJob, &gameLogic);
	gameLogic.startGame();

	glutMainLoop();
	return 0;
}

void myReshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	viewManager.windowWidth = width;
	viewManager.windowHeight = height;

	viewManager.mapCamera.windowResized(width, height);
	viewManager.topMenuCamera.windowResized(width, height);
	viewManager.rightMenuCamera.windowResized(width, height);
	viewManager.minimapCamera.windowResized(width, height);
	viewManager.promptCamera.windowResized(width, height); // TODO listen to mapcam?
	viewManager.modelCamera.windowResized(width, height);
	viewManager.combatCamera.windowResized(width, height);
	viewManager.buildingCamera.windowResized(width, height);
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		viewManager.troopCamera[i].windowResized(width, height);
	}

	inputManager.windowResized(width, height);
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

void mySpecial(int key, int x, int y) {
	inputManager.processKeySpecial(key, x, y);
}

void myKeyboard(unsigned char key, int x, int y) {
	inputManager.processKeyDown(key, x, y);
}

void myKeyboardUp(unsigned char key, int x, int y) {
	inputManager.processKeyUp(key, x, y);
}

void myPassiveMouseMovement(int x, int y) {
	inputManager.processHover(x, y);
}

void myActiveMouseMovement(int x, int y) {
	inputManager.processDrag(x, y);
}

void myMouse(int button, int state, int x, int y) {
	inputManager.processClick(button, state, x, y);
}