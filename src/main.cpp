#include <d3d11.h>
#include "FTSData.h"
#include "RE/Havok/hkVector4.h"
#include <hooking.h>
#include <DirectXMath.h>

using namespace RE;
using namespace BSScript;
using namespace std;


bool bNeedToUpdateFTSData = true;
bool bChangeAnimFlag = false;
bool nvgFlag = false;
bool hasCombo = false;
bool hasNvgCommit = false;
bool InGameFlag = false;
bool IsHooked = false;
bool bHasStartedScope = false;
bool testingFlag = false;
bool hasUpdateSighted = false;
bool hasEjectShellCasing = false;

bool bEnableScope = false;
float gameDeltaZoom = 1;
float scopeTimer = 0;

//ReshadeImpl::Impl* reshadeImpl;
ScopeData::ScopeDataHandler* sdh;
PlayerCharacter* player;
PlayerCamera* pcam;

RE::BGSKeyword* an_45;
RE::BGSKeyword* AnimsXM2010_scopeKH45;
RE::BGSKeyword* AnimsXM2010_scopeKM;
RE::BGSKeyword* AX50_toounScope_K;
RE::BGSKeyword* AnimsAX50_scopeKH45;
RE::BGSKeyword* QMW_AnimsQBZ191M_on;
RE::BGSKeyword* QMW_AnimsQBZ191M_off;
RE::BGSKeyword* QMW_AnimsRU556M_on;
RE::BGSKeyword* QMW_AnimsRU556M_off;
RE::BGSKeyword* Tull_SideAimKeyword;
RE::BGSKeyword* Tull_SupportKeyword;
RE::BGSKeyword* AX50_toounScope_L;
RE::BGSKeyword* AnimsAX50_scopeK;

RE::NiAVObject* scopeNode;
RE::NiAVObject* camNode;
RE::NiAVObject* weaponNode;
RE::NiAVObject* rootNode;

RE::NiNode* scopeNormalNode3rd_i;
RE::NiNode* scopeAimingNode3rd_i;

RE::NiNode* scopeNormalNode_i;
RE::NiNode* scopeAimingNode_i;

PlayerControls* pc;
HMODULE Upscaler;

NiPoint4 lastPosition;
NiPoint4 currPosition;
REL::Relocation<float*> ptr_deltaTime{ REL::ID(1013228) };


REL::Relocation<uintptr_t> ptr_PCUpdateMainThread{ REL::ID(633524), 0x22D };
uintptr_t PCUpdateMainThreadOrig;


BGSKeyword* ChangeAnimFlavorKeyword = nullptr;
ScopeData::FTSData* currentData;
const char* customPath = "Data\\F4SE\\Plugins\\FTS";

using namespace Hook;

D3D* hookIns;

char* _MESSAGE(const char* fmt, ...);

template <class Ty>
Ty SafeWrite64Function(uintptr_t addr, Ty data)
{
	DWORD oldProtect;
	void* _d[2];
	memcpy(_d, &data, sizeof(data));
	size_t len = sizeof(_d[0]);

	VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	Ty olddata;
	memset(&olddata, 0, sizeof(Ty));
	memcpy(&olddata, (void*)addr, len);
	memcpy((void*)addr, &_d[0], len);
	VirtualProtect((void*)addr, len, oldProtect, &oldProtect);
	return olddata;
}


TESForm* GetFormFromMod(std::string modname, uint32_t formid)
{
	if (!modname.length() || !formid)
		return nullptr;
	TESDataHandler* dh = TESDataHandler::GetSingleton();
	TESFile* modFile = nullptr;
	for (auto it = dh->files.begin(); it != dh->files.end(); ++it) {
		TESFile* f = *it;
		if (strcmp(f->filename, modname.c_str()) == 0) {
			modFile = f;
			break;
		}
	}
	if (!modFile)
		return nullptr;
	uint8_t modIndex = modFile->compileIndex;
	uint32_t id = formid;
	if (modIndex < 0xFE) {
		id |= ((uint32_t)modIndex) << 24;
	} else {
		uint16_t lightModIndex = modFile->smallFileCompileIndex;
		if (lightModIndex != 0xFFFF) {
			id |= 0xFE000000 | (uint32_t(lightModIndex) << 12);
		}
	}
	return TESForm::GetFormByID(id);
}


