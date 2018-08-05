#include "aimbot.h"
#include "autowall.h"
#include "helpers/math.hpp"
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsRcs()
{
	return g_LocalPlayer->m_iShotsFired() >= settings.rcs_start;
}
//--------------------------------------------------------------------------------
float GetRealDistanceFOV(float distance, QAngle angle, CUserCmd* cmd)
{
	Vector aimingAt;
	Math::AngleVectors(cmd->viewangles, aimingAt);
	aimingAt *= distance;
	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;
	return aimingAt.DistTo(aimAt) / 5;
}
//--------------------------------------------------------------------------------
float Aimbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}
//--------------------------------------------------------------------------------
bool Aimbot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos)
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}
//--------------------------------------------------------------------------------
bool Aimbot::IsEnabled(CUserCmd *pCmd)
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return false;
	}
	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon || !(pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsRifle())) {
		return false;
	}
	auto weaponData = pWeapon->GetCSWeaponData();
	auto weapontype = weaponData->WeaponType;
	settings = g_Options.aimbot[pWeapon->m_Item().m_iItemDefinitionIndex()];
	if (!settings.enabled || !(pCmd->buttons & IN_ATTACK)) {
		return false;
	}
	if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) && settings.only_in_zoom && !g_LocalPlayer->m_bIsScoped()) {
		return false;
	}
	if (settings.fov == 0 && settings.silent_fov == 0 && !settings.rcs) {
		return false;
	}
	if (!pWeapon->HasBullets() || pWeapon->IsReloading()) {
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------
float Aimbot::GetSmooth()
{
	float smooth = IsRcs() && settings.rcs_smooth_enabled ? settings.rcs_smooth : settings.smooth;
	if (settings.humanize) {
		smooth += RandomFloat(-1, 4);
	}
	return smooth;
}
//--------------------------------------------------------------------------------
void Aimbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	if (GetSmooth() <= 1) {
		return;
	}
	Vector vAimAngle;
	Math::AngleVectors(aimAngle, vAimAngle);
	Vector vCurrentAngle;
	Math::AngleVectors(currentAngle, vCurrentAngle);
	Vector delta = vAimAngle - vCurrentAngle;
	Vector smoothed = vCurrentAngle + delta / GetSmooth();
	Math::VectorAngles(smoothed, angle);
}
//--------------------------------------------------------------------------------
void Aimbot::RCS(QAngle &angle, C_BasePlayer* target)
{
	if (!settings.rcs || !IsRcs()) {
		return;
	}
	if (settings.rcs_x == 0 && settings.rcs_y == 0) {
		return;
	}
	if (target) {
		QAngle punch = g_LocalPlayer->m_aimPunchAngle();
		angle.pitch -= punch.pitch * (settings.rcs_x / 50.f);
		angle.yaw -= punch.yaw * (settings.rcs_y / 50.f);
	}
	else if (settings.rcs_type == 0) {
		QAngle NewPunch = { CurrentPunch.pitch - RCSLastPunch.pitch, CurrentPunch.yaw - RCSLastPunch.yaw, 0 };
		angle.pitch -= NewPunch.pitch * (settings.rcs_x / 50.f);
		angle.yaw -= NewPunch.yaw * (settings.rcs_y / 50.f);
	}
	Math::FixAngles(angle);
}
//--------------------------------------------------------------------------------
float Aimbot::GetFov()
{
	if (IsRcs() && settings.rcs && settings.rcs_fov_enabled) return settings.rcs_fov;
	if (!silent_enabled) return settings.fov;
	return settings.silent_fov > settings.fov ? settings.silent_fov : settings.fov;
}
//--------------------------------------------------------------------------------
C_BasePlayer* Aimbot::GetClosestPlayer(CUserCmd* cmd, int &bestBone)
{
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();
	if (target && !kill_delay && settings.kill_delay > 0 && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
		kill_delay = true;
		kill_delay_time = (int)GetTickCount() + settings.kill_delay;
	}
	if (kill_delay) {
		if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
		else return NULL;
	}
	C_BasePlayer* player;
	target = NULL;
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	int health;
	float fov;
	float damage;
	float distance;
	int fromBone = settings.aim_type == 1 ? 0 : settings.hitbox;
	int toBone = settings.aim_type == 1 ? 7 : settings.hitbox;
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
	{
		damage = 0.f;
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		if (!settings.deathmatch && player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) {
			continue;
		}
		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			Math::VectorAngles(eVecTarget - pVecTarget, ang);
			Math::FixAngles(ang);
			distance = pVecTarget.DistTo(eVecTarget);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(distance, ang, cmd);
			else
				fov = GetFovToPlayer(cmd->viewangles, ang);
			if (fov > GetFov()) {
				continue;
			}
			if (!g_LocalPlayer->CanSeePlayer(player, eVecTarget)) {
				if (!settings.autowall) {
					continue;
				}
				damage = Autowall::GetDamage(eVecTarget);
				if (damage < settings.min_damage) {
					continue;
				}
			}
			if ((settings.priority == 1 || settings.priority == 2) && damage == 0.f) {
				damage = Autowall::GetDamage(eVecTarget);
			}
			health = player->m_iHealth() - damage;
			if (settings.check_smoke && IsLineGoesThroughSmoke(pVecTarget, eVecTarget)) {
				continue;
			}
			if (settings.aim_type == 1 && bestBoneFov < fov) {
				continue;
			}
			bestBoneFov = fov;
			if (
				(settings.priority == 0 && bestFov > fov) ||
				(settings.priority == 1 && bestHealth > health) ||
				(settings.priority == 2 && bestDamage < damage) ||
				(settings.priority == 3 && distance < bestDistance)
				) {
				bestBone = bone;
				target = player;
				bestFov = fov;
				bestHealth = health;
				bestDamage = damage;
				bestDistance = distance;
			}
		}
	}
	return target;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsNotSilent(float fov)
{
	return IsRcs() || !silent_enabled || (silent_enabled && fov > settings.silent_fov);
}
//--------------------------------------------------------------------------------
void Aimbot::OnMove(CUserCmd *pCmd)
{
	if (!IsEnabled(pCmd)) {
		RCSLastPunch = { 0, 0, 0 };
		is_delayed = false;
		shot_delay = false;
		kill_delay = false;
		silent_enabled = settings.silent && settings.silent_fov > 0;
		target = NULL;
		return;
	}
	QAngle angles = pCmd->viewangles;
	QAngle current = angles;
	float fov = 180.f;
	if (!(settings.check_flash && g_LocalPlayer->IsFlashed())) {
		int bestBone = -1;
		if (GetClosestPlayer(pCmd, bestBone)) {
			Math::VectorAngles(target->GetHitboxPos(bestBone) - g_LocalPlayer->GetEyePos(), angles);
			Math::FixAngles(angles);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(g_LocalPlayer->GetEyePos().DistTo(target->GetHitboxPos(bestBone)), angles, pCmd);
			else
				fov = GetFovToPlayer(pCmd->viewangles, angles);
			if (!settings.silent && !is_delayed && !shot_delay && settings.shot_delay > 0) {
				is_delayed = true;
				shot_delay = true;
				shot_delay_time = GetTickCount() + settings.shot_delay;
			}
			if (shot_delay && shot_delay_time <= GetTickCount()) {
				shot_delay = false;
			}
			if (shot_delay) {
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
	}
	CurrentPunch = g_LocalPlayer->m_aimPunchAngle();
	if (IsNotSilent(fov)) {
		RCS(angles, target);
	}
	RCSLastPunch = CurrentPunch;
	if (target && IsNotSilent(fov)) {
		Smooth(current, angles, angles);
	}
	Math::FixAngles(angles);
	pCmd->viewangles = angles;
	if (IsNotSilent(fov)) {
		g_EngineClient->SetViewAngles(angles);
	}
	silent_enabled = false;
	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && settings.autopistol) {
		float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
		if (next_shot > 0) {
			pCmd->buttons &= ~IN_ATTACK;
		}
	}
}
Aimbot g_Aimbot;