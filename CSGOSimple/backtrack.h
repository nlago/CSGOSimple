#pragma once
#include "options.hpp"
#include "valve_sdk/csgostructs.hpp"
struct backtrack_data {
	int tick_count;
	int m_iTeamNum;
	Vector hitboxPos;
	int EntIndex;
};
class Backtrack {
public:
	void OnMove(CUserCmd *pCmd);
private:
	std::vector<backtrack_data> data = {};
};
extern Backtrack g_Backtrack;
