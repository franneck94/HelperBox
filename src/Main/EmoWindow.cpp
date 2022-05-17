#include "stdafx.h"

#include <array>
#include <cstdint>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include <fmt/format.h>

#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <Helper.h>
#include <Player.h>
#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

#include "EmoWindow.h"

namespace
{
static auto state_emo_casting_routine = ModuleState::INACTIVE;
static auto state_tank_bonding_routine = ModuleState::INACTIVE;
static auto state_player_bonding_routine = ModuleState::INACTIVE;
static auto state_fuse_pull_routine = ModuleState::INACTIVE;

static constexpr auto MIN_FUSE_PULL_RANGE = float{1200.0F};
static constexpr auto MAX_FUSE_PULL_RANGE = float{1248.0F};
static constexpr auto FUSE_PULL_RANGE = float{1220.0F};

static const auto WINDOW_SIZE = ImVec2(120.0, 370.0);
static const auto BUTTON_SIZE = ImVec2(120.0, 80.0);

static const auto ACTIVE_COLOR = ImVec4{0.0F, 200.0F, 0.0F, 102.0F};
static const auto INACTIVE_COLOR = ImVec4{200.0F, 0.0F, 0.0F, 102.0F};
static const auto ON_HOLD_COLOR = ImVec4{255.0F, 226.0F, 0.0F, 102.0F};

static constexpr auto MIN_CYCLE_TIME_MS = uint32_t{100};
}; // namespace

void EmoWindow::Initialize()
{
    HelperBoxWindow::Initialize();
}

void EmoWindow::LoadSettings(CSimpleIni *ini)
{
    HelperBoxWindow::LoadSettings(ini);
    show_menubutton = true;
}

void EmoWindow::SaveSettings(CSimpleIni *ini)
{
    HelperBoxWindow::SaveSettings(ini);
}

void EmoWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
        return;

    if (!visible)
        return;

    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EmoWindow", nullptr, GetWinFlags()))
    {
        auto casting_button_text = "";
        auto casting_button_color = ImVec4{0.0F, 0.0F, 0.0F, 255.0F};

        switch (state_emo_casting_routine)
        {
        case ModuleState::ACTIVE:
        {
            casting_button_text = "Stop Pump";
            casting_button_color = ACTIVE_COLOR;

            break;
        }
        case ModuleState::ON_HOLD:
        {
            casting_button_text = "On Hold Pump";
            casting_button_color = ON_HOLD_COLOR;

            break;
        }
        case ModuleState::INACTIVE:
        default:
        {
            casting_button_text = "Start Pump";
            casting_button_color = INACTIVE_COLOR;

            break;
        }
        }

        auto bonding_tank_button_text = "Tank Bonds";
        auto bonding_player_button_text = "Player Bonds";
        auto fuse_pull_button_text = "Fuse Pull";

        ImGui::PushStyleColor(ImGuiCol_Button, casting_button_color);
        if (ImGui::Button(casting_button_text, BUTTON_SIZE))
        {
            if (IsExplorable())
            {
                state_emo_casting_routine = StateNegation(state_emo_casting_routine);
            }
        }
        ImGui::PopStyleColor();

        if (ImGui::Button(bonding_tank_button_text, BUTTON_SIZE))
        {
            if (IsExplorable())
            {
                state_tank_bonding_routine = StateNegation(state_tank_bonding_routine);
            }
        }

        if (ImGui::Button(bonding_player_button_text, BUTTON_SIZE))
        {
            if (IsExplorable())
            {
                state_player_bonding_routine = StateNegation(state_player_bonding_routine);
            }
        }

        if (ImGui::Button(fuse_pull_button_text, BUTTON_SIZE))
        {
            if (IsExplorable())
            {
                state_fuse_pull_routine = StateNegation(state_fuse_pull_routine);
            }
        }
    }

    ImGui::End();
}

