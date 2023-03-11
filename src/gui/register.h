#pragma once

#include "gui.h"

class SceneRegister : public SceneHandler {
public:
	SceneRegister() {}

	void draw() override;
	void step(float dtime) override {};
	bool OnEvent(const SEvent &e) override;
	bool OnEvent(GameEvent &e) override;

	core::stringw email;
	core::stringw email_confirm;
};
