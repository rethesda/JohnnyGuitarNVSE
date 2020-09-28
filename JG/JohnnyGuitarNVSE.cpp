#include <memory>
#include "nvse/PluginAPI.h"
#include "nvse/GameAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameForms.h"
#include "nvse/GameObjects.h"
#include "nvse/GameEffects.h"
#include "nvse/GameData.h"
#include "nvse/GameExtraData.h"
#include "nvse/GameTasks.h"
#include "nvse/GameProcess.h"
#include "nvse/GameRTTI.h"
#include "nvse/GameOSDepend.h"
#include "nvse/GameUI.h"
#include "nvse/GameUI.cpp"
#include "nvse/GameScript.h"
#include "nvse/SafeWrite.h"
#include "nvse/ScriptUtils.h"
#include "internal/decoding.h"
#include "JohnnyGuitar/JohnnyEventPredefinitions.h"
#include "JohnnyGuitar/misc.h"
#include "JohnnyGuitar/WorldToScreen.h"
#include "JohnnyGuitar/JohnnyGuitarNVSE.h"
#include "JohnnyGuitar/JohnnyParams.h"
#include "JohnnyGuitar/EditorIDs.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_form.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_utility.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_math.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_file.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_gameplay.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_mediaset.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_region.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_terminal.h"
#include "JohnnyGuitar/JohnnyFunctions/fn_ui.h"
#include "JohnnyGuitar/BMPHandling.h"
#include "JohnnyGuitar/EventFilteringInterface.h"
#include "JohnnyGuitar/CustomEventFilters.h"
#include "JohnnyGuitar/JohnnyEvents.h"

HMODULE JohnnyHandle;
IDebugLog		gLog;

void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	
	switch (msg->type)
	{

	case NVSEMessagingInterface::kMessage_NewGame:
	case NVSEMessagingInterface::kMessage_PostLoadGame: {
		isShowLevelUp = true;
		PlayerCharacter* g_thePlayer = PlayerCharacter::GetSingleton();
		ThisStdCall(0x8C17C0, g_thePlayer); // reevaluate reload speed modifiers
		ThisStdCall(0x8C1940, g_thePlayer); // reevaluate equip speed modifiers
		DoSkipMuzzleLights = 0; //reset the muzzle hook every time
		OnDyingHandler->FlushEventCallbacks();
		OnLimbGoneHandler->FlushEventCallbacks();
		OnCrosshairHandler->FlushEventCallbacks();
		if (bArrowKeysDisabled) {
			bArrowKeysDisabled = false;
		}
		break;
	}
	}

}

extern "C"{

bool NVSEPlugin_Query(const NVSEInterface * nvse, PluginInfo * info)
{
	// fill out the info structure
	gLog.Open("JohnnyGuitarNVSE.log");
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "JohnnyGuitarNVSE";
	info->version = 325;

	if (nvse->isNogore) 
	{
		_ERROR("NV noGore is unsupported");
		return false;
	}

	if (nvse->nvseVersion < NVSE_VERSION_INTEGER)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
		return false;
	}

	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	};

	// version checks pass
	_MESSAGE("JohnnyGuitarNVSE Loaded succesfully.\nJohnnyGuitarNVSE plugin version: %u\n", info->version);
	char filename[MAX_PATH];
	GetModuleFileNameA(JohnnyHandle, filename, MAX_PATH);
	return true;
}


