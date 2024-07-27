#include "hooks.h"

using namespace Gothic_II_Addon;

// Disable/Enable original AI_LookingCam()
bool toggleAI_LookingCam = true;

template <class T>
inline void zClamp(T& x, const T min, const T max) {
	if (x < min) x = min; else
	if (x > max) x = max;
}

// Need camera mode list
zSTRING CamModNormal("CAMMODNORMAL");
zSTRING CamModRun("CAMMODRUN");
zSTRING CamModDialog("CAMMODDIALOG");
zSTRING CamModInventory("CAMMODINVENTORY");
zSTRING CamModMelee("CAMMODMELEE");
zSTRING CamModMagic("CAMMODMAGIC");
zSTRING CamModMeleeMult("CAMMODMELEEMULT");
zSTRING CamModRanged("CAMMODRANGED");
zSTRING CamModSwim("CAMMODSWIM");
zSTRING CamModDive("CAMMODDIVE");
zSTRING CamModJump("CAMMODJUMP");
zSTRING CamModJumpUp("CAMMODJUMPUP");
zSTRING CamModClimb("CAMMODCLIMB");
zSTRING CamModDeath("CAMMODDEATH");
zSTRING CamModLook("CAMMODLOOK");
zSTRING CamModLookBack("CAMMODLOOKBACK");
zSTRING CamModFocus("CAMMODFOCUS");
zSTRING CamModRangedShrt("CAMMODRANGEDSHORT");
zSTRING CamModShoulder("CAMMODSHOULDER");
zSTRING CamModFirstPerson("CAMMODFIRSTPERSON");
zSTRING CamModThrow("CAMMODTHROW");
zSTRING CamModMobLadder("CAMMODMOBLADDER");
zSTRING CamModFall("CAMMODFALL");


