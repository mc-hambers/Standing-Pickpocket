


#include <SKSE/Impl/PCH.h>
using namespace std::literals;
#include <RE/A/Actor.h>
#include <RE/T/TESNPC.h>
#include "ScopedStanding.h"
#include "ScopedRequest.h"
#include "ModePolicy.h"
#include "DistanceOverride.h"
#include "../Data/SKSE/Plugins/StandingPickpocketAPI.h"
#include <RE/I/INISettingCollection.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/B/ButtonEvent.h>
#include <RE/B/BSInputDeviceManager.h>
#include <RE/C/ControlMap.h>
#include <RE/C/CrosshairPickData.h>
#include <RE/C/ContainerMenu.h>
#include <RE/U/UI.h>
#include <RE/I/IVirtualMachine.h>
#include <RE/N/NativeFunction.h>
#include <RE/V/VirtualMachine.h>
#include <RE/I/IObjectHandlePolicy.h>
#include <RE/F/FunctionArguments.h>
#include <SKSE/SKSE.h>
#include <SKSE/InputMap.h>
#include <atomic>
#include <Windows.h>

namespace SP {
    constexpr int defaultKey = 34; // G, SKSE scan code
    std::atomic<int> key{defaultKey};
    std::atomic<bool> disableVanilla{false};
    std::atomic<bool> followerInventory{false};
    std::atomic<bool> horseInventory{false};
    thread_local RE::TESObjectREFR* hotkeyTarget=nullptr;
    std::atomic<float> distanceMultiplier{1.0F};
    DistanceOverride distanceOverride;
    std::atomic<bool> queued{false};
    constexpr const wchar_t* config = L"Data/SKSE/Plugins/StandingPickpocket.ini";

    int GetKey(RE::StaticFunctionTag*) { return key.load(); }
    void SetKey(RE::StaticFunctionTag*, int value) {
        if (value < -1 || value >= 282) return;
        key.store(value);
        auto text = std::to_wstring(value);
        WritePrivateProfileStringW(L"Controls", L"PickpocketKey", text.c_str(), config);
    }
    bool GetFollowerInventory(RE::StaticFunctionTag*) {return followerInventory.load();}
    void SetFollowerInventory(RE::StaticFunctionTag*,bool value) {
        followerInventory.store(value);
        WritePrivateProfileStringW(L"Controls",L"FollowerInventory",value ? L"1" : L"0",config);
    }
    bool GetHorseInventory(RE::StaticFunctionTag*) {return horseInventory.load();}
    void SetHorseInventory(RE::StaticFunctionTag*,bool value) {
        horseInventory.store(value);
        WritePrivateProfileStringW(L"Controls",L"HorseInventory",value ? L"1" : L"0",config);
    }
    bool GetDisableVanilla(RE::StaticFunctionTag*) { return disableVanilla.load(); }
    void SetDisableVanilla(RE::StaticFunctionTag*, bool value) {
        disableVanilla.store(value);
        WritePrivateProfileStringW(L"Controls", L"DisableVanillaPickpocket", value ? L"1" : L"0", config);
    }
    float GetDistanceMultiplier(RE::StaticFunctionTag*) { return distanceOverride.Effective(distanceMultiplier.load()); }
    bool IsDistanceOverridden(RE::StaticFunctionTag*) { return distanceOverride.Active(); }
    void RefreshDistanceHUD() {
        if(auto* tasks=SKSE::GetTaskInterface()) tasks->AddUITask([] { if(auto* p=RE::PlayerCharacter::GetSingleton()) p->UpdateCrosshairs(); });
    }
    bool SetExternalDistance(const void* owner,float value) {
        if(!distanceOverride.Set(owner,value)) return false;
        RefreshDistanceHUD();return true;
    }
    bool ReleaseExternalDistance(const void* owner) {
        if(!distanceOverride.Release(owner)) return false;
        RefreshDistanceHUD();return true;
    }
    float EffectiveDistance() {return GetDistanceMultiplier(nullptr);}
    bool ExternalDistanceActive() {return distanceOverride.Active();}
    void SetDistanceMultiplier(RE::StaticFunctionTag*, float value) {
        if(distanceOverride.Active()) return;
        value=std::isfinite(value) ? std::clamp(value,0.1F,1.0F) : 1.0F;
        distanceMultiplier.store(value);
        auto text=std::to_wstring(value);
        WritePrivateProfileStringW(L"Controls",L"DistanceMultiplier",text.c_str(),config);
        SKSE::GetTaskInterface()->AddTask([] { if(auto* p=RE::PlayerCharacter::GetSingleton()) p->UpdateCrosshairs(); });
    }
    bool WithinPickRange(RE::TESObjectREFR* ref) {
        auto* player=RE::PlayerCharacter::GetSingleton();
        // HUD text is built before the engine commits its new crosshair handle.
        // The engine-provided reference is authoritative here; don't require the old handle.
        if(!player || !ref) return false;
        const float multiplier=GetDistanceMultiplier(nullptr);
        // At 1.0 the engine's crosshair targeting is the authoritative vanilla reach.
        if(multiplier>=1.0F) return true;
        auto* ini=RE::INISettingCollection::GetSingleton();
        auto* setting=ini ? ini->GetSetting("fActivatePickLength:Interface") : nullptr;
        const float reach=setting && std::isfinite(setting->data.f) && setting->data.f>0 ? setting->data.f : 180.0F;
        return player->GetDistance(ref)<=reach*multiplier;
    }
    bool Register(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("IsDistanceOverridden","SP_Native",IsDistanceOverridden);
        vm->RegisterFunction("GetDistanceMultiplier","SP_Native",GetDistanceMultiplier);
        vm->RegisterFunction("SetDistanceMultiplier","SP_Native",SetDistanceMultiplier);
        vm->RegisterFunction("GetFollowerInventory","SP_Native",GetFollowerInventory);
        vm->RegisterFunction("SetFollowerInventory","SP_Native",SetFollowerInventory);
        vm->RegisterFunction("GetHorseInventory","SP_Native",GetHorseInventory);
        vm->RegisterFunction("SetHorseInventory","SP_Native",SetHorseInventory);
        vm->RegisterFunction("GetKey", "SP_Native", GetKey);
        vm->RegisterFunction("SetKey", "SP_Native", SetKey);
        vm->RegisterFunction("GetDisableVanilla", "SP_Native", GetDisableVanilla);
        vm->RegisterFunction("SetDisableVanilla", "SP_Native", SetDisableVanilla);
        return true;
    }
    using ActivateFn = bool (*)(RE::TESNPC*, RE::TESObjectREFR*, RE::TESObjectREFR*, std::uint8_t, RE::TESBoundObject*, std::int32_t);
    using TextFn = bool (*)(RE::TESNPC*, RE::TESObjectREFR*, RE::BSString&);
    REL::Relocation<ActivateFn> originalActivate;
    REL::Relocation<TextFn> originalText;

