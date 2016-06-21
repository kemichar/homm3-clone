#pragma once

#include "UIClickableContainer.h"
#include "MOCreature.h"
#include "Player.h"
#include "Hero.h"

namespace Buildings {

	enum BuyStatus {
		OK, MISSING_RES, OUT_OF_STOCK, MISSING_SPACE, NOT_ACTIVE_PLAYER
	};

	class Building : public UIClickableContainer{
		public:
			virtual void interact() = 0;

			BuildingType type;
	};

	class Dwelling : public Building {
		public:
			Dwelling(Creature* _creature);

			virtual void interact() override;
			virtual void draw(intp pos, intp dim) override;
			virtual void areaClicked(int index) override;

			void weeklyRefresh();
			int getMaxAvailable(int budget);
			Creature* getCreature();
			BuyStatus purchase(int amount, Player* player, Hero* hero);

			MOCreature creature;
			int available;
			int selected;
	};

	std::string toString(BuildingType type);

}