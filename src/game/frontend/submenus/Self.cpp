#include "Self.hpp"

#include "core/commands/BoolCommand.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/IntCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/features/Features.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/data/Emotes.hpp"
#include "util/Rewards.hpp"

#include <map>


namespace YimMenu::Features
{
	BoolCommand _RecoveryEnabled("recoveryenabled", "Recovery Enabled", "Is the recovery feature enabled");
	// เพิ่มบรรทัดนี้เพื่อประกาศฟังก์ชันให้ Linker รู้จัก
	std::string& GetChatBubbleText();
}

namespace YimMenu::Submenus
{
	void RenderAnimationsCategory()
	{
		static std::string anim, dict;
		InputTextWithHint("Dictionary", u8"ใส่ชื่อ Dictionary", &dict).Draw();
		InputTextWithHint("Animation", u8"ใส่ชื่อ Animation", &anim).Draw();

		if (ImGui::Button(u8"เล่น Animation"))
		{
			FiberPool::Push([=] {
				for (int i = 0; i < 250; i++)
				{
					if (dict.empty() || anim.empty())
						break;

					if (STREAMING::HAS_ANIM_DICT_LOADED(dict.c_str()))
						break;

					STREAMING::REQUEST_ANIM_DICT(dict.c_str());
					ScriptMgr::Yield();
				}

				TASK::TASK_PLAY_ANIM(YimMenu::Self::GetPed().GetHandle(), dict.c_str(), anim.c_str(), 8.0f, -8.0f, -1, 0, 0, false, false, false, "", 0);
			});
		}

		ImGui::Separator();

		ImGui::Text(u8"หมวดหมู่ Emote");
		if (ImGui::BeginCombo("##Emote Category", Emote::emoteCategories[Emote::selectedEmoteCategoryIndex]))
		{
			for (int i = 0; i < Emote::numCategories; i++)
			{
				bool isSelected = (i == Emote::selectedEmoteCategoryIndex);
				if (ImGui::Selectable(Emote::emoteCategories[i], isSelected))
				{
					Emote::selectedEmoteCategoryIndex = i;
					Emote::selectedEmoteMemberIndex   = 0;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Emote");
		if (ImGui::BeginCombo("##Emote",
		        Emote::emoteCategoryMembers[Emote::selectedEmoteCategoryIndex][Emote::selectedEmoteMemberIndex].name))
		{
			for (int i = 0; i < Emote::maxEmotesPerCategory; i++)
			{
				const auto& emote = Emote::emoteCategoryMembers[Emote::selectedEmoteCategoryIndex][i];
				if (emote.name == nullptr)
					break;
				bool isSelected = (i == Emote::selectedEmoteMemberIndex);
				if (ImGui::Selectable(emote.name, isSelected))
				{
					Emote::selectedEmoteMemberIndex = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button(u8"เล่น Emote"))
		{
			if (*Pointers.IsSessionStarted)
			{
				FiberPool::Push([=] {
					int selectedCategoryIndex = Emote::selectedEmoteCategoryIndex;
					int selectedEmoteIndex    = Emote::selectedEmoteMemberIndex;
					const Emote::EmoteItemData& selectedEmote = Emote::emoteCategoryMembers[selectedCategoryIndex][selectedEmoteIndex];

					TASK::TASK_PLAY_EMOTE_WITH_HASH(YimMenu::Self::GetPed().GetHandle(),
					    static_cast<int>(selectedEmote.type),
					    EMOTE_PM_FULLBODY,
					    static_cast<Hash>(selectedEmote.hash),
					    false,
					    false,
					    false,
					    false,
					    false);
				});
			}
		}

		if (ImGui::Button(u8"หยุด Animation"))
		{
			FiberPool::Push([=] {
				TASK::CLEAR_PED_TASKS(YimMenu::Self::GetPed().GetHandle(), true, false);
			});
		}
	}

	Self::Self() :
	    Submenu::Submenu("Self")
	{
		auto main          = std::make_shared<Category>(u8"หน้าหลัก");
		auto globalsGroup  = std::make_shared<Group>(u8"ทั่วไป");
		auto movementGroup = std::make_shared<Group>(u8"การเคลื่อนที่");
		auto toolsGroup    = std::make_shared<Group>(u8"เครื่องมือ");
		auto customGroup   = std::make_shared<Group>(u8"ปรับแต่ง");
		auto teleportGroup = std::make_shared<Group>(u8"วาร์ป"); // New group for TP features

		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("godmode"_J, u8"อมตะ"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("neverwanted"_J, u8"ไม่ติดดาว"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("invis"_J, u8"ล่องหน"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("offtheradar"_J, u8"ปิดเรดาร์"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("noragdoll"_J, u8"ตัวไม่ล้ม"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("antiafk"_J, u8"กัน AFK"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("keepbarsfilled"_J, u8"พลังเต็มตลอด"));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepbarsfilled"_J, std::make_shared<BoolCommandItem>("keepdeadeyefilled"_J, u8"DeadEye เต็ม")));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepbarsfilled"_J, std::make_shared<BoolCommandItem>("keepstaminafilled"_J, u8"Stamina เต็ม")));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepbarsfilled"_J, std::make_shared<BoolCommandItem>("keephealthfilled"_J, u8"เลือดเต็ม")));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("keepcoresfilled"_J, u8"Core เต็มตลอด"));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepcoresfilled"_J, std::make_shared<BoolCommandItem>("keepdeadeyecorefilled"_J, u8"DeadEye Core เต็ม")));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepcoresfilled"_J, std::make_shared<BoolCommandItem>("keepstaminacorefilled"_J, u8"Stamina Core เต็ม")));
		globalsGroup->AddItem(std::make_shared<ConditionalItem>("keepcoresfilled"_J, std::make_shared<BoolCommandItem>("keephealthcorefilled"_J, u8"เลือด Core เต็ม")));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("keepclean"_J, u8"ตัวสะอาดตลอด"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("antilasso"_J, u8"กันโดนคล้องเชือก"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("antihogtie"_J, u8"กันโดนมัด"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("antimelee"_J, u8"กันโดนต่อย"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("drunk"_J, u8"เมา"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("superpunch"_J, u8"หมัดหนัก"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("quickskin"_J, u8"แล่เนื้อเร็ว"));
		globalsGroup->AddItem(std::make_shared<BoolCommandItem>("fastmoonshine"_J, u8"ต้มเหล้าเร็ว")); // Added feature

		// Teleport Features
		teleportGroup->AddItem(std::make_shared<CommandItem>("tptocamp"_J, u8"วาร์ปไปแคมป์"));
		teleportGroup->AddItem(std::make_shared<CommandItem>("tptomoonshineshack"_J, u8"วาร์ปไปโรงต้มเหล้า")); // Need to ensure command name matches
		teleportGroup->AddItem(std::make_shared<CommandItem>("tptomadamnazar"_J, u8"วาร์ปไปหา Madam Nazar"));
		teleportGroup->AddItem(std::make_shared<CommandItem>("tptotraintrack"_J, u8"วาร์ปไปรางรถไฟ"));
		teleportGroup->AddItem(std::make_shared<CommandItem>("tptomount"_J, u8"วาร์ปไปหาม้า"));
		teleportGroup->AddItem(std::make_shared<BoolCommandItem>("autotp"_J, u8"วาร์ปอัตโนมัติ")); // Added AutoTP

		toolsGroup->AddItem(std::make_shared<CommandItem>("suicide"_J, u8"ฆ่าตัวตาย"));
		toolsGroup->AddItem(std::make_shared<CommandItem>("clearcrimes"_J, u8"ล้างค่าหัว"));
		toolsGroup->AddItem(std::make_shared<BoolCommandItem>("npcignore"_J, u8"NPC ไม่สนใจ"));
		toolsGroup->AddItem(std::make_shared<BoolCommandItem>("eagleeye"_J, u8"Eagle Eye ตลอด"));
		toolsGroup->AddItem(std::make_shared<BoolCommandItem>("overridewhistle"_J, u8"ปรับเสียงผิวปาก"));
		toolsGroup->AddItem(std::make_shared<ConditionalItem>("overridewhistle"_J, std::make_shared<FloatCommandItem>("whistlepitch"_J, "Pitch")));
		toolsGroup->AddItem(std::make_shared<ConditionalItem>("overridewhistle"_J, std::make_shared<FloatCommandItem>("whistleclarity"_J, "Clarity")));
		toolsGroup->AddItem(std::make_shared<ConditionalItem>("overridewhistle"_J, std::make_shared<FloatCommandItem>("whistleshape"_J, "Shape")));

		static float playerScale = 1;
		toolsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::Text(u8"ขนาดตัวละคร");
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputFloat(" ", &playerScale))
				FiberPool::Push([] {
					YimMenu::Self::GetPed().SetScale(playerScale);
				});
		}));

		movementGroup->AddItem(std::make_shared<BoolCommandItem>("climbsteepslopes"_J, u8"ปีนที่สูงชัน"));
		movementGroup->AddItem(std::make_shared<BoolCommandItem>("superjump"_J, u8"กระโดดสูง"));
		movementGroup->AddItem(std::make_shared<BoolCommandItem>("superrun"_J, u8"วิ่งเร็ว"));
		movementGroup->AddItem(std::make_shared<BoolCommandItem>("noclip"_J, u8"เดินทะลุสิ่งของ"));
		movementGroup->AddItem(std::make_shared<ConditionalItem>("noclip"_J, std::make_shared<FloatCommandItem>("noclipspeed"_J, u8"ความเร็ว Noclip")));
		movementGroup->AddItem(std::make_shared<BoolCommandItem>("freecam"_J, u8"กล้องอิสระ"));
		movementGroup->AddItem(std::make_shared<ConditionalItem>("freecam"_J, std::make_shared<FloatCommandItem>("freecamspeed"_J, u8"ความเร็วกล้อง")));

		customGroup->AddItem(std::make_shared<BoolCommandItem>("glowingplayer"_J, u8"ตัวเรืองแสง"));
		customGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ทำให้ตัวละครของคุณเรืองแสงสีทองอบอุ่น!");
		}));

		main->AddItem(globalsGroup);
		main->AddItem(toolsGroup);
		main->AddItem(teleportGroup); // Add the new group
		main->AddItem(movementGroup);
		main->AddItem(customGroup);
		AddCategory(std::move(main));

		auto weapons             = std::make_shared<Category>(u8"อาวุธ");
		auto weaponsGlobalsGroup = std::make_shared<Group>(u8"ทั่วไป");
		auto weaponsGiveGroup    = std::make_shared<Group>(u8"เสกอาวุธ"); // New Group

		weaponsGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("infiniteammo"_J, u8"กระสุนไม่จำกัด"));
		weaponsGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("infiniteclip"_J, u8"แม็กกาซีนไม่จำกัด"));
		weaponsGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("nospread"_J, u8"เป้าไม่บาน"));
		weaponsGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("autocock"_J, u8"ชักปืนอัตโนมัติ"));
		weaponsGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("keepgunsclean"_J, u8"ปืนสะอาดตลอด"));

