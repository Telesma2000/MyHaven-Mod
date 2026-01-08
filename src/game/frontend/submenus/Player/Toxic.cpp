#include "Toxic.hpp"

namespace YimMenu::Submenus
{
	std::shared_ptr<Category> BuildToxicMenu()
	{
		auto menu = std::make_shared<Category>(u8"เกรียน (Toxic)");

		auto general = std::make_shared<Group>(u8"ทั่วไป");
		general->AddItem(std::make_shared<PlayerCommandItem>("kill"_J, u8"ฆ่า"));
		general->AddItem(std::make_shared<PlayerCommandItem>("explode"_J, u8"ระเบิด"));
		general->AddItem(std::make_shared<PlayerCommandItem>("lightning"_J, u8"ฟ้าผ่า"));
		general->AddItem(std::make_shared<PlayerCommandItem>("defensive"_J, u8"บังคับโหมดป้องกัน"));
		general->AddItem(std::make_shared<PlayerCommandItem>("offensive"_J, u8"บังคับโหมดโจมตี"));
		general->AddItem(std::make_shared<PlayerCommandItem>("remotebolas"_J, u8"ปา Bolas"));

		auto events = std::make_shared<Group>(u8"อีเวนต์");
		events->AddItem(std::make_shared<PlayerCommandItem>("maxhonor"_J, u8"เกียรติยศสูงสุด"));
		events->AddItem(std::make_shared<PlayerCommandItem>("minhonor"_J, u8"เกียรติยศต่ำสุด"));
		events->AddItem(std::make_shared<PlayerCommandItem>("startparlay"_J, u8"เริ่ม Parlay"));
		events->AddItem(std::make_shared<PlayerCommandItem>("endparlay"_J, u8"จบ Parlay"));
		events->AddItem(std::make_shared<PlayerCommandItem>("increasebounty"_J, u8"เพิ่มค่าหัว"));
		events->AddItem(std::make_shared<PlayerCommandItem>("sendticker"_J, u8"ส่งข้อความวิ่ง"));
		events->AddItem(std::make_shared<ListCommandItem>("tickermessage"_J, u8"ข้อความ"));
		events->AddItem(std::make_shared<PlayerCommandItem>("sendstablemountevent"_J, u8"ส่งอีเวนต์คอกม้า"));
		events->AddItem(std::make_shared<ListCommandItem>("mountinstance"_J, "Instance"));
		events->AddItem(std::make_shared<ListCommandItem>("stablemountevent"_J, "Event"));

		auto mount = std::make_shared<Group>(u8"ม้า");
		mount->AddItem(std::make_shared<PlayerCommandItem>("kickhorse"_J, u8"เตะม้า"));
		mount->AddItem(std::make_shared<PlayerCommandItem>("deletehorse"_J, u8"ลบม้า"));

		auto vehicle = std::make_shared<Group>(u8"ยานพาหนะ");
		vehicle->AddItem(std::make_shared<PlayerCommandItem>("deletevehicle"_J, u8"ลบยานพาหนะ"));

		menu->AddItem(general);
		menu->AddItem(events);
		menu->AddItem(mount);
		menu->AddItem(vehicle);

		return menu;
	}
}