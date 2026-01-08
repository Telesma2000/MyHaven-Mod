#include "Trolling.hpp"

namespace YimMenu::Submenus
{
	std::shared_ptr<Category> BuildTrollingMenu()
	{
		auto menu = std::make_shared<Category>(u8"กลั่นแกล้ง");

		auto attachments = std::make_shared<Group>(u8"การเกาะติด");

		menu->AddItem(std::make_shared<PlayerCommandItem>("cageplayersmall"_J, u8"กรงขัง (เล็ก)"));
		menu->AddItem(std::make_shared<PlayerCommandItem>("cageplayerlarge"_J, u8"กรงขัง (ใหญ่)"));
		menu->AddItem(std::make_shared<PlayerCommandItem>("cageplayercircus"_J, u8"กรงขัง (ละครสัตว์)"));
		attachments->AddItem(std::make_shared<PlayerCommandItem>("spank"_J, u8"ตีตูด"));
		attachments->AddItem(std::make_shared<PlayerCommandItem>("rideonshoulders"_J, u8"ขี่คอ"));
		attachments->AddItem(std::make_shared<PlayerCommandItem>("touchplayer"_J, u8"แตะตัว"));
		attachments->AddItem(std::make_shared<PlayerCommandItem>("slap"_J, u8"ตบ"));
		attachments->AddItem(std::make_shared<CommandItem>("cancelattachment"_J, u8"ยกเลิกการเกาะติด"));

		menu->AddItem(attachments);

		return menu;
	}
}