void WorldToScreen_Internal(RE::NiPoint3* in, RE::NiPoint3* out)
{
	using func_t = decltype(&WorldToScreen_Internal);
	REL::Relocation<func_t> func{ REL::ID(1132313) };
	return func(in, out);
}

DWORD StartHooking(LPVOID)
{
	Hook::D3D::Register();
	return 0;
}


bool IssueChangeAnim(std::monostate, BGSKeyword* keyword)
{
	bChangeAnimFlag = true;
	ChangeAnimFlavorKeyword = keyword;

	return true;
}

bool TestButton(std::monostate)
{
	return true;
}

bool IsInADS(Actor* a)
{
	return (a->gunState == GUN_STATE::kSighted || a->gunState == GUN_STATE::kFireSighted);
}


bool IsSideAim()
{
	bool temp = false;
	if (player) 
	{
		if (an_45) {
			temp = player->HasKeyword(an_45, reinterpret_cast<RE::TBO_InstanceData*>(player));
		}
		else if (AnimsXM2010_scopeKH45) {
			temp = player->HasKeyword(AnimsXM2010_scopeKH45, reinterpret_cast<RE::TBO_InstanceData*>(player));
		}
		else if (AnimsXM2010_scopeKM) {
			temp = player->HasKeyword(AnimsXM2010_scopeKM, reinterpret_cast<RE::TBO_InstanceData*>(player));
		} 
		else if (AnimsAX50_scopeKH45) {
			temp = player->HasKeyword(AnimsAX50_scopeKH45, reinterpret_cast<RE::TBO_InstanceData*>(player));
		} 
		else if (Tull_SideAimKeyword) {
			temp = player->HasKeyword(Tull_SideAimKeyword, reinterpret_cast<RE::TBO_InstanceData*>(player));
		} 
		else if (AX50_toounScope_K) {
			temp = player->HasKeyword(AX50_toounScope_K, reinterpret_cast<RE::TBO_InstanceData*>(player));
		}
		else if (AX50_toounScope_L) {
			temp = player->HasKeyword(AX50_toounScope_L, reinterpret_cast<RE::TBO_InstanceData*>(player));
		}
		else if (AnimsAX50_scopeK) {
			temp = player->HasKeyword(AnimsAX50_scopeK, reinterpret_cast<RE::TBO_InstanceData*>(player));
		}
	}
	return temp;
}

BGSKeyword* IsMagnifier()
{

	if (player) {
		if (QMW_AnimsQBZ191M_on) {
			if (player->HasKeyword(QMW_AnimsQBZ191M_on, reinterpret_cast<RE::TBO_InstanceData*>(player)))
				return QMW_AnimsQBZ191M_on;
		} else if (QMW_AnimsQBZ191M_off) {
			if (player->HasKeyword(QMW_AnimsQBZ191M_off, reinterpret_cast<RE::TBO_InstanceData*>(player)))
				return QMW_AnimsQBZ191M_off;
		} else if (QMW_AnimsRU556M_off) {
			if (player->HasKeyword(QMW_AnimsRU556M_off, reinterpret_cast<RE::TBO_InstanceData*>(player)))
				return QMW_AnimsRU556M_off;
		} else if (QMW_AnimsRU556M_on) {
			if (player->HasKeyword(QMW_AnimsRU556M_on, reinterpret_cast<RE::TBO_InstanceData*>(player)))
				return QMW_AnimsRU556M_on;
		}
	}
	return nullptr;
}