void EmoWindow::EmoSkillRoutine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
    {
        return;
    }

    timer = TIMER_INIT();

    if (!player.CanCast())
    {
        return;
    }

    const auto &me_buffs = GW::Effects::GetPlayerBuffArray();
    const auto &me_effects = GW::Effects::GetPlayerEffectArray();

    bool found_balth = false;
    bool found_bond = false;

    for (size_t i = 0; i < me_buffs.size(); ++i)
    {
        const auto agent_id = me_buffs[i].target_agent_id;
        const auto skill_id = me_buffs[i].skill_id;

        if (agent_id == player.id)
        {
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Balthazars_Spirit))
            {
                found_balth = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Protective_Bond))
            {
                found_bond = true;
            }
        }
    }

    bool found_ether = false;
    bool found_sb = false;
    bool found_burning = false;

    for (size_t i = 0; i < me_effects.size(); ++i)
    {
        const auto agent_id = me_effects[i].agent_id;
        const auto skill_id = me_effects[i].skill_id;

        if (agent_id == player.id || agent_id == 0)
        {
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Ether_Renewal))
            {
                found_ether = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Spirit_Bond))
            {
                found_sb = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Burning_Speed))
            {
                found_burning = true;
            }
        }
    }

    if (player.skillbar.ether.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.ether.idx, player.id);
        return;
    }

    const bool balth_avail = player.skillbar.balth.CanBeCasted(player.energy);
    if (!found_balth && balth_avail)
    {
        SafeUseSkill(player.skillbar.balth.idx, player.id);
        return;
    }

    const bool bond_avail = player.skillbar.bond.CanBeCasted(player.energy);
    if (!found_bond && bond_avail)
    {
        SafeUseSkill(player.skillbar.bond.idx, player.id);
        return;
    }

    const bool sb_needed = (player.hp_perc < 0.900F) || !found_sb;
    const bool sb_avail = player.skillbar.sb.CanBeCasted(player.energy);
    if (found_ether && sb_needed && sb_avail)
    {
        SafeUseSkill(player.skillbar.sb.idx, player.id);
        return;
    }

    const bool need_burning = (player.energy_perc < 0.975F || !found_burning);
    const bool burning_avail = player.skillbar.burning.CanBeCasted(player.energy);
    if (found_ether && need_burning && burning_avail)
    {

        SafeUseSkill(player.skillbar.burning.idx, player.id);
        return;
    }
}

bool EmoWindow::EmoBondTankRoutine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
    {
        return false;
    }

    timer = TIMER_INIT();

    if (!player.CanCast())
    {
        return false;
    }

    if (!player.target || player.target->type != 0xDB)
    {
        return true;
    }

    const auto target_living = player.target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1)
    {
        return true;
    }

    const GW::AgentEffectsArray &effects = GW::Effects::GetPartyEffectArray();

    if (!effects.valid())
    {
        return true;
    }

    const auto &buffs = effects[0].buffs;

    if (!buffs.valid())
    {
        return true;
    }

    bool found_balth = false;
    bool found_bond = false;
    bool found_life = false;

    for (size_t i = 0; i < buffs.size(); ++i)
    {
        const auto agent_id = buffs[i].target_agent_id;
        const auto skill_id = buffs[i].skill_id;

        if (agent_id == player.target->agent_id)
        {
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Balthazars_Spirit))
            {
                found_balth = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Protective_Bond))
            {
                found_bond = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Life_Bond))
            {
                found_life = true;
            }
        }
    }

    if (!found_balth && player.skillbar.balth.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.balth.idx, player.target->agent_id);
        return false;
    }

    if (!found_bond && player.skillbar.bond.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.bond.idx, player.target->agent_id);
        return false;
    }

    if (!found_life && player.skillbar.life.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.life.idx, player.target->agent_id);
        return false;
    }

    return true;
}

