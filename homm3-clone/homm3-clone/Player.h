#pragma once

#include <vector>
#include "Hero.h"

using namespace std;


class Player : public FactionMember{

public:
	Player(int _factionId) : FactionMember(_factionId) {
		selectedHero = -1;
	}

	void addHero(Hero* hero) {
		heroes.push_back(hero);
		if (heroes.size() == 1) {
			selectedHero = 0;
		}
	}

	Hero* setNextHero() {
		if (heroes.size() == 0) {
			return nullptr;
		}

		selectedHero = (selectedHero + 1) % heroes.size();
		return getCurrentHero();
	}

	Hero* setPreviousHero() {
		if (heroes.size() == 0) {
			return nullptr;
		}

		selectedHero = (selectedHero + heroes.size() - 1) % heroes.size();
		return getCurrentHero();
	}

	Hero* setHeroByIndex(int index) {
		if (heroes.size() == 0) {
			return nullptr;
		}

		selectedHero = index % heroes.size();
		return getCurrentHero();
	}

	Hero* getCurrentHero() {
		if (heroes.size() == 0) {
			return nullptr;
		}

		return heroes[selectedHero];
	}

	int getCurrentHeroIndex() {
		return selectedHero;
	}

	void refresh() {
		for (Hero* hero : heroes) {
			hero->refresh();
		}
	}

	vector<Hero*> heroes;

private:
	int selectedHero;
};