inline void InitCurrentScopeData(BGSKeyword* animFlavorKeyword)
{
	if (player->currentProcess) {
		//_MESSAGE("Equipping Process!!!");
		BSTArray<EquippedItem>& eventEquipped = player->currentProcess->middleHigh->equippedItems;

		if (eventEquipped.size() > 0 && eventEquipped[0].item.instanceData &&
			((TESObjectWEAP::InstanceData*)eventEquipped[0].item.instanceData.get())->type == WEAPON_TYPE::kGun) {
			//_MESSAGE("Equipping EveryThing Done!!!");
			TESObjectWEAP* eventWep = ((TESObjectWEAP*)eventEquipped[0].item.object);
			TESObjectWEAP::InstanceData* eventInstance = (TESObjectWEAP::InstanceData*)eventEquipped[0].item.instanceData.get();

			if (!eventInstance || !eventInstance->GetKeywordData())
				return;

			BSScrapArray<const BGSKeyword*> playerkeywords;
			player->CollectAllKeywords(playerkeywords, reinterpret_cast<TBO_InstanceData*>(player));

			for (uint32_t i = 0; i < eventInstance->GetKeywordData()->numKeywords; i++) {
				std::string_view tempKeywordEditorID = eventInstance->GetKeywordData()->keywords[i]->formEditorID.c_str();

				if (tempKeywordEditorID.starts_with("FTS_")) {
					if (playerkeywords.size() > 0) {

						size_t len = sdh->GetScopeDataMap()->count(tempKeywordEditorID.data());

						if (len == 0)
							return;

						std::map<std::string, ScopeData::FTSData*>::iterator it = sdh->GetScopeDataMap()->find(tempKeywordEditorID.data());

						if (len > 0) {

							if (animFlavorKeyword) {
								for (size_t kk = 0; kk < len; ++kk, ++it) {
									for (auto j : playerkeywords) {
										if (j == animFlavorKeyword) {
											sdh->SetCurrentFTSData(it->second);
											//reshadeImpl->UpdateTexture();
											return;
										}
									}
								}
							}

							it = sdh->GetScopeDataMap()->find(tempKeywordEditorID.data());

							for (size_t kk = 0; kk < len; ++kk, ++it) {
								string animKeyword = it->second->animFlavorEditorID;

								if (animKeyword.compare("FTS_NONE") == 0 || animKeyword.compare("")== 0) {
									sdh->SetCurrentFTSData(it->second);
									//reshadeImpl->UpdateTexture();
									return;
								}
							}
						}
					}
				}
			}

			sdh->SetCurrentFTSData(nullptr);
		}
	}
}

bool IsShowMenu = false;

/// <summary>
/// The hook method is from Bingle
/// </summary>
class InputEventReceiverOverride : public BSInputEventReceiver
{
public:
	typedef void (InputEventReceiverOverride::*FnPerformInputProcessing)(const InputEvent* a_queueHead);

	//using Virtual-Key Codes
	void ProcessButtonEvent(ButtonEvent* evn)
	{
		{
			if (evn->eventType != INPUT_EVENT_TYPE::kButton) {
				if (evn->next)
					ProcessButtonEvent((ButtonEvent*)evn->next);
				return;
			}

			uint32_t id = evn->idCode;
			if (evn->device == INPUT_DEVICE::kMouse)
				id += 0x100;
			if (evn->device == INPUT_DEVICE::kGamepad)
				id += 0x10000;
#ifdef Debug
			if (evn->device == INPUT_DEVICE::kKeyboard && id == VK_OEM_PERIOD && evn->JustPressed()) {
				std::monostate mono;
				TestButton(mono);
			}
#endif 
			
			if (evn->device == INPUT_DEVICE::kKeyboard) {


				if (currentData) {
					if (sdh->comboNVKey == -1) {
						if (id == (uint32_t)sdh->nvKey && evn->QJustPressed()) {
							nvgFlag = !nvgFlag;
							hookIns->SetNVG((int)nvgFlag);
						}
					} else {
						if (id == (uint32_t)(sdh->comboNVKey) && evn->heldDownSecs > 0 && evn->value == 1) {
							hasCombo = true;
						}

						if (id == (uint32_t)(sdh->comboNVKey) && evn->value == 0) {
							hasCombo = false;
						}

						if (hasCombo && id == (uint32_t)sdh->nvKey && evn->QJustPressed()) {
							nvgFlag = !nvgFlag;
							hookIns->SetNVG((int)nvgFlag);
						}
					}
				}
			}
		}
	
		
	
	}