bool EmoWindow::EmoBondPlayerRoutine()
{
    static auto timer = TIMER_INIT();
    const auto timer_diff = TIMER_DIFF(timer);

    if (timer_diff < MIN_CYCLE_TIME_MS)
    {
        return false;
    }

    timer = TIMER_INIT();

    if (!player.CanCast())
    {
        return false;
    }

    if (!player.target || player.target->type != 0xDB)
    {
        return true;
    }

    const auto target_living = player.target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1)
    {
        return true;
    }

    const GW::AgentEffectsArray &effects = GW::Effects::GetPartyEffectArray();

    if (!effects.valid())
    {
        return true;
    }

    const auto &buffs = effects[0].buffs;

    if (!buffs.valid())
    {
        return true;
    }

    bool found_balth = false;
    bool found_bond = false;

    for (size_t i = 0; i < buffs.size(); ++i)
    {
        const auto agent_id = buffs[i].target_agent_id;
        const auto skill_id = buffs[i].skill_id;

        if (agent_id == player.target->agent_id)
        {
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Balthazars_Spirit))
            {
                found_balth = true;
            }
            if (skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Protective_Bond))
            {
                found_bond = true;
            }
        }
    }

    if (!found_balth && player.skillbar.balth.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.balth.idx, player.target->agent_id);
        return false;
    }

    if (!found_bond && player.skillbar.bond.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.bond.idx, player.target->agent_id);
        return false;
    }

    return true;
}

bool EmoWindow::EmoFusePull()
{
    static ActionState state = ActionState::NONE;
    static GW::GamePos requested_pos = GW::GamePos{};
    ResetState(state);

    if (!player.target || player.target->type != 0xDB)
    {
        return true;
    }

    const auto target_living = player.target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1)
    {
        return true;
    }

    const auto me_pos = player.pos;
    const auto target_pos = player.target->pos;

    const auto d = GW::GetDistance(me_pos, target_pos);

    if ((state == ActionState::NONE) && (d < MIN_FUSE_PULL_RANGE || d > MAX_FUSE_PULL_RANGE))
    {
        requested_pos = GW::GamePos{};

        const auto m_x = me_pos.x;
        const auto m_y = me_pos.y;
        const auto t_x = target_pos.x;
        const auto t_y = target_pos.y;

        const auto d_t = FUSE_PULL_RANGE;
        const auto t = d_t / d;

        const auto p_x = ((1.0F - t) * t_x + t * m_x);
        const auto p_y = ((1.0F - t) * t_y + t * m_y);

        requested_pos = GW::GamePos{p_x, p_y, 0};
        state = SafeWalk(requested_pos);

        return false;
    }
    else if (state == ActionState::ACTIVE)
    {
        state = SafeWalk(requested_pos);
        return false;
    }

    state = ActionState::FINISHED;

    if (!player.CanCast())
    {
        return false;
    }

    if (player.skillbar.fuse.CanBeCasted(player.energy))
    {
        SafeUseSkill(player.skillbar.fuse.idx, player.target->agent_id);
    }

    return true;
}

void EmoWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);

    if (!player.ValidateData())
    {
        return;
    }

    player.Update();

    if (state_tank_bonding_routine == ModuleState::ACTIVE)
    {
        const auto done = EmoBondTankRoutine();

        if (done)
        {
            state_tank_bonding_routine = ModuleState::INACTIVE;
        }
    }

    if (state_player_bonding_routine == ModuleState::ACTIVE)
    {
        const auto done = EmoBondPlayerRoutine();

        if (done)
        {
            state_player_bonding_routine = ModuleState::INACTIVE;
        }
    }

    if (state_fuse_pull_routine == ModuleState::ACTIVE)
    {
        if (state_emo_casting_routine == ModuleState::ACTIVE)
        {
            state_emo_casting_routine = ModuleState::ON_HOLD;
        }

        const auto done = EmoFusePull();

        if (done)
        {
            state_fuse_pull_routine = ModuleState::INACTIVE;

            if (state_emo_casting_routine == ModuleState::ON_HOLD)
            {
                state_emo_casting_routine = ModuleState::ACTIVE;
            }
        }
    }

    if (state_emo_casting_routine == ModuleState::ACTIVE)
    {
        EmoSkillRoutine();
    }
}