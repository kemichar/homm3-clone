#pragma once

#include <string>

using namespace std;

class ViewManager {

public:
	static void showPrompt(string _promptText) {
		isPromptUp = true;
		promptText = _promptText;

		testPromptTimer = 5000;
	}

	static string promptText;
	static bool isPromptUp;
	static float testPromptTimer;

};
