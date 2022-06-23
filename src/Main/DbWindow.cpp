#include "stdafx.h"

#include <cmath>
#include <cstdint>
#include <vector>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Maps.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/Context/GameContext.h>
#include <GWCA/GameContainers/GamePos.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Player.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/CtoSMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Packets/Opcodes.h>

#include <GuiUtils.h>
#include <HelperBox.h>
#include <Logger.h>
#include <Timer.h>

#include <Actions.h>
#include <GuiUtils.h>
#include <Helper.h>
#include <MathUtils.h>
#include <Player.h>
#include <SafeFuncs.h>
#include <Skillbars.h>
#include <Types.h>
#include <UwHelper.h>

#include "DbWindow.h"

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

namespace
{
static ActionState *damage_action_state = nullptr;
static auto move_ongoing = false;
}; // namespace

DbWindow::DbWindow() : player({}), skillbar({}), damage(&player, &skillbar)
{
    if (skillbar.ValidateData())
        skillbar.Load();

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
        &MapLoaded_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::MapLoaded *packet) -> void {
            load_cb_triggered = ExplorableLoadCallback(status, packet);
        });

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
        &GenericValue_Entry,
        [this](GW::HookStatus *status, GW::Packet::StoC::GenericValue *packet) -> void {
            UNREFERENCED_PARAMETER(status);
            if (move_ongoing && player.SkillStoppedCallback(packet))
                interrupted = true;
        });
};

void DbWindow::DrawMap()
{
    ImGui::SetNextWindowSize(ImVec2{400.0F, 400.0F}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("PlottingWindow", nullptr, ImGuiWindowFlags_None))
    {
        if (ImPlot::BeginPlot("Plotting", ImVec2{400.0F, 400.0F}, ImPlotFlags_None | ImPlotFlags_NoLegend))
        {
            auto next_pos = GW::GamePos{};

            if (move_idx < moves.size() - 1U && moves[move_idx].move_state == MoveState::WAIT)
                next_pos = moves[move_idx + 1U].pos;
            else
                next_pos = moves[move_idx].pos;

            const auto rect = GameRectangle(player.pos, next_pos, GW::Constants::Range::Spellcast);

            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

            const auto x_ = player.pos.x;
            const auto y_ = player.pos.y;
            plot_point(GW::GamePos{x_ + 4000.0F, y_, 0}, "border1", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_, y_ + 4000.0F, 0}, "border2", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_ - 4000.0F, y_, 0}, "border3", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);
            plot_point(GW::GamePos{x_, y_ - 4000.0F, 0}, "border4", ImVec4{0.0F, 0.0F, 0.0F, 0.0F}, 1.0F);

            plot_point(player.pos, "player", ImVec4{1.0F, 1.0F, 1.0F, 1.0F}, 5.0F);
            plot_point(next_pos, "target", ImVec4{0.5F, 0.5F, 0.0F, 1.0F}, 5.0F);

            plot_rectangle_line(rect.v1, rect.v2, "line1");
            plot_rectangle_line(rect.v1, rect.v3, "line2");
            plot_rectangle_line(rect.v4, rect.v2, "line3");
            plot_rectangle_line(rect.v4, rect.v3, "line4");

            plot_circle(player, "circle", ImVec4{0.0, 0.0, 1.0, 1.0});

            const auto living_agents = GetEnemiesInCompass();
            plot_enemies(living_agents, "enemiesAll", ImVec4{0.0, 1.0, 0.0, 1.0});

            const auto filtered_livings = GetEnemiesInGameRectangle(rect);
            plot_enemies(filtered_livings, "enemyInside", ImVec4{1.0, 0.0, 0.0, 1.0});
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}

void DbWindow::Draw(IDirect3DDevice9 *pDevice)
{
    UNREFERENCED_PARAMETER(pDevice);

    if (!visible)
        return;

    if (!ActivationConditions())
        return;

    if ((!IsUw() && !IsUwEntryOutpost()))
        return;

    ImGui::SetNextWindowSize(ImVec2(110.0F, 170.0F), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("DbWindow", nullptr, GetWinFlags()))
    {
        damage.Draw();
        DrawMovingButtons(moves, move_ongoing, move_idx);
    }
    ImGui::End();

    if (IsUw())
        DrawMap();
}

void DbWindow::UpdateUw()
{
    UpdateUwEntry();
    UpdateUwMoves();
}

void DbWindow::UpdateUwEntry()
{
    if (load_cb_triggered)
    {
        moves[0].Execute();
        load_cb_triggered = false;
        move_ongoing = true;
    }
}

void DbWindow::UpdateUwMoves()
{
    UpdatedUwMoves_Main(player, moves, move_idx, move_ongoing);
}

