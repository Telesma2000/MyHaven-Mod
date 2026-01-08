#include "World.hpp"

#include "World/PedSpawner.hpp"
#include "World/Shows.hpp"
#include "World/Train.hpp"
#include "World/VehicleSpawner.hpp"
#include "World/Weather.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/HotkeySystem.hpp"
#include "core/commands/LoopedCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Pools.hpp"

#include <game/rdr/Natives.hpp>
#include <rage/fwBasePool.hpp>
#include <rage/pools.hpp>


namespace YimMenu::Submenus
{
	void DisplayCurrentDate()
	{
		auto hours      = CLOCK::GET_CLOCK_HOURS();
		auto minutes    = CLOCK::GET_CLOCK_MINUTES();
		auto seconds    = CLOCK::GET_CLOCK_SECONDS();
		auto dayOfWeek  = CLOCK::GET_CLOCK_DAY_OF_WEEK();
		auto dayOfMonth = CLOCK::GET_CLOCK_DAY_OF_MONTH();
		auto month      = CLOCK::GET_CLOCK_MONTH();
		auto year       = CLOCK::GET_CLOCK_YEAR();

		const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
		const char* months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

		std::string dayOfWeekStr = daysOfWeek[dayOfWeek];
		std::string monthStr     = months[month];

		std::string dateString = std::format("{}, {} {}, {}", dayOfWeekStr, monthStr, dayOfMonth, year);
		std::string time       = std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);

