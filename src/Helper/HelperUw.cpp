
#include <algorithm>
#include <cstdint>
#include <vector>

#include <GWCA/Constants/Maps.h>
#include <GWCA/Context/CharContext.h>
#include <GWCA/GameContainers/Array.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <ActionsBase.h>
#include <DataPlayer.h>
#include <Helper.h>
#include <HelperAgents.h>
#include <HelperUwPos.h>
#include <UtilsMath.h>

#include "HelperUw.h"

bool UwHelperActivationConditions()
{
    if (!HelperActivationConditions())
        return false;

    if (!IsUwEntryOutpost() && !IsUw())
        return false;

    return true;
}


uint32_t GetTankId()
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);
    if (!success || party_members.size() < 2)
        return 0;

    auto tank_idx = uint32_t{0};
    switch (GW::Map::GetMapID())
    {
    case GW::Constants::MapID::The_Underworld:
#ifdef _DEBUG
    case GW::Constants::MapID::Isle_of_the_Nameless:
#endif
    {
        tank_idx = party_members.size() - 2;
        break;
    }
    default:
    {
        tank_idx = 0;
        break;
    }
    }

    const auto tank = party_members[tank_idx];
    return tank.id;
}

uint32_t GetEmoId()
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);
    if (!success)
        return 0;

    for (const auto &member : party_members)
    {
        const auto agent = GW::Agents::GetAgentByID(member.id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->primary == static_cast<uint8_t>(GW::Constants::Profession::Elementalist) &&
            living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Monk))
            return agent->agent_id;
    }

    return 0;
}

uint32_t GetDhuumBitchId()
{
    std::vector<PlayerMapping> party_members;
    const auto success = GetPartyMembers(party_members);
    if (!success)
        return 0;

    for (const auto &member : party_members)
    {
        const auto agent = GW::Agents::GetAgentByID(member.id);
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Ranger))
            return agent->agent_id;
    }

    return 0;
}

bool IsEmo(const DataPlayer &player_data)
{
    return (player_data.primary == GW::Constants::Profession::Elementalist &&
            player_data.secondary == GW::Constants::Profession::Monk);
}

bool IsDhuumBitch(const DataPlayer &player_data)
{
    return ((player_data.primary == GW::Constants::Profession::Ritualist ||
             player_data.primary == GW::Constants::Profession::Dervish) &&
            player_data.secondary == GW::Constants::Profession::Ranger);
}

bool IsUwMesmer(const DataPlayer &player_data)
{
    return (player_data.primary == GW::Constants::Profession::Mesmer &&
            (player_data.secondary == GW::Constants::Profession::Ranger ||
             player_data.secondary == GW::Constants::Profession::Elementalist ||
             player_data.secondary == GW::Constants::Profession::Assassin));
}

bool IsSpiker(const DataPlayer &player_data)
{
    return (player_data.primary == GW::Constants::Profession::Mesmer &&
            player_data.secondary == GW::Constants::Profession::Ranger);
}

bool IsLT(const DataPlayer &player_data)
{
    if (player_data.primary == GW::Constants::Profession::Mesmer &&
        player_data.secondary == GW::Constants::Profession::Assassin)
        return true;

    // Check if Me/E has Mantra of Earth => T4 build
    const auto skillbar = GW::SkillbarMgr::GetPlayerSkillbar();
    if (skillbar)
    {
        for (const auto skill : skillbar->skills)
        {
            if (skill.skill_id == static_cast<uint32_t>(GW::Constants::SkillID::Mantra_of_Earth))
                return false;
        }
    }

    return (player_data.primary == GW::Constants::Profession::Mesmer &&
            player_data.secondary == GW::Constants::Profession::Elementalist);
}

bool IsRangerTerra(const DataPlayer &player_data)
{
    return (player_data.primary == GW::Constants::Profession::Ranger &&
            player_data.secondary == GW::Constants::Profession::Assassin);
}