bool NVSEPlugin_Load(const NVSEInterface * nvse)
{
	((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);
	NiPointBuffer = (NiPoint3*) malloc(sizeof(NiPoint3));
	char filename[MAX_PATH];
	GetModuleFileNameA(NULL, filename, MAX_PATH);
	strcpy((char *)(strrchr(filename, '\\') + 1), "Data\\nvse\\plugins\\JohnnyGuitar.ini");
	loadEditorIDs = GetPrivateProfileInt("MAIN", "bLoadEditorIDs", 0, filename);
	//fixHighNoon = GetPrivateProfileInt("MAIN", "bFixHighNoon", 0, filename);
	fixHighNoon = 0;

	WorldMatrx = new JGWorldToScreenMatrix;


	nvse->SetOpcodeBase(0x3100);

	REG_CMD(WorldToScreen);
	REG_CMD(ToggleLevelUpMenu);
	REG_CMD(IsLevelUpMenuEnabled);
	REG_CMD(GetBaseEffectAV);
	REG_CMD(GetBaseEffectArchetype);
	REG_CMD(IsCellVisited);
	REG_CMD(IsCellExpired);
	REG_TYPED_CMD(MD5File, String);
	REG_TYPED_CMD(SHA1File, String);
	REG_CMD(TogglePipBoy);
	REG_CMD(GetCalculatedWeaponDPS);
	REG_CMD(GetInteriorLightingTraitNumeric);
	REG_CMD(SetInteriorLightingTraitNumeric);
	REG_CMD(GetPixelFromBMP);
	REG_TYPED_CMD(GetWorldSpaceMapTexture, String);
	REG_CMD(Jump);
	REG_CMD(SetCameraShake);
	REG_CMD(StopVATSCam);
	REG_CMD(GetIMODAnimatable);
	REG_CMD(SetIMODAnimatable);
	REG_TYPED_CMD(GetEditorID, String);
	REG_CMD(GetJohnnyPatch);
	REG_CMD(SetVelEx);
	REG_CMD(UwUDelete);
	REG_CMD(GetMediaSetTraitNumeric);
	REG_CMD(SetMediaSetTraitNumeric);
	REG_TYPED_CMD(GetMediaSetTraitString, String);
	REG_CMD(SetMediaSetTraitString);
	REG_CMD(GetMediaSetTraitSound);
	REG_CMD(SetMediaSetTraitSound);
	REG_CMD(GetWeapon1stPersonModel);
	REG_CMD(SetWeapon1stPersonModel);
	REG_TYPED_CMD(GetBufferedCellsAlt, Array);
	REG_CMD(GetTimePlayed);
	REG_CMD(GetActorValueModifierAlt);
	REG_CMD(AsmBreak);
	REG_CMD(RefAddr);
	REG_TYPED_CMD(GetMusicTypePath, String);
	REG_CMD(GetMusicTypeDB);
	REG_CMD(SetMusicTypeDB);
	REG_CMD(EditorIDToFormID);
	REG_CMD(GetRegionWeatherOverride);
	REG_CMD(SetRegionWeatherOverride);
	REG_CMD(GetRegionWeatherPriority);
	REG_CMD(SetRegionWeatherPriority);
	REG_CMD(IsWeatherInRegion);
	REG_CMD(RemoveRegionWeather);
	REG_CMD(AddRegionWeather);
	REG_TYPED_CMD(GetRegionWeathers, Array);
	REG_CMD(ClearRegionWeathers);
	REG_CMD(StopSoundAlt);
	REG_CMD(RemovePrimitive);
	REG_CMD(GetPrimitiveType);
	REG_CMD(GetBaseScale);
	REG_CMD(GetCustomMapMarker);
	REG_CMD(UnsetAV);
	REG_CMD(UnforceAV);
	REG_CMD(ToggleNthPipboyLight);
	REG_CMD(SetBipedIconPathAlt);
	REG_CMD(GetFacegenModelFlag);
	REG_CMD(SetFacegenModelFlag);
	REG_TYPED_CMD(GetRaceBodyModelPath, String);
	REG_CMD(SetEquipType);
	REG_TYPED_CMD(GetFactionMembers, Array);
	REG_TYPED_CMD(GetRaceHeadModelPath, String);
	REG_CMD(GetDefaultHeapSize);
	REG_CMD(Get3DDistanceBetweenNiNodes);
	REG_CMD(Get3DDistanceToNiNode);
	REG_CMD(Get3DDistanceFromHitToNiNode);
	REG_CMD(GetVector3DDistance);
	REG_CMD(GetLinearVelocity);
	REG_CMD(GetLifeState);
	REG_CMD(GetRaceFlag);
	REG_CMD(SetRaceFlag);
	REG_CMD(GetContainerSound);
	REG_CMD(SetContainerSound);
	//Events
	REG_CMD(SetJohnnyOnDyingEventHandler);
	REG_CMD(SetJohnnyOnStartQuestEventHandler);
	REG_CMD(SetJohnnyOnStopQuestEventHandler);
	REG_CMD(DisableMuzzleFlashLights);
	REG_CMD(SetCustomMapMarkerIcon);
	REG_CMD(GetCreatureCombatSkill);
	REG_CMD(SetExplosionSound);
	REG_CMD(SetProjectileSound);
	REG_CMD(SetWeaponWorldModelPath);
	REG_CMD(Clamp);
	REG_CMD(Remap);
	REG_CMD(Lerp);
	REG_CMD(SetJohnnySeenDataEventHandler);
	REG_CMD(SetJohnnyOnLimbGoneEventHandler);
	REG_CMD(Sign);
	REG_CMD(AddTerminalMenuItem);
	REG_TYPED_CMD(GetTerminalMenuItemText, String);
	REG_CMD(SetTerminalMenuItemText);
	REG_CMD(GetTerminalMenuItemNote);
	REG_CMD(SetTerminalMenuItemNote);
	REG_CMD(GetTerminalMenuItemSubmenu);
	REG_CMD(SetTerminalMenuItemSubmenu);
	REG_CMD(GetRunSpeed);
	REG_CMD(DisableMenuArrowKeys);
	REG_CMD(EnableMenuArrowKeys);
	REG_CMD(GetQuestFailed);
	REG_CMD(SetJohnnyOnChallengeCompleteEventHandler);
	REG_CMD(GetTerminalMenuItemCount);
	REG_CMD(GetPipBoyMode);
	REG_CMD(GetWeaponVATSTraitNumeric);
	REG_CMD(SetWeaponVATSTraitNumeric);
	REG_CMD(RemoveTerminalMenuItem);
	REG_CMD(SetWorldSpaceMapTexture);
	REG_CMD(GetFormOverrideIndex);
	REG_CMD(SetJohnnyOnCrosshairEventHandler);
	REG_CMD(GetSequenceAnimGroup);
	REG_CMD(QueueObjectiveText);
	REG_CMD(QueueCinematicText);
	REG_TYPED_CMD(ar_SortEditor, Array);
	REG_CMD(SetUIUpdateSound);
	g_script = (NVSEScriptInterface*)nvse->QueryInterface(kInterface_Script);
	CmdIfc = (NVSECommandTableInterface*)nvse->QueryInterface(kInterface_CommandTable);
	initEventHooks(nvse);
	StrArgBuf = (char*) malloc((sizeof(char))*1024);
	ArrIfc = (NVSEArrayVarInterface*)nvse->QueryInterface(kInterface_ArrayVar);
	StrIfc = (NVSEStringVarInterface*)nvse->QueryInterface(kInterface_StringVar);

	if (!nvse->isEditor) {
		NVSEDataInterface *nvseData = (NVSEDataInterface*)nvse->QueryInterface(kInterface_Data);
		InventoryRefGetForID = (InventoryRef* (*)(UInt32))nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceGetForRefID);
		HandleGameHooks();
	}
	return true;
}
	BOOL WINAPI DllMain(
		HANDLE  hDllHandle,
		DWORD   dwReason,
		LPVOID  lpreserved
		)
	{
		switch (dwReason)
		{
		case (DLL_PROCESS_ATTACH) :
			JohnnyHandle = (HMODULE)hDllHandle;
			break;
		}
		return TRUE;
	}
};