// AHTUNG THERE IS SOMETHING WRONG!
void __fastcall oCAIHuman__ChangeCamModeBySituation(oCAIHuman* self, void* vtable);
auto Hook_oCAIHuman_ChangeCamModeBySituation = Union::CreateHook((void*)0x0069CD60, oCAIHuman__ChangeCamModeBySituation, Union::HookType::Hook_Detours);
void __fastcall oCAIHuman__ChangeCamModeBySituation(oCAIHuman* self, void* vtable)
{
	// Disable hook
	Hook_oCAIHuman_ChangeCamModeBySituation.Disable();
	
	// for 'cam->SetMode()'
	zCArray<zCVob*> targetList;

	zCAICamera* cam = zCAICamera::GetCurrent();

	if (!cam)
	{
		// Anyone is use zSpy in 2024 ???? :/
		//zERR_WARNING("C: no camera ai present");
		return;
	}

	if (ztimer->frameTimeFloat == 0) return;		// keine Wechsel wenn das game pausiert

	// **** erst mal die sonderfälle abarbeiten ****

	// keine camera wechsel während eines dialoges
	if (cam->GetMode() == CamModDialog)
	{
		if (cam->GetPreviousMode() == CamModFirstPerson) self->thirdPersonFallback = TRUE;

		return;

		// TODO: Dialog Cam bleibt stecken Bug hier umgehen.
		// funzt so noch nicht!

		/*if (cam->GetTargetList().GetNum()>0)
		{
			oCNpc *npc = zDYNAMIC_CAST<oCNpc>(cam->GetTargetList()[0]);

			if (npc->GetTalkingWith())
			{
				// Einer der beiden muss im ZS_Talk sein.
				int index = parser.GetIndex("ZS_TALK");
				if (npc->state.IsInState(index) || npc->GetTalkingWith()->state.IsInState(index))
				{
					// alles ok, camera darf nicht wechseln.
					return;
				}
			}

		};*/
	};

	// portal hint an die camera

	zCSkyControler_Outdoor* sky = zDYNAMIC_CAST<zCSkyControler_Outdoor>(zCSkyControler::s_activeSkyControler);
	zBOOL camIsInSector = sky && (sky->rainFX.camLocationHint != zCSkyControler::zTCamLocationHint::zCAM_OUTSIDE_SECTOR);
	oCPortalRoomManager* rooms = ogame->GetPortalRoomManager();
	cam->SetHintTargetInPortalRoom(rooms && (rooms->curPlayerRoom != null) || camIsInSector);

	// kein camera wechsel während waffe ziehen
	// kein camera wechsel während magiekranz rotation

	// ermitteln, ob firstperson erlaubt ist
	zBOOL firstPersonAllowed = TRUE;

	{
		// cache ini reading
		static zBOOL zDontSwitchToThirdPerson = zBOOL(-2);
		if (zDontSwitchToThirdPerson == zBOOL(-2))
			zDontSwitchToThirdPerson = zoptions->ReadBool("ENGINE", "zDontSwitchToThirdPerson", FALSE);

		if (!zDontSwitchToThirdPerson)
			if ((self->npc->GetWeaponMode() != NPC_WEAPON_NONE && !self->npc->IsMonster()) ||					// 1. kampf ? 
				self->npc->GetInteractMob() ||					// 2. mobinteraktion ?
				self->npc->HasTorch() ||					// 3. Fackel in Hand ?
				self->IsDead() ||					// 4. tot
				self->npc->IsUnconscious() ||					// 5. bewusstlos
				(self->Pressed(GAME_LOOK) && self->Pressed(GAME_UP)) ||					// 6. Rückspiegel   					
				self->npc->GetBodyState() == BS_SWIM || self->npc->GetBodyState() == BS_DIVE)						// 7. tauchen & schwimmen (probs wegen fft polys)
			{
				firstPersonAllowed = FALSE;
			}
	}

	// firstperson aktiv ?
	if (cam->GetMode() == CamModFirstPerson)
	{
		self->thirdPersonFallback = FALSE;
		// hier darf nur reagiert werden, wenn die 1st person camera nicht sowieso abgeschaltet werden soll
		if (!(self->Toggled(GAME_LOOK_FP) && self->Pressed(GAME_LOOK_FP)))
		{
			// illegaler pc state -> 3rd person erzwingen
			self->thirdPersonFallback = !firstPersonAllowed;

			if (!self->thirdPersonFallback) return;
		}
	}
	else if (self->thirdPersonFallback)
	{
		// nein, aber:
		// bisher war die thirdperson camera erzwungen
		// nun testen wir, ob wir wieder zur 1st person cam zurückschalten müssen
		if (firstPersonAllowed)
		{
			self->thirdPersonFallback = FALSE;
			self->lookedAround = FALSE;			// muss, sonst kann es vorkommen, das der sc noch weiterläuft
			cam->SetMode(CamModFirstPerson, targetList);
			return;
		}
	}

	// *** key driven modes ***
	// 1. umschau kamera / rückspiegel
	if (self->Pressed(GAME_LOOK))
	{
		if (self->IsStanding())
		{
			// umschaukamera im stehen
			self->StopTurnAnis();

			cam->SetMode(CamModLook, targetList);

			zREAL cx = 0.5f, cy = 0.5f;

			self->GetModel()->SetRandAnisEnabled(FALSE);

			// TODO: Rework key movement to camera focusing in 3D space
			//if (self->Pressed(GAME_LEFT))		cx = 0.0f;
			//if (self->Pressed(GAME_RIGHT))	cx = 1.0f;
			//if (self->Pressed(GAME_DOWN))		cy = 1.0f;
			//if (self->Pressed(GAME_UP))		cy = 0.0f;

			self->SetLookAtTarget(cx, cy);

			self->lookedAround = TRUE;
		}
		else if (!self->lookedAround)
		{
			// rückspiegel während des laufens
			cam->SetMode(CamModLookBack, targetList);
			self->lookedAround = TRUE;
		}
		return;
	}
	else if (self->lookedAround)
	{
		self->StopLookAtTarget();
		self->lookedAround = FALSE;
	}

	// 2. first person
	if (ogame && !ogame->singleStep && !ogame->inLoadSaveGame && !ogame->inScriptStartup) // game running?
		if (zinput->GetToggled(GAME_LOOK_FP) && firstPersonAllowed)
		{
			// firstperson einschalten, wenn noch nicht aktiv
			if (cam->GetMode() != CamModFirstPerson)
			{
				cam->SetMode(CamModFirstPerson, targetList);
				// muss nix mehr getan werden -> raus hier
				return;
			}
		}

	// *** state driven camera modes

	// 1. tot / bewusstlos
	if (self->IsDead() || self->npc->IsUnconscious())
	{
		cam->SetMode(CamModDeath, targetList);
		return;
	}


	// 2. mob interaction cam
	oCMobInter* mob_interact = self->npc->GetInteractMob();

	if (mob_interact && !self->Pressed(GAME_WEAPON))
	{
		zSTRING mode = zSTRING("CAMMODMOB") + mob_interact->GetScemeName();

		// gibt es einen mode mit einem passenden scheme namen, so wird dieser genommen, ansonsten der default mob mode
		if (cam->IsModeAvailable(mode)) cam->SetMode(mode, targetList);
		else							cam->SetMode(zSTRING("CAMMODMOBDEFAULT"), targetList);

		return;
	}

	// 3. am tauchen 
	if (self->npc->GetBodyState() == BS_DIVE)
	{
		cam->SetMode(CamModDive, targetList);
		return;
	};


	// 4. am schwimmen ?
	if (self->GetWaterLevel() > 1)
	{
		cam->SetMode(CamModSwim, targetList);
		return;
	}

	// 5. cammodjumpup noch drin ? welches interface ?
	/*
	*/

	// 6. CamModClimb (noch drin ?)
	zCModel* humModel = self->GetModel();
	if (humModel->IsAniActive(humModel->GetAniFromAniID(self->s_hang)))
	{
		cam->SetMode(CamModClimb, targetList);
		return;
	}

	// 7. CamModFall (noch drin ?)
	if (humModel->IsAniActive(humModel->GetAniFromAniID(self->s_fall)) || humModel->IsAniActive(humModel->GetAniFromAniID(self->s_fallb)))
	{
		cam->SetMode(CamModFall, targetList);
		return;
	}


	// 8. inventory/tradescreen offen ? dann mal die inventory kamera an
	if (self->npc->inventory2.IsOpen())
	{
		cam->SetMode(CamModInventory, targetList);
		return;
	}

	// 9. magie gezogen ?
	if (self->npc->GetWeaponMode() == NPC_WEAPON_MAG)
	{
		cam->SetMode(CamModMagic, targetList);
		return;
	}

	// 10. fernkampfwaffe gezogen ?
	if (self->npc->GetWeaponMode() == NPC_WEAPON_BOW || self->npc->GetWeaponMode() == NPC_WEAPON_CBOW)
	{
		cam->SetMode(CamModRanged, targetList);
		return;
	}

	// 11. nahkampfwaffe gezogen
	if (self->npc->GetWeaponMode() >= NPC_WEAPON_FIST && self->npc->GetWeaponMode() <= NPC_WEAPON_2HS)
	{
		cam->SetMode(CamModMelee, targetList);
		return;
	}

	// *** sonderfälle ***
	// CamModDialog
	// *** CamModThrow;

	// noch nix gefunden ? dann schmeiss die normale camera an!
	cam->SetMode(CamModNormal, targetList);


	// Enable hook
	Hook_oCAIHuman_ChangeCamModeBySituation.Enable();
}

