#include "hooks.hpp"
#include <intrin.h>  

#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "aimbot.h"
#include "backtrack.h"
#pragma intrinsic(_ReturnAddress)  

namespace Hooks
{
	vfunc_hook hlclient_hook;
	vfunc_hook direct3d_hook;
	vfunc_hook vguipanel_hook;
	vfunc_hook vguisurf_hook;
	vfunc_hook mdlrender_hook;
	vfunc_hook clientmode_hook;
	vfunc_hook sv_cheats;
	vfunc_hook gameevents_hook; recv_prop_hook* sequence_hook;

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient,"client.dll");
		direct3d_hook.setup(g_D3DDevice9,"shaderapidx9.dll");
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		mdlrender_hook.setup(g_MdlRender,"engine.dll");
		clientmode_hook.setup(g_ClientMode,"client.dll");
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);
		gameevents_hook.setup(g_GameEvents);
		gameevents_hook.hook_index(index::FireEvent, hkFireEvent);
		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), hkRecvProxy);
		
		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);

		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);

		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);

		vguisurf_hook.hook_index(index::PlaySound, hkPlaySound);

		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);

		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);

		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);

		Visuals::CreateFonts();
		std::cout << "Done!" << std::endl;
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		gameevents_hook.unhook_all();
		sequence_hook->~recv_prop_hook();

		Glow::Get().Shutdown();

		Visuals::DestroyFonts();
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* device)
	{
		auto oEndScene = direct3d_hook.get_original<EndScene>(index::EndScene);

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.viewmodel_fov);
		mat_ambient_light_r->SetValue(g_Options.mat_ambient_light_r);
		mat_ambient_light_g->SetValue(g_Options.mat_ambient_light_g);
		mat_ambient_light_b->SetValue(g_Options.mat_ambient_light_b);
		crosshair_cvar->SetValue(!g_Options.esp_crosshair);

		DWORD dwOld_D3DRS_COLORWRITEENABLE;

		device->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);

		Menu::Get().Render();

		device->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);

		if (InputSys::Get().IsKeyDown(VK_TAB))
			Utils::RankRevealAll();

		return oEndScene(device);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		auto oReset = direct3d_hook.get_original<Reset>(index::Reset);

		Visuals::DestroyFonts();
		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0) {
			Menu::Get().OnDeviceReset();
			Visuals::CreateFonts();
		}

		return hr;
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		auto oCreateMove = hlclient_hook.get_original<CreateMove>(index::CreateMove);

		oCreateMove(g_CHLClient, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;

		g_Aimbot.OnMove(cmd);
		g_Backtrack.OnMove(cmd);

		if (g_Options.misc_bhop) {
			BunnyHop::OnCreateMove(cmd);
		}

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __stdcall hkCreateMove_Proxy(int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx
			lea  ecx, [esp]
			push ecx
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<PaintTraverse>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) {
			if (g_EngineClient->IsInGame() && !g_EngineClient->IsTakingScreenshot()) {

				if (!g_LocalPlayer)
					return;

				if (g_Options.esp_enabled) {
					for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
						auto entity = C_BasePlayer::GetPlayerByIndex(i);

						if (!entity)
							continue;

						if (entity == g_LocalPlayer)
							continue;

						if (i < 65 && !entity->IsDormant() && entity->IsAlive()) {
							// Begin will calculate player screen coordinate, bounding box, etc
							// If it returns false it means the player is not inside the screen
							// or is an ally (and team check is enabled)
							if (Visuals::Player::Begin(entity)) {
								if (g_Options.esp_player_snaplines) Visuals::Player::RenderSnapline();
								if (g_Options.esp_player_boxes)     Visuals::Player::RenderBox();
								if (g_Options.esp_player_weapons)   Visuals::Player::RenderWeapon();
								if (g_Options.esp_player_names)     Visuals::Player::RenderName();
								if (g_Options.esp_player_health)    Visuals::Player::RenderHealth();
								if (g_Options.esp_player_armour)    Visuals::Player::RenderArmour();
							}
						}
						else if (g_Options.esp_dropped_weapons && entity->IsWeapon()) {
							Visuals::Misc::RenderWeapon((C_BaseCombatWeapon*)entity);
						}
						else if (g_Options.esp_defuse_kit && entity->IsDefuseKit()) {
							Visuals::Misc::RenderDefuseKit(entity);
						}
						else if (entity->IsPlantedC4()) {
							if (g_Options.esp_planted_c4)
								Visuals::Misc::RenderPlantedC4(entity);
						}
					}
				}

				if (g_Options.esp_crosshair)
					Visuals::Misc::RenderCrosshair();
			}
		}
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkPlaySound(const char* name)
	{
		static auto oPlaySound = vguisurf_hook.get_original<PlaySound>(index::PlaySound);

		oPlaySound(g_VGuiSurface, name);

		// Auto Accept
		if (strstr(name, "UI/competitive_accept_beep.wav")) {
			static auto fnAccept =
				(void(*)())Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 08 56 8B 35 ? ? ? ? 57 83 BE");

			fnAccept();

			//This will flash the CSGO window on the taskbar
			//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
			FLASHWINFO fi;
			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = InputSys::Get().GetMainWindow();
			fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.uCount = 0;
			fi.dwTimeout = 0;
			FlashWindowEx(&fi);
		}
	}
	//--------------------------------------------------------------------------------
	int __stdcall hkDoPostScreenEffects(int a1)
	{
		auto oDoPostScreenEffects = clientmode_hook.get_original<DoPostScreenEffects>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, a1);
	}

	//auto __stdcall vmt_hooks::hk_lockcursor() -> void
	//{
	//	static auto o_lockcursor = surface_vmt.original< void(__thiscall*)(void* pthis) >(67);

	//	if (gui->is_active())
	//	{
	//		sdk::interfaces::surface->UnlockCursor();
	//		return;
	//	}
	//	o_lockcursor(sdk::interfaces::surface);
	//}

	//surface_vmt.hook_table(sdk::interfaces::surface);
	//surface_vmt.hook(67, hk_lockcursor);

	//--------------------------------------------------------------------------------
	void __stdcall hkFrameStageNotify(ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<FrameStageNotify>(index::FrameStageNotify);
		ofunc(g_CHLClient, stage);
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkOverrideView(CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<OverrideView>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView) {
			Visuals::Misc::ThirdPerson();
		}

		ofunc(g_ClientMode, vsView);
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<DrawModelExecute>(index::DrawModelExecute);

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(g_MdlRender, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
	typedef bool(__thiscall *svc_get_bool_t)(PVOID);
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto ofunc = sv_cheats.get_original<svc_get_bool_t>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}

	bool __stdcall hkFireEvent(IGameEvent* pEvent)
	{
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);
		if (!strcmp(pEvent->GetName(), "player_death") && g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")) == g_EngineClient->GetLocalPlayer()) {
			auto& weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon && weapon->IsWeapon()) {
				auto& skin_data = g_Options.skins.m_items[weapon->m_Item().m_iItemDefinitionIndex()];
				if (skin_data.enabled && skin_data.stat_trak) {
					skin_data.stat_trak++;
					weapon->m_nFallbackStatTrak() = skin_data.stat_trak;
					weapon->GetClientNetworkable()->PostDataUpdate(0);
					weapon->GetClientNetworkable()->OnDataChanged(0);
				}
			}
			const auto icon_override = g_Options.skins.get_icon_override(pEvent->GetString("weapon"));
			if (icon_override) {
				pEvent->SetString("weapon", icon_override);
			}
		}
		return oFireEvent(g_GameEvents, pEvent);
	}
	//--------------------------------------------------------------------------------
	static auto random_sequence(const int low, const int high) -> int
	{
		return rand() % (high - low + 1) + low;
	}
	static auto fix_animation(const char* model, const int sequence) -> int
	{
		enum ESequence
		{
			SEQUENCE_DEFAULT_DRAW = 0,
			SEQUENCE_DEFAULT_IDLE1 = 1,
			SEQUENCE_DEFAULT_IDLE2 = 2,
			SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
			SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
			SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
			SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
			SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
			SEQUENCE_DEFAULT_LOOKAT01 = 12,
			SEQUENCE_BUTTERFLY_DRAW = 0,
			SEQUENCE_BUTTERFLY_DRAW2 = 1,
			SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
			SEQUENCE_BUTTERFLY_LOOKAT03 = 15,
			SEQUENCE_FALCHION_IDLE1 = 1,
			SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
			SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
			SEQUENCE_FALCHION_LOOKAT01 = 12,
			SEQUENCE_FALCHION_LOOKAT02 = 13,
			SEQUENCE_DAGGERS_IDLE1 = 1,
			SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
			SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
			SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
			SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,
			SEQUENCE_BOWIE_IDLE1 = 1,
		};
		if (strstr(model, "models/weapons/v_knife_butterfly.mdl")) {
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
			default:
				return sequence + 1;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_falchion_advanced.mdl")) {
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence - 1;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_push.mdl")) {
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
				return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
				return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence + 2;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_survival_bowie.mdl")) {
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_BOWIE_IDLE1;
			default:
				return sequence - 1;
			}
		}
		else {
			return sequence;
		}
	}
	void hkRecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto original_fn = sequence_hook->get_original_function();
		const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
		if (local && local->IsAlive())
		{
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);
			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid())
			{
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()))
				{
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid())
					{
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon)
						{
							if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex()))
							{
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = fix_animation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}
		}
		original_fn(pData, entity, output);
	}
}