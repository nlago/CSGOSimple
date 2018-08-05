#pragma once

#include <string>
#include <map>
#include "valve_sdk/Misc/Color.hpp"
#include "valve_sdk/kit_parser.hpp"
#include "valve_sdk/item_definitions.hpp"
#include <limits>

#define OPTION(type, var, val) type var = val

struct aimbot_settings {
	bool enabled = false;
	bool deathmatch = false;
	bool autopistol = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool autowall = false;
	bool silent = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool humanize = false;
	struct {
		bool enabled = false;
		int ticks = 6;
	} backtrack;
	bool only_in_zoom = true;
	int aim_type = 1;
	int priority = 0;
	int fov_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};

struct item_setting
{
	char name[32] = "Default";
	bool enabled = false;
	int definition_vector_index = 0;
	int definition_index = 1;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	int stat_trak = 0;
	float wear = 0;
	char custom_name[32] = "";
};

class Config
{
public:
	OPTION(int, selected_config_index, 0);
    // 
    // ESP
    // 
    OPTION(bool, esp_enabled, false);
    OPTION(bool, esp_enemies_only, false);
    OPTION(bool, esp_player_boxes, false);
    OPTION(bool, esp_player_names, false);
    OPTION(bool, esp_player_health, false);
    OPTION(bool, esp_player_armour, false);
    OPTION(bool, esp_player_weapons, false);
    OPTION(bool, esp_player_snaplines, false);
    OPTION(bool, esp_crosshair, false);
    OPTION(bool, esp_dropped_weapons, false);
    OPTION(bool, esp_defuse_kit, false);
    OPTION(bool, esp_planted_c4, false);

    // 
    // GLOW
    // 
    OPTION(bool, glow_enabled, false);
    OPTION(bool, glow_enemies_only, false);
    OPTION(bool, glow_players, false);
    OPTION(bool, glow_chickens, false);
    OPTION(bool, glow_c4_carrier, false);
    OPTION(bool, glow_planted_c4, false);
    OPTION(bool, glow_defuse_kits, false);
    OPTION(bool, glow_weapons, false);

    //
    // CHAMS
    //
    OPTION(bool, chams_player_enabled, false);
    OPTION(bool, chams_player_enemies_only, false);
    OPTION(bool, chams_player_wireframe, false);
    OPTION(bool, chams_player_flat, false);
    OPTION(bool, chams_player_ignorez, false);
    OPTION(bool, chams_player_glass, false);
    OPTION(bool, chams_arms_enabled, false);
    OPTION(bool, chams_arms_wireframe, false);
    OPTION(bool, chams_arms_flat, false);
    OPTION(bool, chams_arms_ignorez, false);
    OPTION(bool, chams_arms_glass, false);

    //
    // MISC
    //
    OPTION(bool, misc_bhop, false);
    OPTION(bool, misc_no_hands, false);
	OPTION(bool, misc_thirdperson, false);
	OPTION(float, misc_thirdperson_dist, 50.f);
    OPTION(int, viewmodel_fov, 68);
    OPTION(float, mat_ambient_light_r, 0.0f);
    OPTION(float, mat_ambient_light_g, 0.0f);
    OPTION(float, mat_ambient_light_b, 0.0f);

    // 
    // COLORS
    // 
    OPTION(Color, color_esp_ally_visible, Color(0, 128, 255));
    OPTION(Color, color_esp_enemy_visible, Color(255, 0, 0));
    OPTION(Color, color_esp_ally_occluded, Color(0, 128, 255));
    OPTION(Color, color_esp_enemy_occluded, Color(255, 0, 0));
    OPTION(Color, color_esp_crosshair, Color(255, 255, 255));
    OPTION(Color, color_esp_weapons, Color(128, 0, 128));
    OPTION(Color, color_esp_defuse, Color(0, 128, 255));
    OPTION(Color, color_esp_c4, Color(255, 255, 0));

    OPTION(Color, color_glow_ally, Color(0, 128, 255));
    OPTION(Color, color_glow_enemy, Color(255, 0, 0));
    OPTION(Color, color_glow_chickens, Color(0, 128, 0));
    OPTION(Color, color_glow_c4_carrier, Color(255, 255, 0));
    OPTION(Color, color_glow_planted_c4, Color(128, 0, 128));
    OPTION(Color, color_glow_defuse, Color(255, 255, 255));
    OPTION(Color, color_glow_weapons, Color(255, 128, 0));

    OPTION(Color, color_chams_player_ally_visible, Color(0, 128, 255));
    OPTION(Color, color_chams_player_ally_occluded, Color(0, 255, 128));
    OPTION(Color, color_chams_player_enemy_visible, Color(255, 0, 0));
    OPTION(Color, color_chams_player_enemy_occluded, Color(255, 128, 0));
    OPTION(Color, color_chams_arms_visible, Color(0, 128, 255));
    OPTION(Color, color_chams_arms_occluded, Color(0, 128, 255));
	std::map<int, aimbot_settings> aimbot = {};

	struct {
		std::map<int, item_setting> m_items;
		std::unordered_map<std::string, std::string> m_icon_overrides;
		auto get_icon_override(const std::string original) const -> const char*
		{
			return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
		}
	} skins;
};

extern Config g_Options;
extern bool   g_Unload;
extern bool   g_Save;
extern bool   g_Load;
extern bool   g_Clear;
extern bool   g_View;