void __fastcall zCAICamera__AI_LookingCam(zCAICamera* self, void* vtable);
auto Hook_zCAICamera_AI_LookingCam = Union::CreateHook((void*)0x004A3690, zCAICamera__AI_LookingCam, Union::HookType::Hook_Detours);
void __fastcall zCAICamera__AI_LookingCam(zCAICamera* self, void* vtable)
{
	// Disable hook
	Hook_zCAICamera_AI_LookingCam.Disable();

	// Original engine code
	if (toggleAI_LookingCam) 
	{
		self->AI_LookingCam();
		Hook_zCAICamera_AI_LookingCam.Enable(); 
		return;
	}

	// Do custom code if you want...
	if (true) {Hook_zCAICamera_AI_LookingCam.Enable(); return;}

	const float LOOKINGCAM_MIN_ELEV = -80.0F;
	const float LOOKINGCAM_MAX_ELEV = +89.0F;
	const float LOOKINGCAM_MIN_AZI = -90.0F;
	const float LOOKINGCAM_MAX_AZI = +90.0F;

	const float ELEV_VELO = 200;
	const float RANGE_VELO = 900;
	const float AZI_VELO = 200;

	float frameTime = self->moveTracker->frameTime;

	if		(zinput->GetState(GAME_DOWN)) 											self->bestRotX = self->bestRotX + self->s_iLookAroundSgn * (frameTime * ELEV_VELO);
	else if (zinput->GetState(GAME_UP)) 											self->bestRotX = self->bestRotX - self->s_iLookAroundSgn * (frameTime * ELEV_VELO);
	if		(zinput->GetState(GAME_LEFT) || zinput->GetState(GAME_STRAFELEFT))		self->bestRotY = self->bestRotY + (frameTime * AZI_VELO);
	else if (zinput->GetState(GAME_RIGHT) || zinput->GetState(GAME_STRAFERIGHT))	self->bestRotY = self->bestRotY - (frameTime * AZI_VELO);

	zClamp(self->bestRotY, LOOKINGCAM_MIN_AZI, LOOKINGCAM_MAX_AZI);

	zMAT4	playerTrafo = self->moveTracker->trafoTStoWS;
	zVEC3 aiTry = self->moveTracker->playerHead;

	if (!self->firstPerson && !zinput->GetState(GAME_LOOK_FP))
	{
		float focusAtScale = 1;

		const float MAX_AT_OFFSET = 100;

		focusAtScale = (self->bestRotX / 90) * MAX_AT_OFFSET;
		if (focusAtScale >= 0)
		{
			if (self->focusOffsetZ < focusAtScale) self->focusOffsetZ = self->focusOffsetZ + (frameTime * RANGE_VELO);
			if (self->focusOffsetZ > focusAtScale) self->focusOffsetZ = focusAtScale;
		}
		else
		{
			if (self->focusOffsetZ > focusAtScale) self->focusOffsetZ = self->focusOffsetZ - (frameTime * RANGE_VELO);
			if (self->focusOffsetZ < focusAtScale) self->focusOffsetZ = focusAtScale;
		}

		// langsam zur idealen range zurückfallen
		float lookRange = (self->camVob->GetPositionWorld(), self->moveTracker->playerPos).Length();

		if (lookRange < self->GetBestRange() * 100)
		{
			float rng = lookRange + (frameTime * RANGE_VELO * 10);

			int	  searchFlags = zPATHSEARCH_INCLUDE_PLAYER_CHECK |
				zPATHSEARCH_INCLUDE_NEARWALL_CHECK |
				zPATHSEARCH_INCLUDE_CAM_CHECK;

			if (rng > self->GetBestRange() * 100) rng = self->GetBestRange() * 100;

			zVEC3 test = self->CalcAziElevRange(self->bestRotY, self->bestRotX, rng, self->moveTracker->trafoTStoWS);
			zTPoseFailReason* failReason = nullptr;
			if (self->pathSearch->IsPointValid(test, searchFlags, *failReason))  lookRange = rng;

		}

		aiTry = self->CalcAziElevRange(self->bestRotY, self->bestRotX, lookRange, self->moveTracker->trafoTStoWS);

		self->pathSearch->AdjustCenterSphere(aiTry, self->moveTracker->playerPos, 30.0f);

		if (((aiTry - self->moveTracker->playerHead).Length() < 40) || ((aiTry - self->moveTracker->playerPos).Length() < 65))
		{
			aiTry = self->moveTracker->playerHead;
			self->firstPerson = TRUE;
		}
	}
	else
	{
		aiTry = self->moveTracker->playerHead;
		self->firstPerson = TRUE;
	}

	zSPathSearchResult evasion;

	evasion.bestLerpMode = zPATH_LERP_LINE;
	evasion.p1 = self->camVob->GetPositionWorld();
	evasion.p2 = aiTry;
	evasion.veloTrans = self->veloTrans;
	evasion.veloRot = self->veloRot;
	evasion.r1.Matrix4ToQuat(self->camVob->trafoObjToWorld);

	if (self->firstPerson)
	{
		if (zinput->GetState(GAME_DOWN)) 											self->bestRotX = self->bestRotX + self->s_iLookAroundSgn * (0.3F * (frameTime * ELEV_VELO));
		else if (zinput->GetState(GAME_UP)) 											self->bestRotX = self->bestRotX - self->s_iLookAroundSgn * (0.3F * (frameTime * ELEV_VELO));
		if (zinput->GetState(GAME_LEFT) || zinput->GetState(GAME_STRAFELEFT))		self->bestRotY = self->bestRotY - (4.5F * (frameTime * AZI_VELO));
		else if (zinput->GetState(GAME_RIGHT) || zinput->GetState(GAME_STRAFERIGHT))		self->bestRotY = self->bestRotY + (4.5F * (frameTime * AZI_VELO));

		zClamp(self->bestRotX, -89.999f, 89.999f);
		zClamp(self->bestRotY, -95.0f, 95.0f);

		evasion.bestLerpMode = zPATH_LERP_CUSTOM;
		playerTrafo.SetTranslation(aiTry);
		zMAT4 firstPersonTrafo = self->moveTracker->trafoTStoWS;

		firstPersonTrafo.PostRotateY(self->bestRotY);
		firstPersonTrafo.PostRotateX(self->bestRotX);
		evasion.r2.Matrix4ToQuat(firstPersonTrafo);
	}

	self->moveTracker->InterpolateTo(evasion);
	zMAT4 newTrafo = self->camVob->GetNewTrafoObjToWorld();
	zVEC3 newPos = newTrafo.GetTranslation();
	self->pathSearch->AdjustCenterSphere(newPos, self->moveTracker->playerPos, 1.0f);
	newTrafo.SetTranslation(newPos);
	self->camVob->SetTrafo(newTrafo);

	// Enable hook
	Hook_zCAICamera_AI_LookingCam.Enable();
}