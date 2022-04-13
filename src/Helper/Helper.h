#pragma once

#include <cstdint>
#include <string_view>

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>
#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Skill.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include <Logger.h>

#include <Skillbars.h>
#include <Types.h>
#include <Utils.h>

bool IsLoading();

bool IsExplorable();

bool IsOutpost();

bool IsMapReady();

bool TargetNearest(const TargetType type, const float max_distance = GW::Constants::SqrRange::Compass);

GW::EffectArray *GetEffects(const uint32_t agent_id);

bool DetectNotMoving(const uint32_t threshold = 100U);

bool DetectPlayerIsDead();

std::tuple<uint32_t, uint32_t, float> GetEnergy();

std::tuple<uint32_t, uint32_t, float> GetHp();