	void HookedPerformInputProcessing(const InputEvent* a_queueHead)
	{
		if (!UI::GetSingleton()->menuMode && !UI::GetSingleton()->GetMenuOpen("LooksMenu") && !UI::GetSingleton()->GetMenuOpen("ScopeMenu") && a_queueHead) {
			ProcessButtonEvent((ButtonEvent*)a_queueHead);
			//ProcessButtonEventForMouse((MouseMoveEvent*)a_queueHead);
		}
		
		FnPerformInputProcessing fn = fnHash.at(*(uint64_t*)this);
		if (fn) {
			(this->*fn)(a_queueHead);
		}
	}

	void HookSink()
	{
		uint64_t vtable = *(uint64_t*)this;
		auto it = fnHash.find(vtable);
		if (it == fnHash.end()) {
			FnPerformInputProcessing fn = SafeWrite64Function(vtable, &InputEventReceiverOverride::HookedPerformInputProcessing);
			fnHash.insert(std::pair<uint64_t, FnPerformInputProcessing>(vtable, fn));
		}
	}

	void UnHookSink()
	{
		uint64_t vtable = *(uint64_t*)this;
		auto it = fnHash.find(vtable);
		if (it == fnHash.end())
			return;
		SafeWrite64Function(vtable, it->second);
		fnHash.erase(it);
	}

protected:
	static unordered_map<uint64_t, FnPerformInputProcessing> fnHash;
};
unordered_map<uint64_t, InputEventReceiverOverride::FnPerformInputProcessing> InputEventReceiverOverride::fnHash;


bool IsNeedToBeCull(int indexCount = 0, int StrideCount = 0)
{
	if (!ScopeData::ScopeDataHandler::GetSingleton())
		return false;
	if (!ScopeData::ScopeDataHandler::GetSingleton()->GetCurrentFTSData())
		return false;
	if (ScopeData::ScopeDataHandler::GetSingleton()->GetCurrentFTSData()->UsingSTS)
		return false;
	if (hasUpdateSighted)
		return false;

	return true;
}

void HandleScopeNode()
{
	if (currentData && currentData->UsingSTS)
		return;

	if (player) {
		if (player->Get3D(false)) {
			scopeNormalNode3rd_i = (RE::NiNode*)RE::PlayerCharacter::GetSingleton()->Get3D(false)->GetObjectByName("ScopeNormal");
			scopeAimingNode3rd_i = (RE::NiNode*)RE::PlayerCharacter::GetSingleton()->Get3D(false)->GetObjectByName("ScopeAiming");

			if (scopeNormalNode3rd_i && scopeAimingNode3rd_i) {
				if (RE::PlayerCharacter::GetSingleton()->IsInThirdPerson()) {
					scopeNormalNode3rd_i->SetAppCulled(false);
					scopeAimingNode3rd_i->SetAppCulled(true);
					return;
				}
			}
		}

		if (player->firstPerson3D) {
			scopeNormalNode_i = (RE::NiNode*)RE::PlayerCharacter::GetSingleton()->firstPerson3D->GetObjectByName("ScopeNormal");
			scopeAimingNode_i = (RE::NiNode*)RE::PlayerCharacter::GetSingleton()->firstPerson3D->GetObjectByName("ScopeAiming");

			if (scopeNormalNode_i && scopeAimingNode_i) {
				if (bEnableScope) {
					if (IsNeedToBeCull()) {
						scopeNormalNode_i->SetAppCulled(true);
						scopeAimingNode_i->SetAppCulled(false);

					} else {
						scopeNormalNode_i->SetAppCulled(false);
						//setbit(scopeAimingNode_i->flags.flags, 1);
						scopeAimingNode_i->SetAppCulled(true);
					}
				} else {
					scopeNormalNode_i->SetAppCulled(false);
					scopeAimingNode_i->SetAppCulled(true);
				}
			}
		}
	}
}

