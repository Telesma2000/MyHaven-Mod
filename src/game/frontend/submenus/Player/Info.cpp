#include "Info.hpp"
#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/PlayerDatabase.hpp"
#include "game/backend/Self.hpp"
#include "game/features/Features.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Network.hpp"

#include <network/rlGamerInfo.hpp>

namespace YimMenu::Submenus
{
	std::string BuildIPStr(int field1, int field2, int field3, int field4)
	{
		std::ostringstream oss;
		oss << field1 << '.' << field2 << '.' << field3 << '.' << field4;
		return oss.str();
	}

	std::shared_ptr<Category> BuildInfoMenu()
	{
		auto menu = std::make_shared<Category>(u8"ข้อมูล");

		auto teleportGroup      = std::make_shared<Group>(u8"วาร์ป");
		auto playerOptionsGroup = std::make_shared<Group>(u8"ข้อมูลทั่วไป");

		playerOptionsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			if (Players::GetSelected().IsValid())
			{
				ImGui::Text(Players::GetSelected().GetName());
				ImGui::Checkbox(u8"ส่องดู", &YimMenu::g_Spectating);
				ImGui::Checkbox(u8"บล็อคระเบิด", &Players::GetSelected().GetData().m_BlockExplosions);
				ImGui::Checkbox(u8"บล็อคอนุภาค", &Players::GetSelected().GetData().m_BlockParticles);
				if (ImGui::Checkbox(u8"โหมดวิญญาณ", &Players::GetSelected().GetData().m_GhostMode))
				{
					if (Players::GetSelected().GetData().m_GhostMode)
					{
						FiberPool::Push([] {
							if (Players::GetSelected().IsValid())
							{
								Network::ForceRemoveNetworkEntity(Self::GetPed().GetNetworkObject(), false, Players::GetSelected());
							}
						});
					}
				}

				ImGui::Text(u8"เลเวล: %s", std::to_string(Players::GetSelected().GetRank()));

				if (Players::GetSelected().GetPed())
				{
					auto health    = Players::GetSelected().GetPed().GetHealth();
					auto maxHealth = Players::GetSelected().GetPed().GetMaxHealth();
					std::string healthStr = std::format(u8"เลือด: {}/{} ({:.2f}%)", health, maxHealth, (float)health / maxHealth * 100.0f);
					ImGui::Text("%s", healthStr.c_str());

					auto coords = Players::GetSelected().GetPed().GetPosition();
					ImGui::Text(u8"พิกัด: %.2f, %.2f, %.2f", coords.x, coords.y, coords.z);

					auto distance = Players::GetSelected().GetPed().GetPosition().GetDistance(Self::GetPed().GetPosition());
					ImGui::Text(u8"ระยะห่าง: %.2f", distance);
				}
				else
				{
					ImGui::Text(u8"ไม่พบตัวละครหรือถูกลบ");
				}

				auto rid        = Players::GetSelected().GetGamerInfo()->m_GamerHandle.m_RockstarId;
				auto rid1       = Players::GetSelected().GetRID();
				bool spoofedRid = (rid != rid1);

				if (!spoofedRid)
				{
					std::string ridStr = std::to_string(rid1);

					ImGui::Text("RID:");
					ImGui::SameLine();
					if (ImGui::Button(std::to_string(rid1).c_str()))
					{
						ImGui::SetClipboardText(std::to_string(rid1).c_str());
					}
				}
				else
				{
					std::string spoofedRidStr = std::to_string(rid);
					std::string ridStr        = std::to_string(rid1);

					ImGui::Text(u8"RID ปลอม:");
					ImGui::SameLine();
					if (ImGui::Button(spoofedRidStr.c_str()))
					{
						ImGui::SetClipboardText(spoofedRidStr.c_str());
					}

					ImGui::Text(u8"RID จริง:");
					ImGui::SameLine();
					if (ImGui::Button(ridStr.c_str()))
					{
						ImGui::SetClipboardText(ridStr.c_str());
					}
				}

				auto ip        = Players::GetSelected().GetExternalAddress();
				auto ip2        = Players::GetSelected().GetAddress()->m_external_ip;
				auto ip1       = Players::GetSelected().GetGamerInfo()->m_ExternalAddress;
				bool spoofedIp = (ip.m_packed != ip1.m_packed);

				auto addr2 = BuildIPStr(ip2.m_field1, ip2.m_field2, ip2.m_field3, ip2.m_field4);

				ImGui::Text(u8"ที่อยู่ IP ปลายทาง:");
				ImGui::SameLine();
				if (ImGui::Button(addr2.c_str()))
				{
					ImGui::SetClipboardText(addr2.c_str());
				}

				if (!spoofedIp)
				{
					auto ipStr = BuildIPStr(ip.m_field1, ip.m_field2, ip.m_field3, ip.m_field4);

					ImGui::Text(u8"ที่อยู่ IP:");
					ImGui::SameLine();
					if (ImGui::Button(ipStr.c_str()))
					{
						ImGui::SetClipboardText(ipStr.c_str());
					}
				}
				else
				{
					auto spoofedIpStr = BuildIPStr(ip1.m_field1, ip1.m_field2, ip1.m_field3, ip1.m_field4);
					auto realIpStr    = BuildIPStr(ip.m_field1, ip.m_field2, ip.m_field3, ip.m_field4);

					ImGui::Text(u8"IP ปลอม:");
					ImGui::SameLine();
					if (ImGui::Button(spoofedIpStr.c_str()))
					{
						ImGui::SetClipboardText(spoofedIpStr.c_str());
					}

					ImGui::Text(u8"IP จริง:");
					ImGui::SameLine();
					if (ImGui::Button(realIpStr.c_str()))
					{
						ImGui::SetClipboardText(realIpStr.c_str());
					}
				}

				if (ImGui::Button(u8"ดูโปรไฟล์ SC"))
					FiberPool::Push([] {
						uint64_t handle[18];
						NETWORK::NETWORK_HANDLE_FROM_PLAYER(Players::GetSelected().GetId(), (Any*)&handle);
						NETWORK::NETWORK_SHOW_PROFILE_UI((Any*)&handle);
					});
				ImGui::SameLine();
				if (ImGui::Button(u8"เพิ่มเพื่อน"))
					FiberPool::Push([] {
						uint64_t handle[18];
						NETWORK::NETWORK_HANDLE_FROM_PLAYER(Players::GetSelected().GetId(), (Any*)&handle);
						NETWORK::NETWORK_ADD_FRIEND((Any*)&handle, "");
					});
				ImGui::SameLine();
				if (ImGui::Button(u8"เพิ่มลงฐานข้อมูล"))
				{
					auto plyr = Players::GetSelected();
					g_PlayerDatabase->AddPlayer(plyr.GetRID(), plyr.GetName());
				}

				if (ImGui::Button(u8"ข้อมูลเพิ่มเติม"))
					ImGui::OpenPopup("More Info");

				ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
				if (ImGui::BeginPopupModal("More Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal | ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text(u8"ภาษา: %s", g_LanguageMap[Players::GetSelected().GetLanguage()].c_str());

					auto honor = Players::GetSelected().GetHonor();
					std::string honorLevel;

					if (honor >= 0 && honor <= 7)
					{
						honorLevel = u8"ต่ำ";
					}
					else if (honor > 7 && honor <= 10)
					{
						honorLevel = u8"ปานกลาง";
					}
					else if (honor > 10 && honor <= 15)
					{
						honorLevel = u8"สูง";
					}
					else
					{
						honorLevel = u8"ไม่ถูกต้อง";
					}

					honorLevel += " (" + std::to_string(honor) + "/15)";
					ImGui::Text(u8"เกียรติยศ: %s", honorLevel.c_str());

					std::string model = std::format("0x{:08X}", (joaat_t)Players::GetSelected().GetPed().GetModel());
					ImGui::Text(u8"โมเดล: %s", model.c_str());
					ImGui::SameLine();
					if (ImGui::Button(u8"คัดลอก"))
						ImGui::SetClipboardText(model.c_str());

					if (auto it = g_DistrictMap.find(Players::GetSelected().GetDistrict()); it != g_DistrictMap.end())
						ImGui::Text(u8"เขต: %s", it->second.c_str());

					if (auto it = g_RegionMap.find(Players::GetSelected().GetRegion()); it != g_RegionMap.end())
						ImGui::Text(u8"ภูมิภาค: %s", it->second.c_str());

					auto internalIp = Players::GetSelected().GetInternalAddress();
					ImGui::Text(u8"IP ภายใน: %s",
					    std::format("{}.{}.{}.{}:{}",
					        static_cast<int>(internalIp.m_field1),
					        static_cast<int>(internalIp.m_field2),
					        static_cast<int>(internalIp.m_field3),
					        static_cast<int>(internalIp.m_field4),
					        Players::GetSelected().GetInternalPort())
					        .c_str());

					auto relayIp = Players::GetSelected().GetRelayAddress();
					ImGui::Text(u8"IP รีเลย์: %s",
					    std::format("{}.{}.{}.{}:{}",
					        static_cast<int>(relayIp.m_field1),
					        static_cast<int>(relayIp.m_field2),
					        static_cast<int>(relayIp.m_field3),
					        static_cast<int>(relayIp.m_field4),
					        Players::GetSelected().GetRelayPort())
					        .c_str());


					ImGui::Text(u8"ประเภทการเชื่อมต่อ: %u", Players::GetSelected().GetConnectionType());

					ImGui::Text(u8"ความหน่วงเฉลี่ย: %.2f", Players::GetSelected().GetAverageLatency());
					ImGui::Text(u8"Packet Loss: %.2f", Players::GetSelected().GetAveragePacketLoss());

					ImGui::Spacing();

					if (ImGui::Button(u8"ปิด") || ((!ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
			}
			else
			{
				Players::SetSelected(Self::GetPlayer());
				ImGui::Text(u8"ยังไม่มีผู้เล่น!");
			}
		}));

		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tptoplayer"_J, u8"วาร์ปไปหาผู้เล่น"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tptoplayercamp"_J, u8"วาร์ปไปแคมป์ผู้เล่น"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tpbehindplayer"_J, u8"วาร์ปไปหลังผู้เล่น"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tpintovehicle"_J, u8"วาร์ปเข้ายานพาหนะ"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("bring"_J, u8"ดึงผู้เล่นมา"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tpplayertowaypoint"_J, u8"ส่งผู้เล่นไปจุดมาร์ค"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tpplayertomadamnazar"_J, u8"ส่งผู้เล่นไปหา Madam Nazar"));
		teleportGroup->AddItem(std::make_shared<PlayerCommandItem>("tpplayertojail"_J, u8"ส่งผู้เล่นเข้าคุก"));

		menu->AddItem(playerOptionsGroup);
		menu->AddItem(teleportGroup);

		return menu;
	}
}