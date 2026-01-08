#include "Kick.hpp"
#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Submenus
{
	std::shared_ptr<Category> BuildKickMenu()
	{ 
		auto menu = std::make_shared<Category>(u8"เตะออก");

		auto kicks   = std::make_shared<Group>(u8"ตัวเลือกการเตะ");

		kicks->AddItem(std::make_shared<PlayerCommandItem>("splitkick"_J, u8"เตะแยกเซสชั่น (Split)"));
		kicks->AddItem(std::make_shared<PlayerCommandItem>("popkick"_J, u8"เตะ (Population)"));
		kicks->AddItem(std::make_shared<PlayerCommandItem>("icekick"_J, u8"เตะ (Ice)"));

		menu->AddItem(std::move(kicks));

		return menu;
	}
}