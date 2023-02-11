#pragma once

#include "core/types.h"
#include <IEventReceiver.h>
#include <rect.h>
#include <vector>

namespace irr {
	namespace gui {
		class IGUIEnvironment;
	}
}

class SceneBlockSelector : public IEventReceiver {
public:
	SceneBlockSelector(gui::IGUIEnvironment *gui);
	void setHotbarPos(const core::position2di &pos) { m_hotbar_pos = pos; }

	void draw();
	void step(float dtime);
	bool OnEvent(const SEvent &e) override;

	bid_t getSelectedBid() const { return m_selected_bid; }

private:
	bool drawBlockButton(bid_t bid, const core::recti &rect, gui::IGUIElement *parent, int id);
	void drawBlockSelector();

	gui::IGUIEnvironment *m_gui = nullptr;

	std::vector<bid_t> m_hotbar_ids;
	core::position2di m_hotbar_pos;
	bid_t m_selected_bid = 0;

	bool m_show_selector = false;
	bid_t m_dragged_bid = -1;
	gui::IGUIElement *m_showmore = nullptr;
};