    bool UseStandingInteraction(RE::TESObjectREFR* ref) {
        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
        return hotkeyTarget!=ref && player && player->IsSneaking() && actor && actor != player && !actor->IsDead() && actor->CanPickpocket()
            && BlockVanillaPickpocket(disableVanilla.load(), WithinPickRange(ref));
    }
    bool Activate(RE::TESNPC* npc, RE::TESObjectREFR* target, RE::TESObjectREFR* activator,
                  std::uint8_t arg3, RE::TESBoundObject* object, std::int32_t count) {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if(player && activator==player && target && target==hotkeyTarget) {
            auto* actor=target->As<RE::Actor>();
            if(!actor || actor->IsDead() || actor->IsDisabled() || !actor->Is3DLoaded()
                || actor->IsPlayerTeammate() || actor->IsAMount() || !actor->CanPickpocket()
                || !WithinPickRange(actor)) return false;
            ScopedRequest consume(hotkeyTarget,static_cast<RE::TESObjectREFR*>(nullptr));
            // Menu/chance/crime hooks see the real stance. Never fake the player's sneak flag.
            RE::ContainerMenu::OpenMenu(target,RE::ContainerMenu::ContainerMode::kPickpocket);
            return true;
        }
        const bool override = player && activator == player && UseStandingInteraction(target);
        ScopedStanding scope(player ? player->AsActorState() : nullptr, override);
        return originalActivate(npc, target, activator, arg3, object, count);
    }
    bool GetText(RE::TESNPC* npc, RE::TESObjectREFR* ref, RE::BSString& text) {
        auto* player = RE::PlayerCharacter::GetSingleton();
        ScopedStanding scope(player ? player->AsActorState() : nullptr, UseStandingInteraction(ref));
        return originalText(npc, ref, text);
    }
    bool CanUse();
    using UpdateFn=void (*)(RE::PlayerCharacter*,float);
    REL::Relocation<UpdateFn> originalUpdate;
    void Update(RE::PlayerCharacter* player,float delta) {
        originalUpdate(player,delta);
        static bool previous=false;
        auto* crosshair=RE::CrosshairPickData::GetSingleton();
        auto handle=crosshair ? crosshair->GetActiveTarget() : RE::ObjectRefHandle{};
        auto ref=handle.get();
        const bool current=!disableVanilla.load() && player->IsSneaking() && WithinPickRange(ref.get());
        if(current!=previous) {
            previous=current;
            SKSE::GetTaskInterface()->AddUITask([] {
                if(CanUse()) RE::PlayerCharacter::GetSingleton()->UpdateCrosshairs();
            });
        }
    }
    void InstallHooks() {
        REL::Relocation<std::uintptr_t> table{RE::TESNPC::VTABLE[0]};
        originalActivate = table.write_vfunc(0x37, Activate);
        originalText = table.write_vfunc(0x4C, GetText);
        REL::Relocation<std::uintptr_t> playerTable{RE::PlayerCharacter::VTABLE[0]};
        originalUpdate=playerTable.write_vfunc(0xAD,Update);
    }
    bool CanUse() {
        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* ui = RE::UI::GetSingleton();
        auto* controls = RE::ControlMap::GetSingleton();
        return player && !player->IsDead() && ui && !ui->GameIsPaused()
            && !ui->IsMenuOpen("Dialogue Menu") && !ui->IsMenuOpen("ContainerMenu")
            && !ui->IsMenuOpen("BarterMenu") && !ui->IsMenuOpen("Console")
            && controls && controls->IsActivateControlsEnabled();
    }
    void InventoryTrace(const char* stage) {
        SKSE::log::info("Inventory hotkey: {}",stage);
        spdlog::default_logger()->flush();
    }
    void Open(RE::ObjectRefHandle handle) {
        InventoryTrace("task entered");
        queued.store(false);
        if (!CanUse()) return;
        auto ref = handle.get();
        auto* target = ref ? ref->As<RE::Actor>() : nullptr;
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!target || target == player || target->IsDead() || target->IsDisabled()
            || !target->Is3DLoaded()
            || !WithinPickRange(target)) return;
        auto* pick=RE::CrosshairPickData::GetSingleton();
        if(!pick || pick->GetActiveTarget().get().get()!=target) return;
        InventoryTrace("target validated");
        const bool teammate=target->IsPlayerTeammate();
        // Avoid the runtime-sensitive last-ridden-mount virtual call entirely.
        // NPC targets must never query player mount history.
        const bool mount=target->IsAMount();
        const bool ownHorse=mount && target->GetOwner()==player->GetActorBase();
        InventoryTrace("companion classification completed");
        if(teammate || ownHorse) {
            if(mount ? (ownHorse && horseInventory.load()) : (teammate && followerInventory.load())) {
                // Let the engine native prepare the actor and inventory menu together.
                // Directly opening kNPCMode omits the normal OpenInventory setup.
                InventoryTrace("dispatching actor inventory");
                auto* vm=RE::BSScript::Internal::VirtualMachine::GetSingleton();
                auto* policy=vm ? vm->GetObjectHandlePolicy() : nullptr;
                if(!policy) {SKSE::log::warn("Actor inventory unavailable: VM not ready");return;}
                const auto actorHandle=policy->GetHandleForObject(RE::FormType::ActorCharacter,target);
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                auto* args=RE::MakeFunctionArguments(false);
                if(!vm->DispatchMethodCall(actorHandle,"Actor","OpenInventory",args,callback))
                    SKSE::log::warn("Actor.OpenInventory request rejected for {:08X}",target->GetFormID());
            }
            return;
        }
        if(mount || !target->CanPickpocket()) return;
        InventoryTrace("pickpocket eligibility passed");
        // Keep upstream reference activation hooks, but identify the requested NPC
        // without exposing a fake player stance to menus, followers or other mods.
        ScopedRequest request(hotkeyTarget,static_cast<RE::TESObjectREFR*>(target));
        InventoryTrace("calling NPC activation");
        target->ActivateRef(player,0,nullptr,1,false);
        InventoryTrace("NPC activation returned");
    }
    class Input final : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* events,
            RE::BSTEventSource<RE::InputEvent*>*) override {
            if (!events || key.load() < 0) return RE::BSEventNotifyControl::kContinue;
            for (auto* event = *events; event; event = event->next) {
                auto* button = event->AsButtonEvent();
                if (!button || !button->IsDown()) continue;
                auto code = button->GetIDCode();
                switch (button->GetDevice()) {
                case RE::INPUT_DEVICE::kKeyboard: break;
                case RE::INPUT_DEVICE::kMouse: code += 256; break;
                case RE::INPUT_DEVICE::kGamepad: code = SKSE::InputMap::GamepadMaskToKeycode(code); break;
                default: continue;
                }
                if (static_cast<int>(code) != key.load() || !CanUse()) continue;
                auto* crosshair = RE::CrosshairPickData::GetSingleton();
                if (!crosshair) continue;
                auto handle = crosshair->GetActiveTarget();
                if (!handle || queued.exchange(true)) continue;
                SKSE::GetTaskInterface()->AddTask([handle] { Open(handle); });
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    } input;
}

