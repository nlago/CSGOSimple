#include "backtrack.h"
#include "aimbot.h"
#include "helpers/math.hpp"
void Backtrack::OnMove(CUserCmd *pCmd)
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return;
	}
	C_BasePlayer* player;
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
	{
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		backtrack_data bd;
		bd.m_iTeamNum = player->m_iTeamNum();
		bd.tick_count = pCmd->tick_count;
		bd.hitboxPos = player->GetHitboxPos(0);
		bd.EntIndex = player->EntIndex();
		data.insert(data.begin(), bd);
	}
	auto it = data.begin();
	while (it != data.end()) {
		if (it->tick_count + 12 < pCmd->tick_count) it = data.erase(it);
		else it++;
	}
	if (!g_Aimbot.IsEnabled(pCmd)) {
		return;
	}
	if (!g_Aimbot.settings.backtrack.enabled) {
		return;
	}
	int tick_count = -1;
	QAngle angles;
	float bestFov = 180.f;
	for (int i = 0; i < data.size(); i++) {
		if (data[i].tick_count + g_Aimbot.settings.backtrack.ticks < pCmd->tick_count) {
			continue;
		}
		Math::VectorAngles(data[i].hitboxPos - g_LocalPlayer->GetEyePos(), angles);
		float fov = g_Aimbot.GetFovToPlayer(pCmd->viewangles, angles);
		if (fov < bestFov && fov < 5) {
			bestFov = fov;
			tick_count = data[i].tick_count;
		}
	}
	if (tick_count != -1) {
		pCmd->tick_count = tick_count;
	}
}
Backtrack g_Backtrack;