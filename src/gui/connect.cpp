#include "connect.h"
#include <IGUIButton.h>
#include <IGUIEditBox.h>
#include <IGUIEnvironment.h>
#include <IGUIFont.h>
#include <IGUIListBox.h>
#include <IGUIStaticText.h>
#include <IVideoDriver.h>
#include <fstream>
#include <vector2d.h>

enum ElementId : int {
	ID_BoxNickname = 101,
	ID_BoxPassword,
	ID_BoxAddress,
	ID_BtnConnect,
	ID_BtnHost,
	ID_ListServers
};

SceneConnect::SceneConnect()
{

}

// -------------- Public members -------------

void SceneConnect::draw()
{
	auto gui = m_gui->guienv;

	auto rect_1 = m_gui->getRect({50, 15}, {-160, -30});

	{
		// Logo
		auto texture = gui->getVideoDriver()->getTexture("assets/logo.png");
		auto dim = texture->getOriginalSize();

		core::vector2di pos = rect_1.getCenter() - core::vector2di(rect_1.getWidth() + dim.Width, dim.Height) / 2;
		gui->addImage(texture, pos, false, 0, -1, L"test test");

		rect_1 += core::vector2di(0, dim.Height);
	}


	// Label column
	rect_1 += core::vector2di(-120 * 2, -30);
	// Editbox column
	auto rect_2 =  [&rect_1]() {
		return rect_1 + core::vector2di(120, -5);
	};

	// Side button
	auto rect_3 =  [&rect_1]() {
		return rect_1 + core::vector2di(120 + 180, -5);
	};

	const core::vector2di VSPACING(0, 50);

	{

		gui->addButton(
			rect_3(), nullptr, ID_BtnHost, L"Host");
	}

	{
		auto text_a = gui->addStaticText(L"Username", rect_1, false, false);
		text_a->setOverrideColor(Gui::COLOR_ON_BG);

		gui->addEditBox(
			nickname.c_str(), rect_2(), true, nullptr, ID_BoxNickname);

		rect_1 += VSPACING;
	}

	{
		auto text_a = gui->addStaticText(L"Password", rect_1, false, false);
		text_a->setOverrideColor(Gui::COLOR_ON_BG);

		auto e = gui->addEditBox(
			password.c_str(), rect_2(), true, nullptr, ID_BoxPassword);
		e->setPasswordBox(true);

		rect_1 += VSPACING;
	}

	{
		auto text_a = gui->addStaticText(L"Address", rect_1, false, false);
		text_a->setOverrideColor(Gui::COLOR_ON_BG);

		core::stringw str;

		gui->addEditBox(
			address.c_str(), rect_2(), true, nullptr, ID_BoxAddress);

		gui->addButton(
			rect_3(), nullptr, ID_BtnConnect, L"Connect");

		rect_1 += VSPACING;
	}

	{
		// Server selector dropdown
		auto text_a = gui->addStaticText(L"Servers", rect_1, false, false);
		text_a->setOverrideColor(Gui::COLOR_ON_BG);

		auto rect = rect_2();
		auto rect_3_tmp = rect_3();
		rect.addInternalPoint(rect_3_tmp.LowerRightCorner + core::vector2di(0, 50));

		auto listbox = gui->addListBox(rect, nullptr, ID_ListServers, true);

		{
			m_index_to_address.clear();

			// Read in the saved servers
			std::ifstream is("client_servers.txt");
			std::string line;
			while (std::getline(is, line)) {
				LoginInfo info;
				utf8_to_wide(info.address, get_next_part(line).c_str());
				utf8_to_wide(info.nickname, get_next_part(line).c_str());
				if (info.nickname.empty())
					continue;

				m_index_to_address.push_back(info);
			}
		}

		// Add to the gui
		for (auto &info : m_index_to_address) {
			std::wstring label = info.nickname + L"  -  " + info.address;

			auto i = listbox->addItem(label.c_str());
			listbox->setItemOverrideColor(i, Gui::COLOR_ON_BG);
			if (info.address == address.c_str())
				listbox->setSelected(i);
		}
	}
}

void SceneConnect::step(float dtime)
{
}

bool SceneConnect::OnEvent(const SEvent &e)
{
	if (e.EventType == EET_GUI_EVENT) {
		switch (e.GUIEvent.EventType) {
			case gui::EGET_BUTTON_CLICKED:
				onSubmit(e.GUIEvent.Caller->getID());
				return true;
			case gui::EGET_LISTBOX_CHANGED:
				if (e.GUIEvent.Caller->getID() == ID_ListServers) {
					gui::IGUIListBox *list = (gui::IGUIListBox *)e.GUIEvent.Caller;

					try {
						auto info = m_index_to_address.at(list->getSelected());
						address = info.address.c_str();
						nickname = info.nickname.c_str();

						m_gui->requestRenew();
					} catch (std::exception &) {
						break;
					}
				}
				break;
			default: break;
		}
	}
	return false;
}

bool SceneConnect::OnEvent(GameEvent &e)
{
	return false;
}


void SceneConnect::onSubmit(int elementid)
{
	auto root = m_gui->guienv->getRootGUIElement();

	nickname = root->getElementFromId(ID_BoxNickname)->getText();
	password = root->getElementFromId(ID_BoxPassword)->getText();
	address = root->getElementFromId(ID_BoxAddress)->getText();

	start_localhost = (elementid == ID_BtnHost);

	m_gui->connect(this);
}