		// Give Weapons features
		weaponsGiveGroup->AddItem(std::make_shared<CommandItem>("giveallweapons"_J, u8"เสกอาวุธทั้งหมด"));
		weaponsGiveGroup->AddItem(std::make_shared<CommandItem>("giveallammo"_J, u8"เติมกระสุนทั้งหมด")); // Need to ensure command name matches

		weapons->AddItem(weaponsGlobalsGroup);
		weapons->AddItem(weaponsGiveGroup);
		AddCategory(std::move(weapons));

		auto horse             = std::make_shared<Category>(u8"ม้า");
		auto horseGlobalsGroup = std::make_shared<Group>(u8"ทั่วไป");
		auto horseCustomGroup = std::make_shared<Group>(u8"ปรับแต่ง");

		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("horsegodmode"_J, u8"ม้าอมตะ"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("horsenoragdoll"_J, u8"ม้าไม่ล้ม"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("horsesuperrun"_J, u8"ม้าวิ่งเร็ว"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("keephorseclean"_J, u8"ม้าสะอาดตลอด"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("horseclimbsteepslopes"_J, u8"ม้าปีนที่สูงชัน"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("keephorsebarsfilled"_J, u8"พลังม้าเต็มตลอด"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("keephorsecoresfilled"_J, u8"Core ม้าเต็มตลอด"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("keephorseagitationlow"_J, u8"ม้าไม่ตื่นกลัว"));
		horseGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("flaminghooves"_J, u8"รอยเท้าม้าติดไฟ"));
		horseGlobalsGroup->AddItem(std::make_shared<CommandItem>("tpmounttoself"_J, u8"วาร์ปม้ามาหาตัว"));
		static float horseScale = 1;
		horseGlobalsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::Text(u8"ขนาดม้า");
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputFloat(" ", &horseScale))
				FiberPool::Push([] {
					YimMenu::Self::GetMount().SetScale(horseScale);
				});
		}));

		horseCustomGroup->AddItem(std::make_shared<BoolCommandItem>("rainbowhorse"_J, u8"ม้าสีรุ้ง"));
		horseCustomGroup->AddItem(std::make_shared<IntCommandItem>("customhorsecolor"_J, u8"สีม้ากำหนดเอง"));
		horseCustomGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"รหัสสี: 0=แดง, 6=ส้ม, 24=เหลือง, 66=เขียว, 73=ฟ้า, 67=น้ำเงิน, 49=ม่วง, 71=ชมพู");
		}));

		horse->AddItem(horseGlobalsGroup);
		horse->AddItem(horseCustomGroup);
		AddCategory(std::move(horse));

		auto vehicle             = std::make_shared<Category>(u8"ยานพาหนะ");
		auto vehicleGlobalsGroup = std::make_shared<Group>(u8"ทั่วไป");
		auto vehicleFunGroup     = std::make_shared<Group>(u8"สนุกสนาน");
		auto vehicleCustomGroup  = std::make_shared<Group>(u8"ปรับแต่ง");

		vehicleGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("vehiclegodmode"_J, u8"ยานพาหนะอมตะ"));
		vehicleGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("vehiclenodetach"_J, u8"รถไม่หลุดจากม้า"));
		vehicleGlobalsGroup->AddItem(std::make_shared<BoolCommandItem>("flaminghoovesdraft"_J, u8"รอยเท้าม้าลากรถติดไฟ"));
		vehicleGlobalsGroup->AddItem(std::make_shared<CommandItem>("repairvehicle"_J, u8"ซ่อมยานพาหนะ"));

		vehicleFunGroup->AddItem(std::make_shared<BoolCommandItem>("superdrive"_J, u8"ขับเร็วพิเศษ"));
		vehicleFunGroup->AddItem(std::make_shared<ConditionalItem>("superdrive"_J, std::make_shared<BoolCommandItem>("superdrivedirectional"_J, "Directional")));
		vehicleFunGroup->AddItem(std::make_shared<ConditionalItem>("superdrive"_J, std::make_shared<IntCommandItem>("superdriveforce"_J, "Force")));
		vehicleFunGroup->AddItem(std::make_shared<BoolCommandItem>("superbrake"_J, u8"เบรคพิเศษ"));

		vehicleCustomGroup->AddItem(std::make_shared<BoolCommandItem>("rainbowvehicle"_J, u8"ยานพาหนะสีรุ้ง"));

		vehicle->AddItem(vehicleGlobalsGroup);
		vehicle->AddItem(vehicleFunGroup);
		vehicle->AddItem(vehicleCustomGroup);
		AddCategory(std::move(vehicle));

		auto animations = std::make_shared<Category>(u8"ท่าทาง");
		animations->AddItem(std::make_shared<ImGuiItem>([] {
			RenderAnimationsCategory();
		}));
		AddCategory(std::move(animations));

		// CREATIVE MODE
		auto creative = std::make_shared<Category>(u8"สร้างสรรค์");
		auto visualsGroup = std::make_shared<Group>(u8"เอฟเฟกต์ภาพ");
		auto particlesGroup = std::make_shared<Group>(u8"เอฟเฟกต์อนุภาค");
		auto playerEffectsGroup = std::make_shared<Group>(u8"เอฟเฟกต์ผู้เล่น");
		auto worldGroup = std::make_shared<Group>(u8"ควบคุมโลก");
		auto companionsGroup = std::make_shared<Group>(u8"ผู้ติดตาม");
		auto vehiclesGroup = std::make_shared<Group>(u8"เครื่องร่อน");
		auto ridablesGroup = std::make_shared<Group>(u8"สัตว์ขี่");
		auto explorationGroup = std::make_shared<Group>(u8"การสำรวจ");
		auto peacefulGroup = std::make_shared<Group>(u8"ฟีเจอร์รักสงบ");

		visualsGroup->AddItem(std::make_shared<BoolCommandItem>("visualfilterapplier"_J, u8"เปิดใช้ฟิลเตอร์ภาพ"));
		visualsGroup->AddItem(std::make_shared<ConditionalItem>("visualfilterapplier"_J, std::make_shared<ListCommandItem>("visualfilter"_J, u8"เลือกฟิลเตอร์")));
		visualsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"เลือกจาก 10 ฟิลเตอร์ภาพยนตร์: Sepia Western, Film Noir, Golden Hour, Dreamy และอื่นๆ!");
		}));
		visualsGroup->AddItem(std::make_shared<BoolCommandItem>("customfov"_J, u8"ปรับ FOV"));
		visualsGroup->AddItem(std::make_shared<ConditionalItem>("customfov"_J, std::make_shared<FloatCommandItem>("customfovvalue"_J, u8"ค่า FOV")));
		visualsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ปรับมุมกล้อง FOV (30=ซูม, 90=กว้าง, 120=ตาปลา) สำหรับถ่ายรูปสวยๆ!");
		}));
		visualsGroup->AddItem(std::make_shared<BoolCommandItem>("cinematicletterbox"_J, u8"ขอบดำภาพยนตร์"));
		visualsGroup->AddItem(std::make_shared<ConditionalItem>("cinematicletterbox"_J, std::make_shared<FloatCommandItem>("letterboxsize"_J, u8"ขนาดขอบ")));
		visualsGroup->AddItem(std::make_shared<BoolCommandItem>("hidehud"_J, u8"ซ่อน HUD"));
		visualsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"โหมดภาพยนตร์: เพิ่มขอบดำแบบหนัง & ซ่อน HUD เพื่อภาพสวยๆ!");
		}));

		particlesGroup->AddItem(std::make_shared<BoolCommandItem>("particletrails"_J, u8"เอฟเฟกต์ตามตัว"));
		particlesGroup->AddItem(std::make_shared<ConditionalItem>("particletrails"_J, std::make_shared<ListCommandItem>("particletrail"_J, u8"เลือกเอฟเฟกต์")));
		particlesGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ทิ้งรอยวิเศษ: ผีเสื้อ, หิ่งห้อย, ประกายไฟ, ไฟ, หิมะ, กลีบดอกไม้ และอื่นๆ!");
		}));

		playerEffectsGroup->AddItem(std::make_shared<BoolCommandItem>("playerscale"_J, u8"ขนาดตัวละคร"));
		playerEffectsGroup->AddItem(std::make_shared<ConditionalItem>("playerscale"_J, std::make_shared<FloatCommandItem>("playerscalevalue"_J, u8"ค่าขนาด")));
		playerEffectsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ปรับขนาดตัวคุณ! 0.2=จิ๋ว, 1.0=ปกติ, 5.0=ยักษ์ มองโลกในมุมมองใหม่!");
		}));
		playerEffectsGroup->AddItem(std::make_shared<BoolCommandItem>("flaminglasso"_J, u8"บ่วงบาศติดไฟ"));
		playerEffectsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"บ่วงบาศของคุณลุกเป็นไฟ! เอฟเฟกต์สุดเท่!");
		}));
		playerEffectsGroup->AddItem(std::make_shared<BoolCommandItem>("ghosttrail"_J, u8"ร่างเงา"));
		playerEffectsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ทิ้งร่างเงาโฮโลแกรมไว้ข้างหลังขณะเดิน - สุดหลอน!");
		}));
		playerEffectsGroup->AddItem(std::make_shared<BoolCommandItem>("telekinesis"_J, u8"พลังจิต"));
		playerEffectsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ย้ายสิ่งของด้วยจิต! กด E ค้างเพื่อจับ, ปล่อยเพื่อขว้าง ใช้ Q/C ปรับระยะ, Shift ซ้ายเพื่อหมุน!");
		}));
		playerEffectsGroup->AddItem(std::make_shared<IntCommandItem>("clonearmycount"_J, u8"จำนวนร่างโคลน"));
		playerEffectsGroup->AddItem(std::make_shared<CommandItem>("spawnclonearmy"_J, u8"เสกกองทัพร่างโคลน"));
		playerEffectsGroup->AddItem(std::make_shared<CommandItem>("removeclonearmy"_J, u8"ลบร่างโคลน"));
		playerEffectsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"สร้างกองทัพตัวคุณเอง! เสก 1-20 ร่างที่จะตามคุณไปทุกที่!");
		}));

		worldGroup->AddItem(std::make_shared<BoolCommandItem>("slowmotion"_J, u8"ภาพช้า"));
		worldGroup->AddItem(std::make_shared<ConditionalItem>("slowmotion"_J, std::make_shared<FloatCommandItem>("slowmotionspeed"_J, u8"ความเร็ว")));
		worldGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ภาพช้าแบบภาพยนตร์! 0.1=ช้ามาก, 1.0=ปกติ เหมาะสำหรับช็อตเด็ด!");
		}));
		worldGroup->AddItem(std::make_shared<BoolCommandItem>("freezetime"_J, u8"หยุดเวลา"));
		worldGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"หยุดเวลาเพื่อจับภาพช่วงเวลาที่สมบูรณ์แบบและสำรวจตามใจชอบ!");
		}));
		worldGroup->AddItem(std::make_shared<BoolCommandItem>("gravitymodifier"_J, u8"ปรับแรงโน้มถ่วง"));
		worldGroup->AddItem(std::make_shared<ConditionalItem>("gravitymodifier"_J, std::make_shared<ListCommandItem>("gravitymode"_J, u8"โหมด")));
		worldGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"สัมผัสแรงโน้มถ่วงแบบต่างๆ: ดวงจันทร์, ไร้น้ำหนัก, กลับหัว (ตกขึ้นฟ้า!), หรือหนักอึ้ง!");
		}));
		worldGroup->AddItem(std::make_shared<BoolCommandItem>("skycolorchanger"_J, u8"เปลี่ยนสีท้องฟ้า"));
		worldGroup->AddItem(std::make_shared<ConditionalItem>("skycolorchanger"_J, std::make_shared<ListCommandItem>("skycolortype"_J, u8"สี")));
		worldGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ระบายสีท้องฟ้า! ม่วงฝัน, ชมพูสายไหม, เขียวเอเลี่ยน, รุ้ง และอื่นๆ!");
		}));
		worldGroup->AddItem(std::make_shared<ListCommandItem>("fireworktype"_J, u8"ประเภทพลุ"));
		worldGroup->AddItem(std::make_shared<CommandItem>("launchfireworks"_J, u8"จุดพลุ"));
		worldGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"จุดพลุ! ลูกเดียว, หลายลูก, รุ้ง, น้ำพุ, หรือรูปหัวใจ - ฉลองอย่างมีสไตล์!");
		}));

		companionsGroup->AddItem(std::make_shared<ListCommandItem>("companiontype"_J, u8"ประเภทผู้ติดตาม"));
		companionsGroup->AddItem(std::make_shared<CommandItem>("spawncompanion"_J, u8"เสกผู้ติดตาม"));
		companionsGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"เสกสัตว์เลี้ยงผู้ติดตาม: หมา, หมาป่า, หมี, นกอินทรี, กวาง และอื่นๆ! พวกมันจะตามคุณไปทุกที่!");
		}));

		vehiclesGroup->AddItem(std::make_shared<ListCommandItem>("flyingmachinetype"_J, u8"ประเภทเครื่องบิน"));
		vehiclesGroup->AddItem(std::make_shared<CommandItem>("spawnflyingmachine"_J, u8"เสกเครื่องบิน"));
		vehiclesGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"บินด้วยเรือเหาะ, เครื่องบิน, UFO, บอลลูนลมร้อน และอื่นๆ! สำรวจท้องฟ้า!");
		}));

		ridablesGroup->AddItem(std::make_shared<ListCommandItem>("ridableanimaltype"_J, u8"ประเภทสัตว์ขี่"));
		ridablesGroup->AddItem(std::make_shared<CommandItem>("spawnridableanimal"_J, u8"เสกสัตว์ขี่"));
		ridablesGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ขี่นกอินทรีบนฟ้า, สิงโตข้ามทุ่ง, หมีผ่านป่า!");
		}));

		explorationGroup->AddItem(std::make_shared<ListCommandItem>("playermodeltype"_J, u8"โมเดลตัวละคร"));
		explorationGroup->AddItem(std::make_shared<CommandItem>("changeplayermodel"_J, u8"เปลี่ยนโมเดล"));
		explorationGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"แปลงร่างเป็นใครก็ได้! สวมบทเป็น Arthur, Sadie, Dutch หรือ NPC คนไหนก็ได้!");
		}));
		explorationGroup->AddItem(std::make_shared<BoolCommandItem>("nightvision"_J, u8"มองเห็นกลางคืน"));
		explorationGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"มองเห็นชัดเจนในความมืดด้วยวิสัยทัศน์พิเศษ!");
		}));
		explorationGroup->AddItem(std::make_shared<BoolCommandItem>("walkonwater"_J, u8"เดินบนน้ำ"));
		explorationGroup->AddItem(std::make_shared<BoolCommandItem>("walkonair"_J, u8"เดินบนอากาศ"));
		explorationGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"เดินข้ามน้ำหรือสร้างทางเดินล่องหนบนท้องฟ้า!");
		}));
		explorationGroup->AddItem(std::make_shared<CommandItem>("teleporttoguarma"_J, u8"วาร์ปไป Guarma"));
		explorationGroup->AddItem(std::make_shared<CommandItem>("teleporttosisika"_J, u8"วาร์ปไป Sisika"));
		explorationGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ไปเยือนพื้นที่หวงห้าม: เกาะ Guarma, คุก Sisika, และอื่นๆ!");
		}));

		peacefulGroup->AddItem(std::make_shared<BoolCommandItem>("antibounty"_J, u8"ป้องกันค่าหัว"));
		peacefulGroup->AddItem(std::make_shared<BoolCommandItem>("autoremovebounty"_J, u8"ลบค่าหัวอัตโนมัติ"));
		peacefulGroup->AddItem(std::make_shared<BoolCommandItem>("autopaybounty"_J, u8"จ่ายค่าหัวอัตโนมัติ"));
		peacefulGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"ลบหรือจ่ายค่าหัวโดยอัตโนมัติ - เล่นอย่างสงบสุข!");
		}));
		peacefulGroup->AddItem(std::make_shared<CommandItem>("opennearbydoors"_J, u8"เปิดประตูใกล้เคียง"));
		peacefulGroup->AddItem(std::make_shared<CommandItem>("unlockalldoors"_J, u8"ปลดล็อคประตูทั้งหมด"));
		peacefulGroup->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::TextWrapped(u8"เปิดประตูที่ล็อค: ธนาคาร, ร้านค้า, คอกม้า - สำรวจได้ทุกที่!");
		}));
		peacefulGroup->AddItem(std::make_shared<BoolCommandItem>("chatbubble"_J, u8"ข้อความบนหัว"));
		peacefulGroup->AddItem(std::make_shared<ImGuiItem>([] {
			static char chatText[128] = "";
			ImGui::InputText(u8"ข้อความ", chatText, sizeof(chatText));
			if (ImGui::Button(u8"อัพเดทข้อความ"))
			{
				YimMenu::Features::GetChatBubbleText() = chatText;
			}
			ImGui::TextWrapped(u8"พิมพ์ข้อความเพื่อแสดงบนหัวของคุณ! เหมาะสำหรับการสื่อสารแบบรักสงบ!");
		}));

		creative->AddItem(visualsGroup);
		creative->AddItem(particlesGroup);
		creative->AddItem(playerEffectsGroup);
		creative->AddItem(worldGroup);
		creative->AddItem(companionsGroup);
		creative->AddItem(vehiclesGroup);
		creative->AddItem(ridablesGroup);
		creative->AddItem(explorationGroup);
		creative->AddItem(peacefulGroup);
		AddCategory(std::move(creative));
	}
}