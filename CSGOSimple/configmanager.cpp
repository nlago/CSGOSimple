#include "configmanager.h"
#include "options.hpp"
#include "valve_sdk\misc\Color.hpp"


void CConfig::SetupValue(int &value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue< int >(category, name, &value));
}

void CConfig::SetupValue(float& value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue< float >(category, name, &value));
}


void CConfig::SetupValue(bool& value, bool def, std::string category, std::string name)
{
	value = def;
	bools.push_back(new ConfigValue< bool >(category, name, &value));
}

void CConfig::SetupValueC(Color &value, Color def, std::string category, std::string name)
{

}
void CConfig::Setup()
{
	//ESP
	SetupValue(g_Options.esp_enabled, false, "ESP", "esp_enabled");
	SetupValue(g_Options.esp_enemies_only, false, "ESP", "esp_enemies_only");
	SetupValue(g_Options.esp_player_boxes, false, "ESP", "esp_player_boxes");
	SetupValue(g_Options.esp_player_names, false, "ESP", "esp_player_names");
	SetupValue(g_Options.esp_player_health, false, "ESP", "esp_player_health");
	SetupValue(g_Options.esp_player_armour, false, "ESP", "esp_player_armour");
	SetupValue(g_Options.esp_player_weapons, false, "ESP", "esp_player_weapons");
	SetupValue(g_Options.esp_player_snaplines, false, "ESP", "esp_player_snaplines");
	SetupValue(g_Options.esp_crosshair, false, "ESP", "esp_crosshair");
	SetupValue(g_Options.esp_dropped_weapons, false, "ESP", "esp_dropped_weapons");
	SetupValue(g_Options.esp_defuse_kit, false, "ESP", "esp_defuse_kit");
	SetupValue(g_Options.esp_planted_c4, false, "ESP", "esp_planted_c4");
	//GLOW
	SetupValue(g_Options.glow_enabled, false, "GLOW", "glow_enabled");
	SetupValue(g_Options.glow_enemies_only, false, "GLOW", "glow_enemies_only");
	SetupValue(g_Options.glow_players, false, "GLOW", "glow_players");
	SetupValue(g_Options.glow_chickens, false, "GLOW", "glow_chickens");
	SetupValue(g_Options.glow_c4_carrier, false, "GLOW", "glow_c4_carrier");
	SetupValue(g_Options.glow_planted_c4, false, "GLOW", "glow_planted_c4");
	SetupValue(g_Options.glow_defuse_kits, false, "GLOW", "glow_defuse_ktis");
	SetupValue(g_Options.glow_weapons, false, "GLOW", "glow_weapons");
	//CHAMS
	SetupValue(g_Options.chams_player_enabled, false, "CHAMS", "chams_player_enabled");
	SetupValue(g_Options.chams_player_enemies_only, false, "CHAMS", "chams_player_enemies_only");
	SetupValue(g_Options.chams_player_wireframe, false, "CHAMS", "chams_player_wireframe");
	SetupValue(g_Options.chams_player_flat, false, "CHAMS", "chams_player_flat");
	SetupValue(g_Options.chams_player_ignorez, false, "CHAMS", "chams_player_ignorez");
	SetupValue(g_Options.chams_player_glass, false, "CHAMS", "chams_player_glass");
	SetupValue(g_Options.chams_arms_enabled, false, "CHAMS", "chams_arms_enabled");
	SetupValue(g_Options.chams_arms_wireframe, false, "CHAMS", "chams_arms_wireframe");
	SetupValue(g_Options.chams_arms_flat, false, "CHAMS", "chams_arms_flat");
	SetupValue(g_Options.chams_arms_ignorez, false, "CHAMS", "chams_arms_ignorez");
	SetupValue(g_Options.chams_arms_glass, false, "CHAMS", "chams_arms_glass");
	//	MISC
	SetupValue(g_Options.misc_bhop, false, "MISC", "misc_bhop");
	SetupValue(g_Options.misc_no_hands, false, "MISC", "misc_no_hands");
	SetupValue(g_Options.viewmodel_fov, 68, "MISC", "viewmodel_fov");
	SetupValue(g_Options.mat_ambient_light_r, 0.0f, "MISC", "mat_ambinet_light_r");
	SetupValue(g_Options.mat_ambient_light_g, 0.0f, "MISC", "mat_ambinet_light_g");
	SetupValue(g_Options.mat_ambient_light_b, 0.0f, "MISC", "mat_ambinet_light_b");
	//Colors
	SetupValueC(g_Options.color_esp_ally_visible, Color(0, 128, 255), "COLOR", "color_esp_ally_visible");
	SetupValueC(g_Options.color_esp_enemy_visible, Color(255, 0, 0), "COLOR", "color_esp_enemy_visible");
	SetupValueC(g_Options.color_esp_ally_occluded, Color(0, 128, 255), "COLOR", "color_esp_ally_accluded");
	SetupValueC(g_Options.color_esp_enemy_occluded, Color(255, 0, 0), "COLOR", "color_esp_enemy_occluded");
	SetupValueC(g_Options.color_esp_crosshair, Color(255, 255, 255), "COLOR", "color_esp_crosshair");
	SetupValueC(g_Options.color_esp_weapons, Color(128, 0, 255), "COLOR", "color_esp_weapons");
	SetupValueC(g_Options.color_esp_defuse, Color(0, 128, 255), "COLOR", "color_esp_defuse");
	SetupValueC(g_Options.color_esp_c4, Color(255, 255, 0), "COLOR", "color_esp_c4");

	SetupValueC(g_Options.color_glow_ally, Color(0, 128, 255), "COLOR", "color_glow_ally");
	SetupValueC(g_Options.color_glow_enemy, Color(255, 0, 0), "COLOR", "color_glow_enemy");
	SetupValueC(g_Options.color_glow_chickens, Color(0, 128, 0), "COLOR", "color_glow_chickens");
	SetupValueC(g_Options.color_glow_c4_carrier, Color(255, 255, 0), "COLOR", "color_glow_c4_carrier");
	SetupValueC(g_Options.color_glow_planted_c4, Color(128, 0, 128), "COLOR", "color_glow_planted_c4");
	SetupValueC(g_Options.color_glow_defuse, Color(255, 255, 255), "COLOR", "color_glow_defuse");
	SetupValueC(g_Options.color_glow_weapons, Color(255, 128, 0), "COLOR", "color_glow_weapons");

	SetupValueC(g_Options.color_chams_player_ally_visible, Color(0, 128, 255), "COLOR", "color_chams_player_ally_visible");
	SetupValueC(g_Options.color_chams_player_ally_occluded, Color(0, 255, 128), "COLOR", "color_chams_player_ally_occluded");
	SetupValueC(g_Options.color_chams_player_enemy_visible, Color(255, 0, 0), "COLOR", "color_chams_player_enemy_visible");
	SetupValueC(g_Options.color_chams_player_enemy_occluded, Color(255, 128, 0), "COLOR", "color_chams_player_enemy_accluded");
	SetupValueC(g_Options.color_chams_arms_visible, Color(0, 128, 255), "COLOR", "color_chams_arms_visible");
	SetupValueC(g_Options.color_chams_arms_occluded, Color(0, 128, 255), "COLOR", "color_chams_arms_occluded");

}

