#pragma once

#include "client/gameevent.h"
#include "core/macros.h"
#include "core/utils.h" // text conversion
#include <IEventReceiver.h>
#include <rect.h>
#include <map>

namespace irr {
	class IrrlichtDevice;

	namespace gui {
		class IGUIEnvironment;
		class IGUIFont;
	}
	namespace scene {
		class ISceneManager;
	}
	namespace video {
		class IVideoDriver;
	}
}

using namespace irr;

class Client;
class ClientStartData;
class Server;

class SceneHandler;
class SceneConnect;
class SceneLobby;
class SceneGameplay;

enum class SceneHandlerType {
	Connect,
	Register,
	Lobby,
	Gameplay,
	CTRL_QUIT,
	CTRL_RENEW
};

extern BlockManager *g_blockmanager;

class Gui : public IEventReceiver, public GameEventHandler {
public:
	Gui();
	~Gui();

	void run();
	void requestShutdown() { m_scenetype_next = SceneHandlerType::CTRL_QUIT; }
	void requestRenew() { m_scenetype_next = SceneHandlerType::CTRL_RENEW; }

	// Global callbacks
	bool OnEvent(const SEvent &event) override;
	bool OnEvent(GameEvent &e) override;

	// Helpers for SceneHandler
	Client *getClient() { return m_client; }
	void showPopupText(const std::string &str);

	// Actions to perform
	void connect(SceneConnect *sc);
	void disconnect();
	void joinWorld(SceneLobby *sc);
	void leaveWorld();

	// GUI utility functions
	core::recti getRect(core::vector2df pos_perc, core::dimension2di size_perc);
	void displaceRect(core::recti &rect, core::vector2df pos_perc);

	// For use in SceneHandler
	scene::ISceneManager *scenemgr = nullptr;
	gui::IGUIEnvironment *guienv = nullptr;
	gui::IGUIFont *font = nullptr;
	video::IVideoDriver *driver = nullptr; // 2D images
	core::dimension2du window_size;

	static constexpr u32 COLOR_ON_BG { 0xFFFFFFFF };
private:
	void registerHandler(SceneHandlerType type, SceneHandler *handler);
	SceneHandler *getHandler(SceneHandlerType type);
	// Only "Gui" is allowed to decide on the next screen. Use one of the actions functions
	inline void setNextScene(SceneHandlerType type) { m_scenetype_next = type; }

	bool m_initialized = false;

	SceneHandlerType m_scenetype_next;
	SceneHandlerType m_scenetype;

	IrrlichtDevice *m_device = nullptr;

	std::map<SceneHandlerType, SceneHandler *> m_handlers;

	bool m_pending_disconnect = false;
	Client *m_client = nullptr;
	Server *m_server = nullptr;

	void drawFPS();

	void drawPopup(float dtime);
	float m_popup_timer = 0;
	core::stringw m_popup_text;
};

class SceneHandler {
public:
	virtual ~SceneHandler() {}

	DISABLE_COPY(SceneHandler)

	virtual void OnOpen() {}
	virtual void OnClose() {};

	virtual void draw() = 0; // GUI updates
	virtual void step(float dtime) = 0;
	virtual bool OnEvent(const SEvent &e) = 0;
	virtual bool OnEvent(GameEvent &e) = 0;

protected:
	friend class Gui;

	SceneHandler() = default;

	Gui *m_gui = nullptr;
};