bool IsMesmerTerra(const DataPlayer &player_data)
{
    if (player_data.primary != GW::Constants::Profession::Mesmer ||
        player_data.secondary != GW::Constants::Profession::Elementalist)
        return false;

    return !IsLT(player_data);
}

const GW::Agent *GetDhuumAgent()
{
    const auto agents_array = GW::Agents::GetAgentArray();
    const GW::Agent *dhuum_agent = nullptr;

    for (const auto agent : *agents_array)
    {
        if (!agent)
            continue;

        const auto living = agent->GetAsAgentLiving();
        if (!living)
            continue;

        if (living->player_number == static_cast<uint16_t>(GW::Constants::ModelID::UW::Dhuum))
        {
            dhuum_agent = agent;
            break;
        }
    }

    return dhuum_agent;
}

bool IsInDhuumFight(uint32_t *dhuum_id, float *dhuum_hp, uint32_t *dhuum_max_hp)
{
    if (!IsUw())
        return false;

    const auto dhuum_agent = GetDhuumAgent();
    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    if (dhuum_id)
        *dhuum_id = dhuum_living->agent_id;

    if (dhuum_living->allegiance == GW::Constants::Allegiance::Enemy)
    {
        if (dhuum_hp)
            *dhuum_hp = dhuum_living->hp;
        if (dhuum_max_hp)
            *dhuum_max_hp = dhuum_living->max_hp;
        return true;
    }

    return false;
}

bool TankIsFullteamLT()
{
    const auto lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);
    if (!lt_agent)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (lt_living->primary == static_cast<uint8_t>(GW::Constants::Profession::Mesmer) &&
        lt_living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Assassin))
        return true;

    return false;
}

bool TankIsSoloLT()
{
    const auto lt_id = GetTankId();
    if (!lt_id)
        return false;

    const auto lt_agent = GW::Agents::GetAgentByID(lt_id);
    if (!lt_agent)
        return false;

    const auto lt_living = lt_agent->GetAsAgentLiving();
    if (!lt_living)
        return false;

    if (lt_living->primary == static_cast<uint8_t>(GW::Constants::Profession::Mesmer) &&
        lt_living->secondary == static_cast<uint8_t>(GW::Constants::Profession::Elementalist))
        return true;

    return false;
}

bool TargetIsReaper(DataPlayer &player_data)
{
    if (!player_data.target)
        return false;

    const auto living_target = player_data.target->GetAsAgentLiving();
    if (!living_target || living_target->player_number != static_cast<uint32_t>(GW::Constants::ModelID::UW::Reapers))
        return false;

    return true;
}

bool TargetReaper(DataPlayer &player_data, const std::vector<GW::AgentLiving *> &npcs)
{
    return TargetClosestNpcById(player_data, npcs, GW::Constants::ModelID::UW::Reapers) != 0U;
}

bool TalkReaper(DataPlayer &player_data, const std::vector<GW::AgentLiving *> &npcs)
{
    const auto id = TargetClosestNpcById(player_data, npcs, GW::Constants::ModelID::UW::Reapers);
    if (!id)
        return true;

    const auto agent = GW::Agents::GetAgentByID(id);
    if (!agent)
        return true;

    GW::Agents::GoNPC(agent, 0U);

    return true;
}

bool TargetClosestKeeper(DataPlayer &player_data, const std::vector<GW::AgentLiving *> enemies)
{
    return TargetClosestEnemyById(player_data, enemies, GW::Constants::ModelID::UW::KeeperOfSouls) != 0;
}

bool AcceptChamber()
{
    const auto dialog = QuestRewardDialog(GW::Constants::QuestID::UW::Chamber);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeRestore()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Restore);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeEscort()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Escort);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakeUWG()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::UWG);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakePits()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Pits);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool TakePlanes()
{
    const auto dialog = QuestAcceptDialog(GW::Constants::QuestID::UW::Planes);
    GW::Agents::SendDialog(dialog);
    return true;
}

