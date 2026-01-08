#include "Network.hpp"

#include "Network/Voice.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/HotkeySystem.hpp"
#include "core/commands/LoopedCommand.hpp"
#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/PlayerDatabase.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/pointers/Pointers.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "util/Chat.hpp"
#include "util/Storage/Spoofing.hpp"
#include "util/teleport.hpp"
#include "Self.hpp"
#include "game/rdr/Packet.hpp"

#include <map>
#include <network/rlGamerHandle.hpp>
#include <network/CNetworkScSession.hpp>
#include <network/snConnectToPeerTask.hpp>
#include <rage/rlTaskStatus.hpp>
#include <network/CNetworkPlayerMgr.hpp>
#include <network/netEncryption.hpp>
#include <ranges>
#include <string>

namespace rage
{
	class rlQueryPresenceAttributesContext
	{
	public:
		char m_presence_attribute_key[64]; //0x0000
		union {
			char m_presence_attribute_string_value[256]; //0x0040
			uint64_t m_presence_attribute_int_value;
		};
		uint32_t m_presence_attibute_type; //0x0140
		char pad_0144[4];                  //0x0144
	};                                     //Size: 0x0148
	static_assert(sizeof(rlQueryPresenceAttributesContext) == 0x148);

	struct rlScTaskStatus
	{
		void* pad  = 0;
		int status = 0;
		int unk    = 0;
	};
}

namespace YimMenu::Submenus
{
	std::shared_ptr<persistent_player> current_player;
	static char search[64];
	static char name_buf[32];
	static char new_player_name_buf[32];
	static uint64_t new_player_rid;
	static bool show_player_editor = false;
	static bool show_new_player    = true;

	void draw_player_db_entry(std::shared_ptr<persistent_player> player, const std::string& lower_search)
	{
		std::string name = player->name;
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if (lower_search.empty() || name.find(lower_search) != std::string::npos)
		{
			ImGui::PushID(player->rid);

			if (ImGui::Selectable(player->name.c_str(), player == g_PlayerDatabase->GetSelected()))
			{
				g_PlayerDatabase->SetSelected(player);
				current_player = player;
				strncpy(name_buf, current_player->name.data(), sizeof(name_buf));
				show_new_player    = false;
				show_player_editor = true;
			}

			ImGui::PopID();
		}
	}