void HookedUpdate()
{
	if (InGameFlag&& player && player->Get3D(true)) {
		scopeNode = player->Get3D(true)->GetObjectByName("FTS:CenterPoint");
		camNode = player->Get3D(true)->GetObjectByName("Camera");
		rootNode = player->Get3D(true)->GetObjectByName("RArm_UpperArm");
		NiAVObject* RealRootNode = player->Get3D(true);
		pc = PlayerControls::GetSingleton();
		
		//MenuCursor::GetSingleton()->CenterCursor();
		
		NiPointer<bhkCharacterController> con = player->currentProcess->middleHigh->charController;
		uintptr_t charProxy = *(uintptr_t*)((uintptr_t)con.get() + 0x470);
		hkTransform* charProxyTransform = (hkTransform*)(charProxy + 0x40);


		if (scopeNode && camNode && rootNode) 
		{
			currPosition = charProxyTransform->m_translation;
			NiPoint4 VirTransLerp = { currPosition.pt[0] - lastPosition.pt[0], 
				currPosition.pt[1] - lastPosition.pt[1],
				currPosition.pt[2] - lastPosition.pt[2],
				currPosition.pt[3] - lastPosition.pt[3] };
			lastPosition = currPosition;

			NiPoint3 virDir = scopeNode->world.translate - camNode->world.translate;
			NiPoint3 lastVirDir = scopeNode->previousWorld.translate - camNode->previousWorld.translate;
			NiPoint3 VirDirLerp = NiPoint3( virDir - lastVirDir);
			
			NiPoint3 weaponPos = scopeNode->world.translate;
			NiPoint3 rootPos = camNode->world.translate;

			D3D::GameConstBuffer gcb;
	
			gcb.virDir = virDir;
			
			gcb.lastVirDir = lastVirDir;
			gcb.VirDirLerp = VirDirLerp;
			gcb.VirTransLerp = { VirTransLerp.pt[0], VirTransLerp.pt[1], VirTransLerp.pt[2] };
			gcb.weaponPos = weaponPos;
			gcb.rootPos = rootPos;

			gcb.camMat = camNode->world.rotate;

			if (bHasStartedScope) {
				scopeTimer += *ptr_deltaTime *1000;
				if (!bHasStartedScope) {
					scopeTimer = 0;
					return;
				}
				if (scopeTimer >= sdh->GetCurrentFTSData()->scopeFrame) {
					bEnableScope = true;
					hookIns->SetScopeEffect(true);
					scopeTimer = 0;
					bHasStartedScope = false;
				}
			} else {
				scopeTimer = 0;
			}

			hookIns->SetGameConstData(gcb);
			HandleScopeNode();
			hookIns->QueryRender(true);

			if(IsSideAim())
				hookIns->QueryRender(false);



		}
	}
	

	typedef void (*FnUpdate)();
	FnUpdate fn = (FnUpdate)PCUpdateMainThreadOrig;
	if (fn)
		(*fn)();
}



/// <summary>
/// The hook method is from Bingle
/// </summary>
class EquipWatcher : public BSTEventSink<TESEquipEvent>
{
public:
	virtual BSEventNotifyControl ProcessEvent(const TESEquipEvent& evn, BSTEventSource<TESEquipEvent>* a_source)
	{
		Actor* a = evn.a;

		if (a == player) {

			TESForm* item = TESForm::GetFormByID(evn.formId);
			/*reshade::log_message(4, "" + evn.formId);
			reshade::log_message(4, "Player!");*/
			
			if (evn.isEquip) {
				
			}

			if (evn.isEquip && item && item->formType == ENUM_FORM_ID::kWEAP) {
				hookIns->QueryRender(false);
				sdh->SetCurrentFTSData(nullptr);
				InitCurrentScopeData(nullptr);
				hookIns->SetInterfaceTextRefresh(true);
				bChangeAnimFlag = false;
				hookIns->QueryChangeReticleTexture();
				hookIns->ResetZoomDelta();
			}
		}
		
		return BSEventNotifyControl::kContinue;
	}
	F4_HEAP_REDEFINE_NEW(EquipWatcher);
};

/// <summary>
/// The hook method is from Bingle
/// </summary>
using std::unordered_map;
class AnimationGraphEventWatcher
{
public:
	typedef BSEventNotifyControl (AnimationGraphEventWatcher::*FnProcessEvent)(BSAnimationGraphEvent& evn, BSTEventSource<BSAnimationGraphEvent>* dispatcher);

