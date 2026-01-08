#include "Recovery.hpp"

#include "core/commands/BoolCommand.hpp"
#include "core/commands/Commands.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/ScriptFunction.hpp"
#include "game/rdr/Scripts.hpp"
#include "util/Rewards.hpp"

namespace YimMenu::Submenus
{

	Recovery::Recovery() :
	    Submenu::Submenu("Recovery")
	{
		auto recovery               = std::make_shared<Category>(u8"ปั๊มของ/เงิน");
		auto spawnCollectiblesGroup = std::make_shared<Group>(u8"เสกของสะสม");
		auto spawnHerbsGroup		= std::make_shared<Group>(u8"เสกสมุนไพร");
		auto unlocksGroup           = std::make_shared<Group>(u8"ปลดล็อค");
		auto recoveryOptions        = std::make_shared<Group>(u8"ตัวเลือก");

		static auto recoveryCommand = Commands::GetCommand<BoolCommand>("recoveryenabled"_J);

		spawnCollectiblesGroup->AddItem(std::make_shared<ImGuiItem>([=] {
			if (recoveryCommand->GetState())
			{
				static Rewards::eRewardType selected{};
				std::map<Rewards::eRewardType, std::string> reward_translations = {
				    {Rewards::eRewardType::HEIRLOOMS, u8"สมบัติประจำตระกูล"},
				    {Rewards::eRewardType::COINS, u8"เหรียญ"},
				    {Rewards::eRewardType::ALCBOTTLES, u8"ขวดเหล้า"},
				    {Rewards::eRewardType::ARROWHEADS, u8"หัวลูกธนู"},
				    {Rewards::eRewardType::BRACELETS, u8"กำไล"},
				    {Rewards::eRewardType::EARRINGS, u8"ต่างหู"},
				    {Rewards::eRewardType::NECKLACES, u8"สร้อยคอ"},
				    {Rewards::eRewardType::RINGS, u8"แหวน"},
				    {Rewards::eRewardType::TAROTCARDS_CUPS, u8"ไพ่ทาโรต์ - ถ้วย"},
				    {Rewards::eRewardType::TAROTCARDS_PENTACLES, u8"ไพ่ทาโรต์ - เหรียญ"},
				    {Rewards::eRewardType::TAROTCARDS_SWORDS, u8"ไพ่ทาโรต์ - ดาบ"},
				    {Rewards::eRewardType::TAROTCARDS_WANDS, u8"ไพ่ทาโรต์ - ไม้เท้า"},
				    {Rewards::eRewardType::FOSSILS, u8"ฟอสซิล"},
				    {Rewards::eRewardType::EGGS, u8"ไข่"},
				    {Rewards::eRewardType::TREASURE, u8"รางวัลสมบัติ"},
				    {Rewards::eRewardType::CAPITALE, "Capitale"},
				    {Rewards::eRewardType::XP, "25K XP"},
				    {Rewards::eRewardType::MOONSHINERXP, "200 Moonshiner XP"},
				    {Rewards::eRewardType::TRADERXP, "200 Trader XP"},
				    {Rewards::eRewardType::COLLECTORXP, "200 Collector XP"},
				    {Rewards::eRewardType::NATURALISTXP, "300 Naturalist XP"},
				    {Rewards::eRewardType::BOUNTYHUNTERXP, "200 Bounty Hunter XP"},
				    {Rewards::eRewardType::TRADERGOODS, u8"สินค้า Trader เต็ม"},
				};

				if (ImGui::BeginCombo(u8"รางวัล", reward_translations[selected].c_str()))
				{
					for (auto& [type, translation] : reward_translations)
					{
						if (ImGui::Selectable(std::string(translation).c_str(), type == selected, ImGuiSelectableFlags_AllowDoubleClick))
						{
							selected = type;
						}
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						{
							FiberPool::Push([] {
								Rewards::GiveRequestedRewards({selected});
							});
						}
					}
					ImGui::EndCombo();
				}

				if (ImGui::Button(u8"เพิ่มที่เลือก"))
				{
					FiberPool::Push([] {
						Rewards::GiveRequestedRewards({selected});
					});
				}
			}
			else
			{
				ImGui::Text(u8"ฟีเจอร์ Recovery ถูกจำกัด");
				ImGui::TextWrapped(u8"ฟีเจอร์ปั๊มของ/ของสะสมมีความเสี่ยงและคุณอาจโดนแบนได้ คุณต้องรับผิดชอบการกระทำของคุณเอง นักพัฒนาหรือองค์กร YimMenu จะไม่รับผิดชอบต่อความเสียหายใดๆ ต่อบัญชีของคุณ");
				if (ImGui::Button(u8"ยอมรับและเปิดใช้งาน"))
				{
					recoveryCommand->SetState(true);
				}
			}
		}));
		spawnHerbsGroup->AddItem(std::make_shared<ImGuiItem>([=] {
			if (recoveryCommand->GetState())
			{
				static joaat_t selectedHerb;
				std::map<joaat_t, std::string> herb_translations = {{"HERB_LOOT_ALASKAN_GINSENG"_J, "Alaskan Ginseng"},{"HERB_LOOT_AMERICAN_GINSENG"_J, "American Ginseng"},{"HERB_LOOT_BAY_BOLETE"_J, "Bay Bolete"},{"HERB_LOOT_BLACK_BERRY"_J, "Black Berry"},{"HERB_LOOT_BLACK_CURRANT"_J, "Black Currant"},{"HERB_LOOT_BURDOCK_ROOT"_J, "Burdock Root"},{"HERB_LOOT_CHANTERELLES"_J, "Chanterelles"},{"HERB_LOOT_COMMON_BULRUSH"_J, "Common Bulrush"},{"HERB_LOOT_CREEPING_THYME"_J, "Creeping Thyme"},{"HERB_LOOT_DESERT_SAGE"_J, "Desert Sage"},{"HERB_LOOT_ENGLISH_MACE"_J, "English Mace"},{"HERB_LOOT_EVERGREEN_HUCKLEBERRY"_J, "Evergreen Huckleberry"},{"HERB_LOOT_GOLDEN_CURRANT"_J, "Golden Currant"},{"HERB_LOOT_HUMMINGBIRD_SAGE"_J, "Hummingbird Sage"},{"HERB_LOOT_INDIAN_TOBACCO"_J, "Indian Tobacco"},{"HERB_LOOT_MILKWEED"_J, "Milkweed"},{"HERB_LOOT_OLEANDER_SAGE"_J, "Oleander Sage"},{"HERB_LOOT_OREGANO"_J, "Oregano"},{"HERB_LOOT_PARASOL_MUSHROOM"_J, "Parasol Mushroom"},{"HERB_LOOT_PRAIRIE_POPPY"_J, "Prairie Poppy"},{"HERB_LOOT_RAMS_HEAD"_J, "Rams Head"},{"HERB_LOOT_RED_RASPBERRY"_J, "Red Raspberry"},{"HERB_LOOT_RED_SAGE"_J, "Red Sage"},{"HERB_LOOT_VANILLA_FLOWER"_J, "Vanilla Flower"},{"HERB_LOOT_VIOLET_SNOWDROP"_J, "Violet Snowdrop"},{"HERB_LOOT_WILD_CARROTS"_J, "Wild Carrots"},{"HERB_LOOT_WILD_FEVERFEW"_J, "Wild Feverfew"},{"HERB_LOOT_WILD_MINT"_J, "Wild Mint"},{"HERB_LOOT_WINTERGREEN_BERRY"_J, "Wintergreen Berry"},{"HERB_LOOT_YARROW"_J, "Yarrow"},{"HERB_LOOT_AGARITA"_J, "Agarita"},{"HERB_LOOT_BITTERWEED"_J, "Bitterweed"},{"HERB_LOOT_BLUE_BONNET"_J, "Blue Bonnet"},{"HERB_LOOT_BLOOD_FLOWER"_J, "Blood Flower"},{"HERB_LOOT_CARDINAL_FLOWER"_J, "Cardinal Flower"},{"HERB_LOOT_CHOCOLATE_DAISY"_J, "Chocolate Daisy"},{"HERB_LOOT_CREEK_PLUM"_J, "Creek Plum"},{"HERB_LOOT_RHUBARB"_J, "Rhubarb"},{"HERB_LOOT_WISTERIA"_J, "Wisteria"},{"HERB_LOOT_HARRIETUM"_J, "Harrietum"},};
				if (ImGui::BeginCombo(u8"สมุนไพร", herb_translations[selectedHerb].c_str()))
				{
					for (auto& [herb, translation] : herb_translations)
					{
						if (ImGui::Selectable(std::string(translation).c_str(), herb == selectedHerb))
						{
							selectedHerb = herb;
						}
					}
					ImGui::EndCombo();
				}

				static int amount = 1;
				ImGui::SliderInt(u8"จำนวน", &amount, 1, 10);

				if (ImGui::Button(u8"เสกที่เลือก"))
				{
					FiberPool::Push([] {
						if (!Scripts::RequestScript("interactive_campfire"_J))
							return;

						for (int i = 0; i < amount; i++)
							ScriptFunctions::GiveLootTableAward.StaticCall(selectedHerb, 0);
					});
				}
			}
		}));

		// Unlock features
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockeverything"_J, u8"ปลดล็อคทุกอย่าง")); // The big button
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockallclothing"_J, u8"ปลดล็อคเสื้อผ้าทั้งหมด"));
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockoutlawpass"_J, u8"ปลดล็อค Outlaw Pass"));
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockemotes"_J, u8"ปลดล็อค Emotes"));
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockhorses"_J, u8"ปลดล็อคม้า"));
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockweapons"_J, u8"ปลดล็อคอาวุธ"));
		unlocksGroup->AddItem(std::make_shared<CommandItem>("unlockcamps"_J, u8"ปลดล็อคแคมป์"));
		
		unlocksGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"คำเตือน: 'ปลดล็อคทุกอย่าง' จะทำการปลดล็อคเสื้อผ้า, ท่าทาง, ม้า, อาวุธ และของในแคมป์ทั้งหมดทันที ไม่สามารถย้อนกลับได้!");
		}));

		recoveryOptions->AddItem(std::make_shared<BoolCommandItem>("unlimiteditems"_J, u8"ไอเทมไม่จำกัด"));
		recovery->AddItem(spawnCollectiblesGroup);
		recovery->AddItem(spawnHerbsGroup);
		recovery->AddItem(unlocksGroup);
		recovery->AddItem(recoveryOptions);

		AddCategory(std::move(recovery));
	}
}