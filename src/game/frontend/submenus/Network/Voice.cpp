#include "Voice.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/BoolCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/Self.hpp"
#include "game/backend/Voice.hpp"
#include "game/rdr/Packet.hpp"

#include <network/rlGamerInfo.hpp>

namespace YimMenu::Submenus
{
	static std::optional<std::vector<std::string>> g_AudioFiles;

	static void RefreshAudioFileMap()
	{
		g_AudioFiles = std::vector<std::string>();

		if (!std::filesystem::exists(Voice::GetAudioDirectory()))
			return;

		for (const auto& entry : std::filesystem::recursive_directory_iterator(Voice::GetAudioDirectory()))
		{
			if (entry.is_regular_file())
				g_AudioFiles->push_back(entry.path().filename().string());
		}
	}

	static void ResendVoiceInfoPackets()
	{
		Packet pkt;
		pkt.WriteMessageHeader(NetMessageType::VOICE_CHAT_STATUS);
		pkt.GetBuffer().Write(0x41, 9);
		pkt.GetBuffer().Write(0, 7);
		pkt.GetBuffer().Write(0, 7);
		
		Player player = Self::GetPlayer();
		if (Voice::GetSpoofingPlayer().IsValid())
			player = Voice::GetSpoofingPlayer();

		if (player.GetGamerInfo())
		{
			player.GetGamerInfo()->m_GamerHandle.Serialize(pkt.GetBuffer());
		}
		
		// ส่วนนี้ถูกปิดไว้เนื่องจาก Backend ไม่มีฟังก์ชัน GetVoiceData
		// if (auto voice = Voice::GetVoiceData(); voice && voice->m_Url[0])
		// 	pkt.Writestring(voice->m_Url, 64);

		// แก้ไขให้ส่ง Argument ครบตาม Packet.hpp: Send(msg_id, connection_id)
		pkt.Send(0, -1); 
	}

	void ShowVoiceSpoofingMenu()
	{
		auto player_name = Voice::GetSpoofingPlayer().IsValid() ? Voice::GetSpoofingPlayer().GetName() : u8"ปิดการใช้งาน";
		ImGui::SetNextItemWidth(150);
		if (ImGui::BeginCombo(u8"ผู้ส่งที่ปลอม", player_name))
		{
			if (ImGui::Selectable(u8"ปิดการใช้งาน", !Voice::GetSpoofingPlayer().IsValid()))
			{
				Voice::SetSpoofingPlayer(nullptr);
				FiberPool::Push([] {
					ResendVoiceInfoPackets();
				});
			}

			for (auto& [id, plyr] : Players::GetPlayers())
			{
				if (!plyr.IsValid())
					continue;

				if (ImGui::Selectable(plyr.GetName(), plyr == Voice::GetSpoofingPlayer()))
				{
					Voice::SetSpoofingPlayer(plyr);
					FiberPool::Push([] {
						ResendVoiceInfoPackets();
					});
				}

				if (plyr == Voice::GetSpoofingPlayer())
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	std::shared_ptr<Category> BuildVoiceMenu()
	{
		auto voice = std::make_shared<Category>(u8"เสียง");
		voice->AddItem(std::make_shared<BoolCommandItem>("hearall"_J));
		voice->AddItem(std::make_shared<BoolCommandItem>("logvoice"_J));
		voice->AddItem(std::make_shared<ImGuiItem>([] {
			ShowVoiceSpoofingMenu();
		}));

		auto override_ = std::make_shared<Group>(u8"แทนที่เสียงไมค์");
		override_->AddItem(std::make_shared<ImGuiItem>([] {
			if (!g_AudioFiles)
				RefreshAudioFileMap();

			static std::string current_file = u8"เลือกไฟล์";
			
			if (ImGui::Button(u8"รีเฟรช"))
			{
				RefreshAudioFileMap();
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(200.f);
			if (ImGui::BeginCombo(u8"ไฟล์เสียง", current_file.c_str()))
			{
				for (auto& file : *g_AudioFiles)
				{
					if (ImGui::Selectable(file.c_str(), file == current_file))
					{
						current_file = file;
					}

					if (file == current_file)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button(u8"เล่น"))
			{
				// แก้ไขชื่อฟังก์ชันให้ตรงกับ Voice.hpp
				Voice::SetVoiceFile(current_file);
			}

			ImGui::SameLine();

			if (ImGui::Button(u8"หยุด"))
			{
				// แก้ไขชื่อฟังก์ชันให้ตรงกับ Voice.hpp
				Voice::SetVoiceFile("");
			}

			ImGui::SameLine();

			if (ImGui::Button(u8"รีเซ็ต"))
			{
				// ไม่มีฟังก์ชัน Reset ใช้การเคลียร์ค่าแทน
				Voice::SetVoiceFile("");
			}
		}));

		voice->AddItem(override_);

		return voice;
	}
}