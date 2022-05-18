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
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/PlayerMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Packets/Opcodes.h>

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

static const auto ACTIVE_COLOR = ImVec4{0.0F, 200.0F, 0.0F, 80.0F};
static const auto INACTIVE_COLOR = ImVec4{41.0F, 74.0F, 122.0F, 80.0F};
static const auto ON_HOLD_COLOR = ImVec4{255.0F, 226.0F, 0.0F, 80.0F};

static constexpr auto MIN_CYCLE_TIME_MS = uint32_t{100};

static auto COLOR_MAPPING = std::map<uint32_t, ImVec4>{{static_cast<uint32_t>(ModuleState::INACTIVE), INACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ModuleState::ACTIVE), ACTIVE_COLOR},
                                                       {static_cast<uint32_t>(ModuleState::ON_HOLD), ON_HOLD_COLOR}};
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

void DrawButton(ModuleState &state, const ImVec4 color, std::string_view text)
{
    bool pushed_style = false;

    if (state != ModuleState::INACTIVE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        pushed_style = true;
    }

    if (ImGui::Button(text.data(), BUTTON_SIZE))
    {
        if (IsExplorable())
        {
            state = StateNegation(state);
        }
    }
    if (pushed_style)
        ImGui::PopStyleColor();
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
        const auto casting_button_text = "Pumping";
        const auto casting_button_color = COLOR_MAPPING[static_cast<uint32_t>(state_emo_casting_routine)];
        DrawButton(state_emo_casting_routine, casting_button_color, casting_button_text);

        const auto bonding_tank_button_text = "Tank Bonds";
        const auto tank_bonding_button_color = COLOR_MAPPING[static_cast<uint32_t>(state_tank_bonding_routine)];
        DrawButton(state_tank_bonding_routine, tank_bonding_button_color, bonding_tank_button_text);

        const auto bonding_player_button_text = "Player Bonds";
        const auto player_bonding_button_color = COLOR_MAPPING[static_cast<uint32_t>(state_player_bonding_routine)];
        DrawButton(state_player_bonding_routine, player_bonding_button_color, bonding_player_button_text);

        const auto fuse_pull_button_text = "Fuse Pull";
        const auto fuse_pull_button_color = COLOR_MAPPING[static_cast<uint32_t>(state_fuse_pull_routine)];
        DrawButton(state_fuse_pull_routine, fuse_pull_button_color, fuse_pull_button_text);
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

    const bool found_balth = player.HasBuff(GW::Constants::SkillID::Balthazars_Spirit);
    const bool found_bond = player.HasBuff(GW::Constants::SkillID::Protective_Bond);

    const bool found_ether = player.HasEffect(GW::Constants::SkillID::Ether_Renewal);
    const bool found_sb = player.HasEffect(GW::Constants::SkillID::Spirit_Bond);
    const bool found_burning = player.HasEffect(GW::Constants::SkillID::Burning_Speed);

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

    const bool sb_needed = (player.hp_perc < 0.90F) || !found_sb;
    const bool sb_avail = player.skillbar.sb.CanBeCasted(player.energy);
    if (found_ether && sb_needed && sb_avail)
    {
        SafeUseSkill(player.skillbar.sb.idx, player.id);
        return;
    }

    const bool need_burning = (player.energy_perc < 0.905F || !found_burning);
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

    // Expect that the emo is last and tank is second last in party
    if (!player.target)
    {
        std::vector<PlayerMapping> party_members;

        bool success = GetPartyMembers(party_members);

        if (party_members.size() < 2)
        {
            return true;
        }

        const auto tank = party_members[party_members.size() - 2];
        player.ChangeTarget(tank.id);

        if (!success || !player.target)
        {
            return true;
        }
    }

    if (player.target->type != 0xDB)
    {
        return true;
    }

    const auto target_living = player.target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
    {
        return true;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player.target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player.target->agent_id);
    const bool found_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, player.target->agent_id);

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

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
    {
        return true;
    }

    const bool found_balth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, player.target->agent_id);
    const bool found_bond = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, player.target->agent_id);

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
    static auto timer = TIMER_INIT();
    static ActionState state = ActionState::NONE;
    static GW::GamePos requested_pos = GW::GamePos{};
    static GW::GamePos last_pos = GW::GamePos{};
    static uint32_t stuck_counter = 0;
    static uint32_t step = 0;
    ResetState(state);

    if (!player.target || player.target->type != 0xDB)
    {
        return true;
    }

    const auto target_living = player.target->GetAsAgentLiving();

    if (target_living->allegiance != 0x1 || target_living->GetIsDead())
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
        last_pos = player.pos;

        return false;
    }
    else if (state == ActionState::ACTIVE && !(me_pos == requested_pos))
    {
        // Stuck, or somehow walking canceled
        if (last_pos == GW::GamePos{} || last_pos == me_pos)
        {
            ++stuck_counter;

            if (stuck_counter >= 25)
            {
                state = ActionState::FINISHED;
                stuck_counter = 0;
                last_pos = GW::GamePos{};
                requested_pos = GW::GamePos{};
                timer = TIMER_INIT();
                return true;
            }
        }
        last_pos = player.pos;

        return false;
    }

    const auto timer_diff = TIMER_DIFF(timer);

    if (step == 0)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return false;
    }
    if (timer_diff > 400)
    {
        timer = TIMER_INIT();
    }
    else
    {
        return false;
    }

    if (step == 1)
    {
        ChangeFullArmor(5, 16);
        ++step;
        return false;
    }
    if (timer_diff > 400)
    {
        timer = TIMER_INIT();
    }
    else
    {
        return false;
    }

    if (step == 2)
    {
        if (!player.CanCast())
        {
            return false;
        }

        if (player.skillbar.fuse.CanBeCasted(player.energy))
        {
            SafeUseSkill(player.skillbar.fuse.idx, player.target->agent_id);
        }
        ++step;
        return false;
    }
    if (timer_diff > 800)
    {
        timer = TIMER_INIT();
    }
    else
    {
        return false;
    }

    if (step == 3)
    {
        GW::CtoS::SendPacket(0x4, GAME_CMSG_CANCEL_MOVEMENT);
        ++step;

        return false;
    }
    if (timer_diff > 400)
    {
        timer = TIMER_INIT();
    }
    else
    {
        return false;
    }

    if (step == 4)
    {
        ChangeFullArmor(5, 16);
        ++step;

        return false;
    }
    if (timer_diff > 400)
    {
        timer = TIMER_INIT();
    }
    else
    {
        return false;
    }

    state = ActionState::FINISHED;
    stuck_counter = 0;
    last_pos = GW::GamePos{};
    requested_pos = GW::GamePos{};
    timer = TIMER_INIT();

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
        StateOnHold(state_emo_casting_routine);
        const auto done = EmoBondTankRoutine();

        if (done)
        {
            state_tank_bonding_routine = ModuleState::INACTIVE;
            StateOnActive(state_emo_casting_routine);
        }
    }

    if (state_player_bonding_routine == ModuleState::ACTIVE)
    {
        StateOnHold(state_emo_casting_routine);
        const auto done = EmoBondPlayerRoutine();

        if (done)
        {
            state_player_bonding_routine = ModuleState::INACTIVE;
            StateOnActive(state_emo_casting_routine);
        }
    }

    if (state_fuse_pull_routine == ModuleState::ACTIVE)
    {
        StateOnHold(state_emo_casting_routine);
        const auto done = EmoFusePull();

        if (done)
        {
            state_fuse_pull_routine = ModuleState::INACTIVE;
            StateOnActive(state_emo_casting_routine);
        }
    }

    if (state_emo_casting_routine == ModuleState::ACTIVE)
    {
        EmoSkillRoutine();
    }
}