		ImGui::Text(u8"วันที่: %s", dateString.c_str());
		ImGui::Text(u8"เวลา: %s", time.c_str());
	}

	World::World() :
	    Submenu::Submenu("World")
	{
		auto main    = std::make_shared<Category>(u8"หน้าหลัก");
		auto weather = std::make_shared<Category>(u8"สภาพอากาศ");
		auto shows   = std::make_shared<Category>(u8"การแสดง");
		auto time    = std::make_shared<Category>(u8"เวลา");


		time->AddItem(std::make_shared<ImGuiItem>([] {
			static int hour, minute, second;
			static bool freeze;

			DisplayCurrentDate();

			ImGui::SliderInt(u8"ชั่วโมง", &hour, 0, 23);
			ImGui::SliderInt(u8"นาที", &minute, 0, 59);
			ImGui::SliderInt(u8"วินาที", &second, 0, 59);
			ImGui::Checkbox(u8"หยุดเวลา", &freeze);
			if (ImGui::Button(u8"เปลี่ยนเวลา"))
			{
				FiberPool::Push([] {
					ChangeTime(hour, minute, second, 0, freeze);
				});
			}
			if (ImGui::Button(u8"คืนค่า"))
			{
				FiberPool::Push([] {
					NETWORK::_NETWORK_CLEAR_CLOCK_OVERRIDE_OVERTIME(0);
				});
			}
		}));


		weather->AddItem(std::make_shared<ImGuiItem>([] {
			static const char* current_weather = WeatherTypes[0]; // Default weather
			if (ImGui::BeginCombo(u8"สภาพอากาศ", current_weather))
			{
				for (auto& weather_type : WeatherTypes)
				{
					bool is_selected = (current_weather == weather_type);
					if (ImGui::Selectable(weather_type, is_selected))
					{
						current_weather = weather_type;
						FiberPool::Push([=] {
							ChangeWeather(weather_type);
						});
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button(u8"คืนค่า"))
			{
				FiberPool::Push([] {
					MISC::CLEAR_OVERRIDE_WEATHER();
				});
			}
		}));


		auto spawners            = std::make_shared<Category>(u8"เสกของ");
		auto pedSpawnerGroup     = std::make_shared<Group>(u8"เสกคน (Ped)");
		auto vehicleSpawnerGroup = std::make_shared<Group>(u8"เสกยานพาหนะ");
		auto trainSpawnerGroup   = std::make_shared<Group>(u8"เสกรถไฟ");
		auto buildModeGroup      = std::make_shared<Group>(u8"โหมดสร้าง (สไตล์ Sims)"); // Added Group

		// Build Mode Integration
		buildModeGroup->AddItem(std::make_shared<BoolCommandItem>("buildmodeactive"_J, u8"เปิดโหมดสร้าง"));
		buildModeGroup->AddItem(std::make_shared<ConditionalItem>("buildmodeactive"_J, std::make_shared<BoolCommandItem>("buildmodecamera"_J, u8"กล้องโหมดสร้าง")));
		buildModeGroup->AddItem(std::make_shared<ConditionalItem>("buildmodeactive"_J, std::make_shared<ListCommandItem>("buildcategory"_J, u8"หมวดหมู่")));
		buildModeGroup->AddItem(std::make_shared<ConditionalItem>("buildmodeactive"_J, std::make_shared<CommandItem>("spawnbuildobject"_J, u8"เสกวัตถุ")));
		buildModeGroup->AddItem(std::make_shared<ConditionalItem>("buildmodeactive"_J, std::make_shared<CommandItem>("clearallobjects"_J, u8"ลบวัตถุทั้งหมด")));
		buildModeGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"สร้างแคมป์ของคุณเอง! ใช้ WASD เพื่อเลื่อนกล้อง, ปุ่มลูกศรเพื่อหมุนวัตถุ");
		}));

		pedSpawnerGroup->AddItem(std::make_shared<ImGuiItem>([] {
			RenderPedSpawnerMenu();
		}));

		vehicleSpawnerGroup->AddItem(std::make_shared<ImGuiItem>([] {
			RenderVehicleSpawnerMenu();
		}));

		trainSpawnerGroup->AddItem(std::make_shared<ImGuiItem>([] {
			RenderTrainsMenu();
		}));

		spawners->AddItem(buildModeGroup); // Add Build Mode
		spawners->AddItem(pedSpawnerGroup);
		spawners->AddItem(vehicleSpawnerGroup);
		spawners->AddItem(trainSpawnerGroup);

		auto poolCounter = std::make_shared<ImGuiItem>([] {
			if (GetPedPool())
				ImGui::Text("%s",
				    std::format(u8"คน (Peds): {}/{}", GetPedPool()->m_Size - GetPedPool()->GetNumFreeSlots(), GetPedPool()->m_Size)
				        .data());
			if (GetVehiclePool())
				ImGui::Text("%s",
				    std::format(u8"ยานพาหนะ: {}/{}",
				        GetVehiclePool()->m_Size - GetVehiclePool()->GetNumFreeSlots(),
				        GetVehiclePool()->m_Size)
				        .data());
			if (GetObjectPool())
				ImGui::Text("%s",
				    std::format(u8"วัตถุ: {}/{}",
				        GetObjectPool()->m_Size - GetObjectPool()->GetNumFreeSlots(),
				        GetObjectPool()->m_Size)
				        .data());
		});

		auto killPeds = std::make_shared<Group>(u8"ฆ่า", 1);
		killPeds->AddItem(std::make_shared<CommandItem>("killallpeds"_J, u8"ฆ่าคนทั้งหมด"));
		killPeds->AddItem(std::make_shared<CommandItem>("killallenemies"_J, u8"ฆ่าศัตรูทั้งหมด"));
		auto deleteOpts = std::make_shared<Group>(u8"ลบ", 1);
		deleteOpts->AddItem(std::make_shared<CommandItem>("delpeds"_J, u8"ลบคน"));
		deleteOpts->AddItem(std::make_shared<CommandItem>("delvehs"_J, u8"ลบยานพาหนะ"));
		deleteOpts->AddItem(std::make_shared<CommandItem>("delobjs"_J, u8"ลบวัตถุ"));
		auto bringOpts = std::make_shared<Group>(u8"ดึงมา", 1);
		bringOpts->AddItem(std::make_shared<CommandItem>("bringpeds"_J, u8"ดึงคนมา"));
		bringOpts->AddItem(std::make_shared<CommandItem>("bringvehs"_J, u8"ดึงยานพาหนะมา"));
		bringOpts->AddItem(std::make_shared<CommandItem>("bringobjs"_J, u8"ดึงวัตถุมา"));
		auto minigames = std::make_shared<Group>(u8"มินิเกม", 1);
		minigames->AddItem(std::make_shared<BoolCommandItem>("undeadnightmare"_J, u8"Undead Nightmare"));
		minigames->AddItem(std::make_shared<ConditionalItem>("undeadnightmare"_J, std::make_shared<BoolCommandItem>("zombieslogging"_J, u8"บันทึกข้อมูลซอมบี้")));
		minigames->AddItem(std::make_shared<ConditionalItem>("undeadnightmare"_J, std::make_shared<BoolCommandItem>("hardmode"_J, u8"โหมดยาก")));
		auto misc = std::make_shared<Group>(u8"อื่นๆ");
		misc->AddItem(std::make_shared<BoolCommandItem>("disableguardzones"_J, u8"ปิดเขตหวงห้าม"));
		auto eventOverride = std::make_shared<Group>("", 1);
		eventOverride->AddItem(std::make_shared<BoolCommandItem>("eventoverrideenabled"_J, u8"เปิดทับอีเวนต์"));
		eventOverride->AddItem(std::make_shared<ConditionalItem>("eventoverrideenabled"_J, std::make_shared<ListCommandItem>("eventoverride"_J, u8"เลือกอีเวนต์")));
		misc->AddItem(std::move(eventOverride));
		misc->AddItem(std::make_shared<CommandItem>("mapeditor"_J, u8"ตัวแก้ไขแผนที่"));

		main->AddItem(std::move(poolCounter));
		main->AddItem(std::move(killPeds));
		main->AddItem(std::move(deleteOpts));
		main->AddItem(std::move(bringOpts));
		main->AddItem(std::move(minigames));
		main->AddItem(std::move(misc));


		shows->AddItem(std::make_shared<ImGuiItem>([] {
			RenderShowsMenu();
		}));


		AddCategory(std::move(main));
		AddCategory(std::move(weather));
		AddCategory(std::move(spawners));
		AddCategory(std::move(shows));
		AddCategory(std::move(time));
	}

}