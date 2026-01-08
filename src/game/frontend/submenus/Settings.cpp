#include "settings.hpp"

#include "core/commands/Commands.hpp"
#include "core/commands/HotkeySystem.hpp"
#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/features/Features.hpp"
#include "game/frontend/items/Items.hpp"

namespace YimMenu::Submenus
{
    // Refactored Hotkeys function
    static void DrawHotkeySettings()
    {
        ImGui::BulletText(u8"คลิกค้างที่ชื่อคำสั่งเพื่อเปลี่ยนคีย์ลัด");
        ImGui::BulletText(u8"กดปุ่มที่ตั้งค่าไว้เพื่อลบออก");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        for (auto& [Hash, Command] : Commands::GetCommands())
        {
            ImGui::PushID(Hash);

            if (g_HotkeySystem.m_CommandHotkeys.find(Hash) != g_HotkeySystem.m_CommandHotkeys.end())
                HotkeySetter(Hash).Draw();

            ImGui::Spacing();
            ImGui::PopID();
        }
    }

    Settings::Settings() :
        Submenu::Submenu("Settings")
    {
        auto hotkeys           = std::make_shared<Category>(u8"คีย์ลัด");
        auto gui               = std::make_shared<Category>(u8"หน้าจอ");
        auto protections       = std::make_shared<Category>(u8"ป้องกัน");
        auto syncGroup         = std::make_shared<Group>(u8"ซิงค์");
        auto networkEventGroup = std::make_shared<Group>(u8"อีเวนต์เครือข่าย");
        auto scriptEventGroup  = std::make_shared<Group>(u8"สคริปต์อีเวนต์");
        auto playerEsp        = std::make_shared<Group>(u8"มองทะลุผู้เล่น", 10);
        auto pedEsp           = std::make_shared<Group>(u8"มองทะลุ Ped", 10);
        auto overlay          = std::make_shared<Group>(u8"หน้าต่างข้อมูล");
        auto context           = std::make_shared<Group>(u8"เมนูคลิกขวา");
        auto misc              = std::make_shared<Group>(u8"อื่นๆ");

        // Hotkeys
        hotkeys->AddItem(std::make_shared<ImGuiItem>(DrawHotkeySettings));

        // Players
        playerEsp->AddItem(std::make_shared<BoolCommandItem>("espdrawplayers"_J, u8"แสดงเส้นผู้เล่น"));
        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<BoolCommandItem>("espdrawdeadplayers"_J, u8"แสดงคนตาย")));

        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<BoolCommandItem>("espnameplayers"_J, u8"ชื่อผู้เล่น")));
        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<ColorCommandItem>("namecolorplayers"_J, u8"สีชื่อ")));

        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<BoolCommandItem>("espdistanceplayers"_J, u8"ระยะห่างผู้เล่น")));
        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<ColorCommandItem>("distancecolorplayers"_J, u8"สีระยะ"))); // Implemented TODO

        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<BoolCommandItem>("espskeletonplayers"_J, u8"โครงกระดูกผู้เล่น")));
        playerEsp->AddItem(std::make_shared<ConditionalItem>("espdrawplayers"_J, std::make_shared<ColorCommandItem>("skeletoncolorplayers"_J, u8"สีโครงกระดูก")));

        // Peds
        pedEsp->AddItem(std::make_shared<BoolCommandItem>("espdrawpeds"_J, u8"แสดงเส้น Ped"));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espdrawdeadpeds"_J, u8"แสดง Ped ตาย")));

        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espmodelspeds"_J, u8"รหัสโมเดล Ped")));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<ColorCommandItem>("hashcolorpeds"_J, u8"สีโมเดล")));

        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espnetinfopeds"_J, "Ped Net Info")));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espscriptinfopeds"_J, "Ped Script Info")));

        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espdistancepeds"_J, u8"ระยะห่าง Ped")));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<ColorCommandItem>("distancecolorpeds"_J, u8"สีระยะ"))); // Implemented TODO

        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espskeletonpeds"_J, u8"โครงกระดูก Ped")));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<ColorCommandItem>("skeletoncolorpeds"_J, u8"สีโครงกระดูก")));

        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<BoolCommandItem>("espskeletonhorse"_J, u8"โครงกระดูกม้า")));
        pedEsp->AddItem(std::make_shared<ConditionalItem>("espdrawpeds"_J, std::make_shared<ColorCommandItem>("skeletoncolorhorse"_J, u8"สีโครงกระดูกม้า")));

        // Overlay
        overlay->AddItem(std::make_shared<BoolCommandItem>("overlay"_J, u8"เปิดใช้งาน"));
        overlay->AddItem(std::make_shared<ConditionalItem>("overlay"_J, std::make_shared<BoolCommandItem>("overlayfps"_J, u8"แสดง FPS")));
        overlay->AddItem(std::make_shared<ConditionalItem>("overlay"_J, std::make_shared<BoolCommandItem>("overlayprotections"_J, u8"แสดงสถิติการป้องกัน")));

        // Context Menu
        context->AddItem(std::make_shared<BoolCommandItem>("ctxmenu"_J, u8"เปิดใช้งาน"));
        context->AddItem(std::make_shared<ConditionalItem>("ctxmenu"_J, std::make_shared<BoolCommandItem>("ctxmenuplayers"_J, u8"ผู้เล่น")));
        context->AddItem(std::make_shared<ConditionalItem>("ctxmenu"_J, std::make_shared<BoolCommandItem>("ctxmenupeds"_J, "Peds")));
        context->AddItem(std::make_shared<ConditionalItem>("ctxmenu"_J, std::make_shared<BoolCommandItem>("ctxmenuvehicles"_J, u8"ยานพาหนะ")));
        context->AddItem(std::make_shared<ConditionalItem>("ctxmenu"_J, std::make_shared<BoolCommandItem>("ctxmenuobjects"_J, u8"วัตถุ")));

        // Sync Group
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockspectate"_J, u8"บล็อคการส่อง"));
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockspectatesession"_J, u8"บล็อคการส่อง (Session)"));
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockattach"_J, u8"บล็อคการเกาะติด"));
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockvehflood"_J, u8"บล็อคฟลัดยานพาหนะ"));
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockentityflood"_J, u8"บล็อคฟลัด Entity"));
        syncGroup->AddItem(std::make_shared<BoolCommandItem>("blockeventflood"_J, u8"บล็อคฟลัดอีเวนต์"));

        // Network Event Group
        networkEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockexplosions"_J, u8"บล็อคระเบิด"));
        networkEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockptfx"_J, u8"บล็อคเอฟเฟกต์ PTFX"));
        networkEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockclearpedtasks"_J, u8"บล็อคการลบ Task"));
        networkEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockscriptcommand"_J, u8"บล็อคคำสั่งสคริปต์"));
        networkEventGroup->AddItem(std::make_shared<BoolCommandItem>("userelaycxns"_J, u8"ใช้ Relay Connections"));

        // Script Event Group
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockhonormanipulation"_J, u8"บล็อคการปรับ Honor"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockdefensive"_J, u8"บล็อคโหมดป้องกัน"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockoffensive"_J, u8"บล็อคโหมดโจมตี"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockpresscharges"_J, u8"บล็อคการแจ้งความ"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockstartparlay"_J, u8"บล็อคการเริ่ม Parlay"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockendparlay"_J, u8"บล็อคการจบ Parlay"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blocktickerspam"_J, u8"บล็อคสแปมข้อความ"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockstableevents"_J, u8"บล็อคอีเวนต์คอกม้า"));
        scriptEventGroup->AddItem(std::make_shared<BoolCommandItem>("blockkickfrommissionlobby"_J, u8"บล็อคเตะจากภารกิจ"));

        // Add categories
        gui->AddItem(playerEsp);
        gui->AddItem(pedEsp);
        gui->AddItem(overlay);
        gui->AddItem(context);
        gui->AddItem(misc);

        protections->AddItem(syncGroup);
        protections->AddItem(networkEventGroup);
        protections->AddItem(scriptEventGroup);

        AddCategory(std::move(hotkeys));
        AddCategory(std::move(gui));
        AddCategory(std::move(protections));
    }
}