	BSEventNotifyControl HookedProcessEvent(BSAnimationGraphEvent& evn, BSTEventSource<BSAnimationGraphEvent>* src)
	{
		FnProcessEvent fn = fnHash.at(*(uint64_t*)this);
		string prefix = "";

		_MESSAGE("evn.animEvent: %s; evn.argument: %s", evn.animEvent.c_str(), evn.argument.c_str());

		if (IsInADS(player)) {

			BGSKeyword* magKeyword = IsMagnifier();
			if (magKeyword != nullptr) {
				bChangeAnimFlag = true;
				ChangeAnimFlavorKeyword = magKeyword;
			}

			if (!IsSideAim() && !player->IsInThirdPerson() && bNeedToUpdateFTSData) {

				if (bChangeAnimFlag) {
					InitCurrentScopeData(ChangeAnimFlavorKeyword);
					bChangeAnimFlag = false;
				} else {
					InitCurrentScopeData(nullptr);
				}

				currentData = sdh->GetCurrentFTSData();

				if (currentData && !bHasStartedScope) {

					hookIns->StartScope(true);
					hookIns->SetFinishAimAnim(true);
					bHasStartedScope = true;
				}

				bNeedToUpdateFTSData = false;
			}

			if (sdh->GetCurrentFTSData() && sdh->GetCurrentFTSData()->shaderData.bBoltDisable) 
			{
				if (hasEjectShellCasing) {
					hookIns->SetScopeEffect(true);
					hasEjectShellCasing = false;
					hasUpdateSighted = false;
				}

				if (strcmp(evn.animEvent.c_str(), "UpdateSighted") == 0) {
					hookIns->SetScopeEffect(false);
					hasUpdateSighted = true;
				}

				if (strcmp(evn.animEvent.c_str(), "initiateBoltStart") == 0) {
					hookIns->SetScopeEffect(false);
					hasUpdateSighted = true;
				}

				/*if (hasUpdateSighted && strcmp(evn.animEvent.c_str(), "ReloadEnd") == 0) {
					hasUpdateSighted = false;
					hasEjectShellCasing = true;
				}*/
				if (hasUpdateSighted && strcmp(evn.animEvent.c_str(), "initiateStart") == 0) {
					hasUpdateSighted = false;
					hasEjectShellCasing = true;
				}

				
			}

			
		} 
		else{
			hasUpdateSighted = false;
			hasEjectShellCasing = false;
			hookIns->StartScope(false);
			bNeedToUpdateFTSData = true;
			hookIns->SetScopeEffect(false);
			bHasStartedScope = false;	
			bEnableScope = false;
		}


		return fn ? (this->*fn)(evn, src) : BSEventNotifyControl::kContinue;
	}

	void HookSink()
	{
		uint64_t vtable = *(uint64_t*)this;
		auto it = fnHash.find(vtable);
		if (it == fnHash.end()) {
			FnProcessEvent fn = SafeWrite64Function(vtable + 0x8, &AnimationGraphEventWatcher::HookedProcessEvent);
			fnHash.insert(std::pair<uint64_t, FnProcessEvent>(vtable, fn));
		}
	}

	void UnHookSink()
	{
		uint64_t vtable = *(uint64_t*)this;
		auto it = fnHash.find(vtable);
		if (it == fnHash.end())
			return;
		SafeWrite64Function(vtable + 0x8, it->second);
		fnHash.erase(it);
	}

protected:
	static unordered_map<uint64_t, FnProcessEvent> fnHash;
};
unordered_map<uint64_t, AnimationGraphEventWatcher::FnProcessEvent> AnimationGraphEventWatcher::fnHash;

bool RegisterFuncs(BSScript::IVirtualMachine* vm)
{
	stl::zstring fileName = "FakeThroughScope";
	//vm->BindNativeMethod(fileName, "TestButton", TestButton);

	vm->BindNativeMethod(fileName, "OnChangeAnimFlavor", IssueChangeAnim);


	return true;
}

