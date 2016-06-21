#include "Buildings.h"
#include "GameLogic.h"
#include "ViewManager.h"

Buildings::Dwelling::Dwelling(Creature* _creature)
	: creature(intp(0, 0), _creature, 1){
	available = _creature->growth;
	selected = 0;
	switch (_creature->level) {
		case 1:
			type = BuildingType::DWELL_ONE;
			break;
		case 2:
			type = BuildingType::DWELL_TWO;
			break;
		case 3:
			type = BuildingType::DWELL_THREE;
			break;
		case 4:
			type = BuildingType::DWELL_FOUR;
			break;
		case 5:
			type = BuildingType::DWELL_FIVE;
			break;
		case 6:
			type = BuildingType::DWELL_SIX;
			break;
		case 7:
			type = BuildingType::DWELL_SEVEN;
			break;
	}

	areas.push_back(Rect(floatp(0.1, 0.3), floatp(0.2, 0.2)));
	areas.push_back(Rect(floatp(0.7, 0.3), floatp(0.2, 0.2)));
	areas.push_back(Rect(floatp(0.4, 0.1), floatp(0.2, 0.15)));
}

void Buildings::Dwelling::interact() {
	// TODO show the dwelling recruitment UI
}

void Buildings::Dwelling::draw(intp pos, intp dim) {
	// TEMP

	glColor3f(0.5, 0.5, 0.5);
	glRectf(pos.x + areas[0].pos.x * dim.x, pos.y + areas[0].pos.y * dim.y,
		pos.x + (areas[0].pos.x + areas[0].dim.x) * dim.x, pos.y + (areas[0].pos.y + areas[0].dim.y) * dim.y);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[0].pos.x * dim.x + 5, pos.y + areas[0].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "-");

	glColor3f(0.5, 0.5, 0.5);
	glRectf(pos.x + areas[1].pos.x * dim.x, pos.y + areas[1].pos.y * dim.y,
		pos.x + (areas[1].pos.x + areas[1].dim.x) * dim.x, pos.y + (areas[1].pos.y + areas[1].dim.y) * dim.y);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[1].pos.x * dim.x + 5, pos.y + areas[1].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "+");

	glColor3f(0.5, 0.5, 0.5);
	glRectf(pos.x + areas[2].pos.x * dim.x, pos.y + areas[2].pos.y * dim.y,
		pos.x + (areas[2].pos.x + areas[2].dim.x) * dim.x, pos.y + (areas[2].pos.y + areas[2].dim.y) * dim.y);
	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + areas[2].pos.x * dim.x + 5, pos.y + areas[2].pos.y * dim.y + 5);
	drawText(GLUT_BITMAP_HELVETICA_12, "Buy");

	glColor3f(0, 0, 0);
	glRasterPos2d(pos.x + 0.4 * dim.x , pos.y + 0.7 * dim.y);
	drawText(GLUT_BITMAP_HELVETICA_12, creature.hero->creatures[0]->plural);

	glRasterPos2d(pos.x + 0.4 * dim.x, pos.y + 0.3 * dim.y);
	drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(selected) + " / " + std::to_string(available));
}

void Buildings::Dwelling::areaClicked(int index) {
	if (index == 0) {
		selected--;
		selected = mmax(selected, 0);
	}
	else if (index == 1) {
		selected++;
		selected = mmin(selected, getMaxAvailable(GameLogic::instance().getCurrentPlayer()->wallet[GOLD]));
	}
	else if (index == 2) {
		ViewManager &viewManager = ViewManager::instance();
		GameLogic &gameLogic = GameLogic::instance();
		if (viewManager.isActive[CamId::BUILDING]) {
			purchase(selected, gameLogic.getCurrentPlayer(), gameLogic.map->getObject(viewManager.displayedBuilding)->hero);
		}
		selected = mmin(1, getMaxAvailable(GameLogic::instance().getCurrentPlayer()->wallet[GOLD]));
	}
}

void Buildings::Dwelling::weeklyRefresh() {
	available = creature.hero->creatures[0]->growth;
}

int Buildings::Dwelling::getMaxAvailable(int budget) {
	return mmin(available, budget / creature.hero->creatures[0]->cost);
}

Creature * Buildings::Dwelling::getCreature() {
	return creature.hero->creatures[0];
}

Buildings::BuyStatus Buildings::Dwelling::purchase(int amount, Player * player, Hero * hero) {
	if (player->getFactionId() != GameLogic::instance().getCurrentPlayer()->getFactionId()) {
		return Buildings::BuyStatus::NOT_ACTIVE_PLAYER;
	}

	if (amount < 1) {
		return Buildings::BuyStatus::OUT_OF_STOCK; // TEMP
	}

	if (amount > available) {
		return Buildings::BuyStatus::OUT_OF_STOCK;
	}

	if (getMaxAvailable(player->wallet[GOLD]) < amount) {
		return Buildings::BuyStatus::MISSING_RES;
	}

	int freeSlot = -1;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		Creature* stack = hero->creatures[i];
		if (stack == nullptr || stack->count == 0 || stack->name == creature.hero->creatures[0]->name) {
			freeSlot = i;
			break;
		}
	}

	if (freeSlot == -1) {
		return Buildings::BuyStatus::MISSING_SPACE;
	}

	available -= amount;
	player->wallet[GOLD] -= amount * creature.hero->creatures[0]->cost;
	hero->creatures[freeSlot] = new Creature(creature.hero->creatures[0], amount, player->getFactionId());

	return OK;
}

std::string Buildings::toString(BuildingType type) {
	switch (type) {
		case HALL:
			return "Hall";
		case DWELL_ONE:
			return "DwellOne";
		case DWELL_TWO:
			return "DwellTwo";
		case DWELL_THREE:
			return "DwellThree";
		case DWELL_FOUR:
			return "DwellFour";
		case DWELL_FIVE:
			return "DwellFive";
		case DWELL_SIX:
			return "DwellSix";
		case DWELL_SEVEN:
			return "DwellSeven";
		default:
			return "UNKNOWN";
	}
}
