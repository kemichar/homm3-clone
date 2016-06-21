#pragma once

/*
struct Input {
	Input();
	Input(CamId _cam, intp _pos);
	CamId cam;
	intp pos;
};*/

class InputManager {
public:
	void windowResized(int width, int height);
	void processClick(int button, int state, int x, int y);
	void processHover(int x, int y);
	void processDrag(int x, int y);
	void processKeyDown(unsigned char key, int x, int y);
	void processKeyUp(unsigned char key, int x, int y);
	void processKeySpecial(int key, int x, int y);

	// singleton things
	static InputManager& instance();
	InputManager(InputManager const&) = delete;
	void operator = (InputManager const &) = delete;

private:
	void processAdventureClick(int button, int state, int x, int y);
	void processTopMenuClick(int button, int state, int x, int y);
	void processMinimapClick(int button, int state, int x, int y);
	void processCombatClick(int button, int state, int x, int y);
	void processBuildingClick(int button, int state, int x, int y);
	void processMinimapDrag(int x, int y);

	int windowWidth;
	int windowHeight;

	bool minimapNavigation;

	// singleton things
	InputManager();
};