/*
void CConfig::Save(std::string cfg)
{
static TCHAR path[MAX_PATH];
std::string folder, file;

if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
{
folder = std::string (path) + "\\" + cfg + "\\";
file = std::string(path) + "\\" + cfg + "\\config.ini";
}

CreateDirectory(folder.c_str(), NULL);

for (auto value : ints)
WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

for (auto value : floats)
WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

for (auto value : bools)
WritePrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
}

bool krass	(const TCHAR *kurwo)
{
return (GetFileAttributes(kurwo) != 0xFFFFFFFF);
}

void CConfig::Clean(std::string cfg)
{
static TCHAR path[MAX_PATH];
std::string folder, file;

if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
{
folder = std::string(path) + "\\" + cfg + "\\";
file = std::string(path) + "\\" + cfg + "\\config.ini";
}

if (krass(file.c_str)) {
remove(file.c_str());
}
}


void CConfig::Show(std::string cfg)
{
static TCHAR path[MAX_PATH];
std::string folder, file;

if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
{
folder = std::string(path) + "\\" + cfg + "\\";
file = std::string(path) + "\\" + cfg + "\\config.ini";
}

if (krass(file.c_str)) {
std::string gowno = "notepad.exe" + file;
system(gowno.c_str());
}
else
{
MessageBox(NULL, "1", "LOL", MB_OK);
}
}
void CConfig::Load(std::string cfg)
{
static TCHAR path[MAX_PATH];
std::string folder, file;

if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
{
folder = std::string(path) + "\\" + cfg + "\\";
file = std::string(path) + "\\" + cfg + "\\config.ini";
}

CreateDirectory(folder.c_str(), NULL);

char value_l[32] = { '\0' };

for (auto value : ints)
{
GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
*value->value = atoi(value_l);
}

for (auto value : floats)
{
GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
*value->value = atof(value_l);
}

for (auto value : bools)
{
GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
*value->value = !strcmp(value_l, "true");
}

}

CConfig* cConfigManager = new CConfig();

*/