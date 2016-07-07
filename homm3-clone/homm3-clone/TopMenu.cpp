#include "TopMenu.h"
#include "GameLogic.h"
#include "ViewManager.h"
#include "Constants.h"
#include "Utility.h"

TopMenu::TopMenu() {
	areas.push_back(Rect(floatp(0.025, 0.4), floatp(0.025, 0.5)));
	areas.push_back(Rect(floatp(0.1, 0.4), floatp(0.06, 0.5)));
	areas.push_back(Rect(floatp(0.2, 0.4), floatp(0.06, 0.5)));
}

void TopMenu::draw(intp pos, intp dim) {
	// TEMP
	ViewManager &viewManager = ViewManager::instance();
	lastDrawPos = pos + intp(viewManager.topMenuCamera.getViewport()[0], viewManager.topMenuCamera.getViewport()[1]);
	lastDrawDim = dim;

	glColor3f(0, 0, 0);
	glRectf(pos.x + areas[0].pos.x * dim.x, pos.y + areas[0].pos.y * dim.y,
		pos.x + (areas[0].pos.x + areas[0].dim.x) * dim.x, pos.y + (areas[0].pos.y + areas[0].dim.y) * dim.y);
	glColor3f(0.7f, 0.7f, 0.7f);
	glRectf(pos.x + areas[0].pos.x * dim.x + 1, pos.y + areas[0].pos.y * dim.y + 1,
		pos.x + (areas[0].pos.x + areas[0].dim.x) * dim.x - 2, pos.y + (areas[0].pos.y + areas[0].dim.y) * dim.y - 2);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[0].pos.x * dim.x + 5, pos.y + areas[0].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "?");

	glColor3f(0, 0, 0);
	glRectf(pos.x + areas[1].pos.x * dim.x, pos.y + areas[1].pos.y * dim.y,
		pos.x + (areas[1].pos.x + areas[1].dim.x) * dim.x, pos.y + (areas[1].pos.y + areas[1].dim.y) * dim.y);
	glColor3f(0.7f, 0.7f, 0.7f);
	glRectf(pos.x + areas[1].pos.x * dim.x + 1, pos.y + areas[1].pos.y * dim.y + 1,
		pos.x + (areas[1].pos.x + areas[1].dim.x) * dim.x - 2, pos.y + (areas[1].pos.y + areas[1].dim.y) * dim.y - 2);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[1].pos.x * dim.x + 5, pos.y + areas[1].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "Quit Game");

	glColor3f(0, 0, 0);
	glRectf(pos.x + areas[2].pos.x * dim.x, pos.y + areas[2].pos.y * dim.y,
		pos.x + (areas[2].pos.x + areas[2].dim.x) * dim.x, pos.y + (areas[2].pos.y + areas[2].dim.y) * dim.y);
	glColor3f(0.7f, 0.7f, 0.7f);
	glRectf(pos.x + areas[2].pos.x * dim.x + 1, pos.y + areas[2].pos.y * dim.y + 1,
		pos.x + (areas[2].pos.x + areas[2].dim.x) * dim.x - 2, pos.y + (areas[2].pos.y + areas[2].dim.y) * dim.y - 2);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[2].pos.x * dim.x + 5, pos.y + areas[2].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "End Turn");
}

void TopMenu::areaClicked(int index) {
	if (index == 0) {
	}
	else if (index == 1) {
		GameLogic::instance().quitGame();
	}
	else if (index == 2) {
		GameLogic::instance().endTurn();
	}
}

void TopMenu::areaHovered(int index) {
	if (index == 0) {
		ViewManager &viewManager = ViewManager::instance();
		viewManager.showTooltip(
			lastDrawPos + intp(lastDrawDim.x * areas[0].pos.x, lastDrawDim.y * areas[0].pos.y),
			splitLines(DEV_HOTKEYS_TOOLTIP),
			&viewManager.totalCamera,
			2000
		);
	}
	else if (index == 1) {
	}
	else if (index == 2) {
	}
}