	Network::Network() :
	    Submenu::Submenu("Network")
	{
		// TODO: this needs a rework
		auto session              = std::make_shared<Category>(u8"เซสชั่น");
		auto spoofing             = std::make_shared<Category>(u8"การปลอมแปลง");
		auto database             = std::make_shared<Category>(u8"ฐานข้อมูลผู้เล่น");
		auto sessionSwitcherGroup = std::make_shared<Group>(u8"เปลี่ยนเซสชั่น");
		auto teleportGroup        = std::make_shared<Group>(u8"วาร์ป");
		auto miscGroup            = std::make_shared<Group>(u8"อื่นๆ");

		sessionSwitcherGroup->AddItem(std::make_shared<Vector3CommandItem>("newsessionpos"_J, u8"ตำแหน่งเซสชั่นใหม่"));
		sessionSwitcherGroup->AddItem(std::make_shared<BoolCommandItem>("newsessionposse"_J, u8"เซสชั่นใหม่ (Posse)"));
		sessionSwitcherGroup->AddItem(std::make_shared<CommandItem>("newsession"_J, u8"หาห้องใหม่"));

		teleportGroup->AddItem(std::make_shared<CommandItem>("bringall"_J, u8"ดึงทุกคนมา"));
		teleportGroup->AddItem(std::make_shared<CommandItem>("tpalltowaypoint"_J, u8"วาร์ปทุกคนไป Waypoint"));
		// Removed: tpalltojail (griefing feature)

		// Removed entire Toxic group (explodeall, honor manipulation)

		miscGroup->AddItem(std::make_shared<BoolCommandItem>("revealall"_J, u8"เปิดเผยผู้เล่นทั้งหมด"));
		miscGroup->AddItem(std::make_shared<BoolCommandItem>("blockalltelemetry"_J, u8"บล็อค Telemetry ทั้งหมด"));
		miscGroup->AddItem(std::make_shared<BoolCommandItem>("locklobby"_J, u8"ล็อคล็อบบี้"));

		session->AddItem(sessionSwitcherGroup);
		session->AddItem(teleportGroup);
		// Removed: toxicGroup
		session->AddItem(miscGroup);

		spoofing->AddItem(std::make_shared<BoolCommandItem>("hidegod"_J, u8"ซ่อนสถานะอมตะ"));
		spoofing->AddItem(std::make_shared<BoolCommandItem>("hidespectate"_J, u8"ซ่อนการส่อง"));

		database->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::SetNextItemWidth(300.f);
			ImGui::PushID(3);
			ImGui::InputText(u8"ค้นหาชื่อผู้เล่น", search, sizeof(search));
			ImGui::PopID();

			if (ImGui::BeginListBox("###players", {180, static_cast<float>(*Pointers.ScreenResY - 400 - 38 * 4)}))
			{
				auto& item_arr = g_PlayerDatabase->GetAllPlayers();
				if (item_arr.size() > 0)
				{
					std::string lower_search = search;
					std::transform(lower_search.begin(), lower_search.end(), lower_search.begin(), ::tolower);

					for (auto& player : item_arr)
					{
						draw_player_db_entry(player.second, lower_search);
					}
				}
				else
				{
					ImGui::Text(u8"ไม่พบผู้เล่น!");
				}

				ImGui::EndListBox();
			}
			if (auto selected = g_PlayerDatabase->GetSelected() && show_player_editor)
			{
				ImGui::PushID(1);
				ImGui::SameLine();
				if (ImGui::BeginChild("###selected_player", {500, static_cast<float>(*Pointers.ScreenResY - 388 - 38 * 4)}, false, ImGuiWindowFlags_NoBackground))
				{
					if (ImGui::InputText(u8"ชื่อ", name_buf, sizeof(name_buf)))
					{
						current_player->name = name_buf;
						g_PlayerDatabase->Save();
					}

					if (ImGui::InputScalar("RID", ImGuiDataType_S64, &current_player->rid)
					    || ImGui::Checkbox(u8"เป็น Modder", &current_player->is_modder)
					    || ImGui::Checkbox(u8"เชื่อถือ", &current_player->trust)
					    || ImGui::Checkbox(u8"บล็อคการเข้าร่วม", &current_player->block_join))
					{
						g_PlayerDatabase->Save();
					}

					if (!current_player->infractions.empty())
					{
						ImGui::Text(u8"การละเมิด");

						std::unordered_map<int, int /*count*/> count_map;

						for (const auto& item : current_player->infractions)
						{
							count_map[item]++;
						}

						for (const auto& pair : count_map)
						{
							// TODO: find a better way to do this
							ImGui::BulletText(std::string(g_PlayerDatabase->ConvertDetectionToDescription(Detection(pair.first)))
							                      .append(" - ")
							                      .append("x")
							                      .append(std::to_string(pair.second))
							                      .c_str());
						}
					}

					if (ImGui::Button(u8"ลบผู้เล่น"))
					{
						g_PlayerDatabase->RemoveRID(current_player->rid);
					}

					if (ImGui::Button(u8"ซ่อนตัวแก้ไข"))
					{
						show_player_editor = false;
						show_new_player    = true;
					}
					ImGui::PopID();
				}
			}
			if (show_new_player)
			{
				ImGui::PushID(2);
				ImGui::NewLine();
				ImGui::InputText(u8"ชื่อผู้เล่น", new_player_name_buf, sizeof(new_player_name_buf));
				ImGui::InputScalar("RID", ImGuiDataType_U64, &new_player_rid);
				if (ImGui::Button(u8"เพิ่ม"))
				{
					current_player = g_PlayerDatabase->GetOrCreatePlayer(new_player_rid, new_player_name_buf);
					memset(new_player_name_buf, 0, sizeof(new_player_name_buf));
				}
				ImGui::PopID();
			}
		}));

		static std::string nameBuf, colorBuf = "";
		static const char* iconBuf = "";
		auto infoSpoofingGroup    = std::make_shared<Group>(u8"ปลอมแปลงข้อมูล");
		auto blipSpoofingGroup    = std::make_shared<Group>(u8"ปลอมแปลง Blip");
		auto sessionSpoofingGroup = std::make_shared<Group>(u8"ปลอมแปลงเซสชั่น");

		infoSpoofingGroup->AddItem(std::make_shared<ImGuiItem>([=] {
			static std::map<std::string, std::string> colors = {{"", "None"}, {"~e~", "Red"}, {"~f~", "Off White"}, {"~p~", "White"}, {"~o~", "Yellow"}, {"~q~", "Pure White"}, {"~d~", "Orange"}, {"~m~", "Light Grey"}, {"~t~", "Grey"}, {"~v~", "Black"}, {"~pa~", "Blue"}, {"~t1~", "Purple"}, {"~t2~", "Orange"}, {"~t3~", "Teal"}, {"~t4~", "Light Yellow"}, {"~t5~", "Pink"}, {"~t6~", "Green"}, {"~t7~", "Dark Blue"}};
			static std::map<const char*, std::string> icons = {{"", "None"}, {(const char*)u8"\u2211", "Rockstar Icon"}};
			ImGui::Text(u8"ข้อมูลที่ปลอมแปลงจะไม่แสดงในเครื่องของคุณ จะเห็นได้เฉพาะเมื่อเข้าห้องใหม่\nหรือเมื่อมีคนอื่นเข้ามาหาคุณ");

			ImGui::Text(u8"ชื่อ");
			ImGui::Checkbox(u8"ปลอมชื่อ", &g_SpoofingStorage.spoofName);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Spoof your name");

			if (ImGui::BeginCombo(u8"สีนำหน้า", colors[colorBuf].c_str()))
			{
				for (auto& [code, translation] : colors)
				{
					if (ImGui::Selectable(translation.c_str(), code == colorBuf))
					{
						colorBuf = code;
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo(u8"ไอคอนนำหน้า", icons[iconBuf].c_str()))
			{
				for (auto& [icon, translation] : icons)
				{
					if (ImGui::Selectable(translation.c_str(), icon == iconBuf))
					{
						iconBuf = icon;
					}
				}
				ImGui::EndCombo();
			}

			InputTextWithHint(u8"ชื่อที่ปลอม", u8"กรอกชื่อที่ต้องการ", &nameBuf).Draw();
			if (ImGui::Button(u8"ตั้งชื่อปลอม"))
			{
				std::string concatName        = std::string(colorBuf) + std::string(iconBuf) + nameBuf;
				g_SpoofingStorage.spoofedName = concatName;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Update your spoofed name");

			ImGui::Text(u8"ที่อยู่ IP");
			ImGui::Checkbox(u8"ปลอม IP", &g_SpoofingStorage.spoofIP);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Spoof your IP");

			ImGui::DragInt4("##ip_fields", g_SpoofingStorage.spoofedIP.data(), 0, 255);

			ImGui::Text("Rockstar ID");
			ImGui::Checkbox(u8"ปลอม RID", &g_SpoofingStorage.spoofRID);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Spoof your Rockstar ID");

			ImGui::InputScalar("##rockstar_id_input", ImGuiDataType_U64, &g_SpoofingStorage.spoofedRID);
		}));

		blipSpoofingGroup->AddItem(std::make_shared<BoolCommandItem>("spoofblip"_J, u8"ปลอม Blip"));
		blipSpoofingGroup->AddItem(std::make_shared<ConditionalItem>("spoofblip"_J, std::make_shared<ListCommandItem>("blipsprite"_J, u8"รูป Blip")));
		blipSpoofingGroup->AddItem(std::make_shared<BoolCommandItem>("spoofprimaryicon"_J, u8"ปลอมไอคอนหลัก"));
		blipSpoofingGroup->AddItem(std::make_shared<ConditionalItem>("spoofprimaryicon"_J, std::make_shared<ListCommandItem>("primaryicon"_J, u8"ไอคอนหลัก")));
		blipSpoofingGroup->AddItem(std::make_shared<BoolCommandItem>("spoofsecondaryicon"_J, u8"ปลอมไอคอนรอง"));
		blipSpoofingGroup->AddItem(std::make_shared<ConditionalItem>("spoofsecondaryicon"_J, std::make_shared<ListCommandItem>("secondaryicon"_J, u8"ไอคอนรอง")));

		auto discriminatorGroup = std::make_shared<Group>("", 1);
		sessionSpoofingGroup->AddItem(std::make_shared<BoolCommandItem>("spoofdiscriminator"_J, u8"ปลอม Discriminator"));
		discriminatorGroup->AddItem(std::make_shared<IntCommandItem>("discriminator"_J, "Discriminator"));
		discriminatorGroup->AddItem(std::make_shared<CommandItem>("copydiscriminator"_J, u8"คัดลอกปัจจุบัน"));
		sessionSpoofingGroup->AddItem(std::make_shared<ConditionalItem>("spoofdiscriminator"_J, std::move(discriminatorGroup)));

		spoofing->AddItem(infoSpoofingGroup);
		spoofing->AddItem(blipSpoofingGroup);
		spoofing->AddItem(sessionSpoofingGroup);

		auto voice = BuildVoiceMenu();

		AddCategory(std::move(session));
		AddCategory(std::move(spoofing));
		AddCategory(std::move(voice));
		AddCategory(std::move(database));
	}
}