bool FoundKeeperAtPos(const std::vector<GW::AgentLiving *> &keeper_livings, const GW::GamePos &keeper_pos)
{
    auto found_keeper = false;

    for (const auto keeper : keeper_livings)
    {
        if (GW::GetDistance(keeper->pos, keeper_pos) < GW::Constants::Range::Earshot)
        {
            found_keeper = true;
            break;
        }
    }

    return found_keeper;
}

bool DhuumIsCastingJudgement(const uint32_t dhuum_id)
{
    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    if (dhuum_living->GetIsCasting() && dhuum_living->skill == static_cast<uint32_t>(3085))
        return true;

    return false;
}

bool CheckForAggroFree(const DataPlayer &player_data, const AgentLivingData *livings_data, const GW::GamePos &next_pos)
{
    if (!livings_data)
        return true;

    const auto filter_ids =
        std::set<uint32_t>{GW::Constants::ModelID::UW::SkeletonOfDhuum1, GW::Constants::ModelID::UW::SkeletonOfDhuum2};
    const auto filter_ids_full = std::set<uint32_t>{GW::Constants::ModelID::UW::SkeletonOfDhuum1,
                                                    GW::Constants::ModelID::UW::SkeletonOfDhuum2,
                                                    GW::Constants::ModelID::UW::BladedAatxe,
                                                    GW::Constants::ModelID::UW::GraspingDarkness};

    const auto livings = FilterAgentsByRange(livings_data->enemies, player_data, GW::Constants::Range::Earshot);
    const auto result_ids_Aggro = FilterAgentIDS(livings, filter_ids);

    if (player_data.pos.x == next_pos.x && player_data.pos.y == next_pos.y)
        return result_ids_Aggro.size() == 0;
    else if (result_ids_Aggro.size() == 1)
        return false;

    const auto rect = GameRectangle(player_data.pos, next_pos, GW::Constants::Range::Spellcast);
    const auto filtered_livings = GetEnemiesInGameRectangle(rect, livings_data->enemies);

    auto result_ids_rect = std::set<uint32_t>{};
    if (GW::GetDistance(GW::GamePos{-7887.61F, 4279.11F, 0}, player_data.pos) < 2500.0F) // At fuse pull 2
        result_ids_rect = FilterAgentIDS(filtered_livings, filter_ids_full);
    else
        result_ids_rect = FilterAgentIDS(filtered_livings, filter_ids);

    return result_ids_rect.size() == 0;
}

float GetProgressValue()
{
    const auto c = GW::CharContext::instance();

    if (!c || !c->progress_bar)
        return 0.0F;

    return c->progress_bar->progress;
}

bool DhuumFightDone(uint32_t dhuum_id)
{
    if (!dhuum_id)
        return false;

    const auto dhuum_agent = GW::Agents::GetAgentByID(dhuum_id);
    if (!dhuum_agent)
        return false;

    const auto dhuum_living = dhuum_agent->GetAsAgentLiving();
    if (!dhuum_living)
        return false;

    const auto progress = GetProgressValue();
    if (dhuum_living->allegiance != GW::Constants::Allegiance::Enemy && progress > 0.99F)
        return true;

    return false;
}

uint32_t GetUwTriggerRoleId(const TriggerRole role)
{
    uint32_t trigger_id = 0U;

    switch (role)
    {
    case TriggerRole::LT:
    {
        trigger_id = GetTankId();
        break;
    }
    case TriggerRole::EMO:
    {
        trigger_id = GetEmoId();
        break;
    }
    case TriggerRole::DB:
    {
        trigger_id = GetDhuumBitchId();
        break;
    }
    }

    return trigger_id;
}

bool LtIsBonded()
{
    const auto lt_id = GetTankId();

    const auto has_prot = AgentHasBuff(GW::Constants::SkillID::Protective_Bond, lt_id);
    const auto has_life = AgentHasBuff(GW::Constants::SkillID::Life_Bond, lt_id);
    const auto has_ballth = AgentHasBuff(GW::Constants::SkillID::Balthazars_Spirit, lt_id);

    return (has_prot && has_life && has_ballth);
}