void DbWindow::Update(float delta)
{
    UNREFERENCED_PARAMETER(delta);
    static auto last_pos = player.pos;

    if (!player.ValidateData())
        return;
    player.Update();

    if (!ActivationConditions())
        return;

    if (IsLoading() || IsOutpost())
        move_idx = 0;

    if (!skillbar.ValidateData())
        return;
    skillbar.Update();

    damage_action_state = &damage.action_state;

    if (IsUw())
    {
        UpdateUw();

        const auto curr_pos = player.pos;
        const auto dist = GW::GetDistance(last_pos, curr_pos);
        if (dist > 5'000.0F)
        {
            Log::Info("Ported!");
            move_idx = GetClostestMove(player, moves);
        }
        last_pos = curr_pos;
    }

    damage.Update();
}

bool DbWindow::ActivationConditions() const
{
    if (!GW::Map::GetIsMapLoaded())
        return false;

    if (!GW::PartyMgr::GetIsPartyLoaded())
        return false;

    if (!IsDhuumBitch(player))
        return false;

    if (IsUwEntryOutpost() || IsUw())
        return true;

    return false;
}

Damage::Damage(Player *p, DbSkillbar *s) : DbActionABC(p, "Damage", s)
{
}

RoutineState Damage::RoutineAtChamberSkele() const
{
    const auto sos_state = skillbar->sos.Cast(player->energy);
    if (sos_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto pi_state = skillbar->pi.Cast(player->energy);
    if (pi_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutineValeSpirits() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_eoe = player->HasEffect(GW::Constants::SkillID::Edge_of_Extinction);
    const auto found_winnow = player->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor)
    {
        const auto honor_state = skillbar->honor.Cast(player->energy);
        if (honor_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto sos_state = skillbar->sos.Cast(player->energy);
    if (sos_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    if (!found_eoe)
    {
        const auto eoe_state = skillbar->eoe.Cast(player->energy);
        if (eoe_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_winnow)
    {
        const auto winnow_state = skillbar->winnow.Cast(player->energy);
        if (winnow_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutinePI(const uint32_t dhuum_id) const
{
    const auto &skill = skillbar->pi;

    if (!skill.SkillFound())
        return RoutineState::ACTIVE;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return RoutineState::ACTIVE;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return RoutineState::ACTIVE;

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
        return skillbar->pi.Cast(player->energy, dhuum_id);

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutineDhuumRecharge() const
{
    static auto qz_timer = clock();

    const auto found_qz = player->HasEffect(GW::Constants::SkillID::Quickening_Zephyr);

    const auto qz_diff_ms = TIMER_DIFF(qz_timer);
    if (qz_diff_ms > 36'000 || !found_qz)
    {
        if (!found_qz)
        {
            const auto sq_state = skillbar->sq.Cast(player->energy);
            if (sq_state == RoutineState::FINISHED)
                return RoutineState::FINISHED;
        }

        const auto qz_state = skillbar->qz.Cast(player->energy);
        if (qz_state == RoutineState::FINISHED)
        {
            qz_timer = clock();
            return RoutineState::FINISHED;
        }
    }

    return RoutineState::ACTIVE;
}

RoutineState Damage::RoutineDhuumDamage() const
{
    const auto found_honor = player->HasEffect(GW::Constants::SkillID::Ebon_Battle_Standard_of_Honor);
    const auto found_winnow = player->HasEffect(GW::Constants::SkillID::Winnowing);

    if (!found_honor)
    {
        const auto honor_state = skillbar->honor.Cast(player->energy);
        if (honor_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (!found_winnow)
    {
        const auto winnow_state = skillbar->winnow.Cast(player->energy);
        if (winnow_state == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto sos_state = skillbar->sos.Cast(player->energy);
    if (sos_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::ACTIVE;
}

RoutineState Damage::Routine()
{
    static auto was_in_dhuum_fight = false;

    if (!player->CanCast())
        return RoutineState::FINISHED;

    if (!IsUw())
        return RoutineState::FINISHED;

    if (IsAtChamberSkele(player))
    {
        const auto chamber_rota = RoutineAtChamberSkele();
        if (chamber_rota == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    if (IsInVale(player))
    {
        if (!player->living->GetIsAttacking() && player->CanAttack())
        {
            if (!player->target || !player->target->agent_id || !player->target->GetIsLivingType())
                TargetNearest(TargetType::Living_Enemy, GW::Constants::Range::Spellcast);
            if (player->target && player->target->agent_id)
                AttackAgent(player->target);
        }

        const auto vale_rota = RoutineValeSpirits();
        if (vale_rota == RoutineState::FINISHED)
            return RoutineState::FINISHED;
    }

    const auto is_in_dhuum_room = IsInDhuumRoom(player);
    if (!is_in_dhuum_room)
        return RoutineState::FINISHED;

    auto dhuum_id = uint32_t{0};
    auto dhuum_hp = float{1.0F};
    const auto is_in_dhuum_fight = IsInDhuumFight(&dhuum_id, &dhuum_hp);

    if (was_in_dhuum_fight && !is_in_dhuum_fight)
        action_state = ActionState::INACTIVE;
    if (!is_in_dhuum_fight || !dhuum_id || dhuum_hp > 0.995F)
        return RoutineState::FINISHED;
    was_in_dhuum_fight = true;

    const auto reachrge_state = RoutineDhuumRecharge();
    if (reachrge_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    if (dhuum_hp < 0.25F)
        return RoutineState::FINISHED;

    player->ChangeTarget(dhuum_id);
    if (!player->living->GetIsAttacking() && player->target && player->target->agent_id)
        AttackAgent(player->target);

    const auto pi_state = RoutinePI(dhuum_id);
    if (pi_state == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    const auto dhuum_rota = RoutineDhuumDamage();
    if (dhuum_rota == RoutineState::FINISHED)
        return RoutineState::FINISHED;

    return RoutineState::FINISHED;
}

bool Damage::PauseRoutine()
{
    if (player->target)
    {
        const auto dist = GW::GetDistance(player->pos, player->target->pos);

        if (TargetIsReaper(*player) && (dist < 300.0F))
            return true;
    }

    if (player->living->GetIsMoving())
        return true;

    return false;
}

bool Damage::ResumeRoutine()
{
    return !PauseRoutine();
}

void Damage::Update()
{
    static auto paused = false;

    if (GW::PartyMgr::GetIsPartyDefeated())
        action_state = ActionState::INACTIVE;

    if (action_state == ActionState::ACTIVE && PauseRoutine())
    {
        paused = true;
        action_state = ActionState::ON_HOLD;
    }

    if (action_state == ActionState::ON_HOLD && ResumeRoutine())
    {
        paused = false;
        action_state = ActionState::ACTIVE;
    }

    if (action_state == ActionState::ACTIVE)
        (void)Routine();
}