extern "C" __declspec(dllexport) constinit SKSE::PluginVersionData SKSEPlugin_Version = [] {
    SKSE::PluginVersionData data;
    data.PluginVersion({1, 0, 2, 0});
    data.PluginName("StandingPickpocket");
    data.AuthorName("Max");
    data.UsesAddressLibrary();
    // Match CommonLibSSE-NG StructCompatibility::Independent for runtime accessors.
    data.UsesNoStructs();
    return data;
}();

extern "C" __declspec(dllexport) bool SKSEPlugin_Query(const SKSE::QueryInterface* skse,
    SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "StandingPickpocket";
    info->version = REL::Version(1, 0, 2, 0).pack();
    return !skse->IsEditor() && skse->RuntimeVersion() >= REL::Version(1, 5, 97, 0);
}
extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    static HANDLE instanceMutex = nullptr;
    const auto mutexName = L"Local\\StandingPickpocket." + std::to_wstring(GetCurrentProcessId());
    instanceMutex = CreateMutexW(nullptr, FALSE, mutexName.c_str());
    if (instanceMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        SKSE::stl::report_and_fail("Two Standing Pickpocket DLLs are loaded. Remove the older or renamed copy before starting Skyrim.");
    }
    wchar_t distanceText[64]{};
    GetPrivateProfileStringW(L"Controls",L"DistanceMultiplier",L"1.0",distanceText,64,SP::config);
    wchar_t* end=nullptr;
    float multiplier=std::wcstof(distanceText,&end);
    SP::distanceMultiplier.store(end!=distanceText && std::isfinite(multiplier) ? std::clamp(multiplier,0.1F,1.0F) : 1.0F);
    int loaded = GetPrivateProfileIntW(L"Controls", L"PickpocketKey", SP::defaultKey, SP::config);
    SP::key.store(loaded >= -1 && loaded < 282 ? loaded : SP::defaultKey);
    const auto legacyInventory=GetPrivateProfileIntW(L"Controls",L"CompanionInventory",0,SP::config);
    const bool follower=GetPrivateProfileIntW(L"Controls",L"FollowerInventory",legacyInventory,SP::config)!=0;
    const bool horse=GetPrivateProfileIntW(L"Controls",L"HorseInventory",legacyInventory,SP::config)!=0;
    SP::SetFollowerInventory(nullptr,follower);
    SP::SetHorseInventory(nullptr,horse);
    WritePrivateProfileStringW(L"Controls",L"CompanionInventory",nullptr,SP::config);
    SP::disableVanilla.store(GetPrivateProfileIntW(L"Controls", L"DisableVanillaPickpocket", 0, SP::config) != 0);
    // Retire the old mode without changing the saved key or vanilla preference.
    WritePrivateProfileStringW(L"Controls", L"DynamicActivationKeyMode", nullptr, SP::config);
    if (!SKSE::GetPapyrusInterface()->Register(SP::Register)) return false;
    return SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            SP::InstallHooks();
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(&SP::input);
        }
    });
}











extern "C" __declspec(dllexport) const StandingPickpocketAPI::V1* StandingPickpocket_RequestAPI(std::uint32_t version) {
    static const StandingPickpocketAPI::V1 api{1,SP::SetExternalDistance,SP::ReleaseExternalDistance,SP::EffectiveDistance,SP::ExternalDistanceActive};
    return version==1 ? &api : nullptr;
}