void InitializePlugin()
{
	player = PlayerCharacter::GetSingleton();

	pcam = PlayerCamera::GetSingleton();
	((InputEventReceiverOverride*)((uint64_t)pcam + 0x38))->HookSink();
	
	an_45 = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("an_45d");
	AnimsXM2010_scopeKH45 = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AnimsXM2010_scopeKH45");
	AX50_toounScope_K = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AX50_toounScope_K");
	AnimsXM2010_scopeKM = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AnimsXM2010_scopeKM");
	AnimsAX50_scopeKH45 = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AnimsAX50_scopeKH45");
	AX50_toounScope_L = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AX50_toounScope_L");
	AnimsAX50_scopeK = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("AnimsAX50_scopeK");
	Tull_SideAimKeyword = (BGSKeyword*)GetFormFromMod("Tull_Framework.esp", 0x804);
	Tull_SupportKeyword = (BGSKeyword*)GetFormFromMod("Tull_Framework.esp", 0x80F);

	QMW_AnimsQBZ191M_on = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("QMW_AnimsQBZ191M_on");
	QMW_AnimsQBZ191M_off = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("QMW_AnimsQBZ191M_off");
	QMW_AnimsRU556M_on = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("QMW_AnimsRU556M_on");
	QMW_AnimsRU556M_off = (RE::BGSKeyword*)RE::TESForm::GetFormByEditorID("QMW_AnimsRU556M_off");

	sdh = ScopeData::ScopeDataHandler::GetSingleton();

	((AnimationGraphEventWatcher*)((uint64_t)PlayerCharacter::GetSingleton() + 0x38))->HookSink();
	

	EquipWatcher* ew = new EquipWatcher();
	EquipEventSource::GetSingleton()->RegisterSink(ew);

	Upscaler = GetModuleHandleA("Fallout4Upscaler.dll");

	hookIns->SetIsUpscaler(Upscaler);
	sdh->SetIsUpscaler(Upscaler);
	hookIns->QueryChangeReticleTexture();
	sdh->ReadCustomScopeDataFiles(customPath);
	sdh->ReadDefaultScopeDataFile();
	
}

void ResetScopeStatus()
{
	hookIns->InitPlayerData(player, pcam);
	InitCurrentScopeData(nullptr);
	hookIns->SetScopeEffect(false);
	hookIns->QueryChangeReticleTexture();
	if (sdh->GetCurrentFTSData()) {
		gameDeltaZoom = sdh->GetCurrentFTSData()->shaderData.minZoom;
	}

	InGameFlag = true;
}




extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v0.8.3"), Version::PROJECT);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor"sv);
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical(FMT_STRING("unsupported runtime v{}"), ver.string());
		return false;
	}

	HANDLE hThread = CreateThread(NULL, 0, StartHooking, (HMODULE)F4SE::WinAPI::GetCurrentModule(), 0, NULL);
	F4SE::AllocTrampoline(8 * 8);

	return true;
}


extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

	F4SE::Trampoline& trampoline = F4SE::GetTrampoline();
	PCUpdateMainThreadOrig = trampoline.write_call<5>(ptr_PCUpdateMainThread.address(), &HookedUpdate);

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	bool succ = papyrus->Register(RegisterFuncs);
	if (succ) {
		logger::warn("succ.");
	}

	const F4SE::MessagingInterface* message = F4SE::GetMessagingInterface();
	message->RegisterListener([](F4SE::MessagingInterface::Message* msg) -> void {

		if (msg->type == F4SE::MessagingInterface::kGameDataReady) {
			
			InitializePlugin();
			hookIns = &Hook::D3D::instance();
			hookIns->EnableRender(true);

		} else if (msg->type == F4SE::MessagingInterface::kPostLoadGame) {
			ResetScopeStatus();
		} else if (msg->type == F4SE::MessagingInterface::kNewGame) {
			ResetScopeStatus();
		}
		else if (msg->type == F4SE::MessagingInterface::kPostSaveGame) {
			//reshadeImpl->SetRenderEffect(true);
		} else if (msg->type == F4SE::MessagingInterface::kGameLoaded) {
			
			
			//reshadeImpl->SetRenderEffect(false);	
		}

	});

	return true;
}