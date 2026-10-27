// Copyright (C) 2026 Maxmc. GPL-3.0-or-later. See LICENSE.txt.
#include "SessionPolicy.h"
#include "Profiles.h"
#include "HookMemory.h"
#include "StandingPickpocketAPI.h"
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <fstream>
#include <vector>
#include <span>
#include <spdlog/sinks/basic_file_sink.h>
namespace {
using namespace SPQP;
using namespace SPQP::Memory;
SessionPolicy session;
const StandingPickpocketAPI::V1* standing=nullptr;
bool installed=false;
// Diagnostic observation only: do not suppress detection, crimes or combat.
std::atomic<std::uint32_t> watchedActor=0,transferEvents=0;
std::atomic<std::uint64_t> watchedAt=0;
bool observing(){return watchedActor.load()!=0&&GetTickCount64()-watchedAt.load()<120000;}
void traceState(const char* reason,RE::TESObjectREFR* ref=nullptr){
 auto* actor=ref?ref->As<RE::Actor>():RE::TESForm::LookupByID<RE::Actor>(watchedActor.load());
 auto* player=RE::PlayerCharacter::GetSingleton();if(!actor||!player)return;
 SKSE::log::info("Session observation [{}]: actor={:08X} '{}' elapsed={}ms transfers={} targetCombat={} playerCombat={} hostileToPlayer={} detection={} playerSneaking={} distance={:.1f}",reason,actor->GetFormID(),actor->GetName(),GetTickCount64()-watchedAt.load(),transferEvents.load(),actor->IsInCombat(),player->IsInCombat(),actor->IsHostileToActor(player),actor->RequestDetectionLevel(player),player->IsSneaking(),player->GetDistance(actor));
}
struct ObservationEvents final:RE::BSTEventSink<RE::TESCombatEvent>,RE::BSTEventSink<RE::TESContainerChangedEvent>{
 RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* e,RE::BSTEventSource<RE::TESCombatEvent>*)override{
  if(e&&observing()){
   auto actor=e->actor?e->actor->GetFormID():0;auto target=e->targetActor?e->targetActor->GetFormID():0;
   if(actor==watchedActor.load()||target==watchedActor.load()||actor==0x14||target==0x14){
    SKSE::log::info("Combat transition: actor={:08X} target={:08X} state={} elapsed={}ms transfers={}",actor,target,e->newState.underlying(),GetTickCount64()-watchedAt.load(),transferEvents.load());traceState("combat event");
   }
  }return RE::BSEventNotifyControl::kContinue;
 }
 RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* e,RE::BSTEventSource<RE::TESContainerChangedEvent>*)override{
  if(e&&observing()&&((e->oldContainer==watchedActor.load()&&e->newContainer==0x14)||(e->newContainer==watchedActor.load()&&e->oldContainer==0x14))){
   ++transferEvents;SKSE::log::info("Inventory transfer during observation: from={:08X} to={:08X} item={:08X} count={}",e->oldContainer,e->newContainer,e->baseObj,e->itemCount);
  }return RE::BSEventNotifyControl::kContinue;
 }
}observationEvents;

using OpenFn=void(*)(RE::TESObjectREFR*,RE::ContainerMenu::ContainerMode);
using CanOpenFn=bool(*)(RE::TESObjectREFR*);
using EndFn=void(*)(bool);
using HandleFn=void(*)(RE::ObjectRefHandle);
OpenFn originalOpen=nullptr;CanOpenFn originalCanOpen=nullptr;EndFn originalEnd=nullptr;
HandleFn qpSetTarget=nullptr,qpBegin=nullptr;
bool (*qpReady)()=nullptr;
// ABI description only: no QuickPocket implementation is linked or bundled.
struct QPConfig {bool enabled;std::uint8_t pad[3];std::int32_t key;bool poisons,jewelry,other,equipped,closeOnDetection;std::uint8_t tail[3];};
static_assert(sizeof(QPConfig)==16&&offsetof(QPConfig,closeOnDetection)==12);
QPConfig (*qpConfig)()=nullptr;

std::string fileHash(HMODULE module){
 std::array<wchar_t,32768> path{};auto n=GetModuleFileNameW(module,path.data(),static_cast<DWORD>(path.size()));if(!n||n>=path.size())return {};
 std::ifstream file(std::filesystem::path(path.data()),std::ios::binary);if(!file)return {};
 BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;
 if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return {};
 DWORD size=0,used=0;BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),sizeof(size),&used,0);
 std::vector<unsigned char> object(size);std::array<unsigned char,32> result{};bool ok=BCryptCreateHash(alg,&hash,object.data(),size,nullptr,0,0)>=0;
 std::array<char,65536> block{};
 while(ok&&file){file.read(block.data(),block.size());auto count=file.gcount();if(count)ok=BCryptHashData(hash,reinterpret_cast<PUCHAR>(block.data()),static_cast<ULONG>(count),0)>=0;}
 ok=ok&&file.eof()&&BCryptFinishHash(hash,result.data(),static_cast<ULONG>(result.size()),0)>=0;
 if(hash)BCryptDestroyHash(hash);BCryptCloseAlgorithmProvider(alg,0);
 if(!ok)return {};std::string hex;for(auto byte:result){hex+="0123456789abcdef"[byte>>4];hex+="0123456789abcdef"[byte&15];}return hex;
}
bool moduleMatches(HMODULE module,const char* hash,std::uint32_t stamp,std::uint32_t size){
 if(!module||fileHash(module)!=hash)return false;
 const auto base=reinterpret_cast<std::uintptr_t>(module);auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return false;
 auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
 return nt->Signature==IMAGE_NT_SIGNATURE&&nt->FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64&&nt->FileHeader.TimeDateStamp==stamp&&nt->OptionalHeader.SizeOfImage==size;
}
TargetFacts facts(RE::TESObjectREFR* ref){
 TargetFacts f;auto* player=RE::PlayerCharacter::GetSingleton();auto* actor=ref?ref->As<RE::Actor>():nullptr;if(!player||!actor||actor==player)return f;
 f.actor=actor->GetFormID();f.alive=!actor->IsDead()&&!actor->IsDisabled();f.loaded=actor->Is3DLoaded();f.combat=actor->IsInCombat();f.companion=actor->IsPlayerTeammate();f.mount=actor->IsAMount();
 auto* cross=RE::CrosshairPickData::GetSingleton();f.underCrosshair=cross&&cross->GetActiveTarget().get().get()==ref;
 const auto multiplier=standing->GetEffectiveDistanceMultiplier();auto* ini=RE::INISettingCollection::GetSingleton();auto* setting=ini?ini->GetSetting("fActivatePickLength:Interface"):nullptr;
 const float reach=setting&&std::isfinite(setting->data.f)&&setting->data.f>0?setting->data.f:180.f;
 f.inRange=std::isfinite(multiplier)&&multiplier>0&&(multiplier>=1.f||player->GetDistance(ref)<=reach*multiplier);
 auto* ui=RE::UI::GetSingleton();f.menuBlocked=!ui||ui->GameIsPaused()||ui->IsMenuOpen("Dialogue Menu")||ui->IsMenuOpen("ContainerMenu")||ui->IsMenuOpen("BarterMenu")||ui->IsMenuOpen("Console");
 return f;
}
void endSession(bool close){
 if(session.permits(watchedActor.load()))traceState(close?"QuickPocket ending; close requested":"QuickPocket ending; target/config/menu change");
 session.end();originalEnd(close);
}
bool canOpen(RE::TESObjectREFR* target){
 const auto f=facts(target);
 const bool wasSelected=session.permits(watchedActor.load());
 if(!session.maintain(f)){
  if(wasSelected){SKSE::log::info("Session lost eligibility: checked={:08X} crosshair={} range={} combat={} blockedMenu={}",f.actor,f.underCrosshair,f.inRange,f.combat,f.menuBlocked);traceState("session eligibility lost");}
  if(observing()&&f.actor==watchedActor.load()){
   static std::uint64_t lastRejected=0;if(GetTickCount64()-lastRejected>1000){lastRejected=GetTickCount64();
    SKSE::log::info("Eligibility rejected: actor={:08X} alive={} loaded={} range={} crosshair={} combat={} blockedMenu={} companion={} mount={}",f.actor,f.alive,f.loaded,f.inRange,f.underCrosshair,f.combat,f.menuBlocked,f.companion,f.mount);traceState("adapter eligibility rejected",target);
   }
  }return false;
 }
 // Only the stance rejection was bypassed in the original routine. Its own
 // environment, detection setting, race, and NPC checks all still execute.
 if(!originalCanOpen(target)){traceState("QuickPocket rejected opening/maintenance",target);session.end();return false;}return true;
}
void open(RE::TESObjectREFR* target,RE::ContainerMenu::ContainerMode mode){
 if(mode!=RE::ContainerMenu::ContainerMode::kPickpocket||!qpReady()||!qpConfig().enabled){originalOpen(target,mode);return;}
 const auto f=facts(target);
 if(!f.eligible()){originalOpen(target,mode);return;}
 if(session.permits(f.actor)){endSession(true);return;}
 endSession(true);
 if(!session.begin(f)||!originalCanOpen(target)){session.end();originalOpen(target,mode);return;}
 watchedActor=f.actor;watchedAt=GetTickCount64();transferEvents=0;traceState("hotkey session begin",target);
 auto handle=target->GetHandle();qpSetTarget(handle);qpBegin(handle);
 SKSE::log::info("QuickPocket hotkey session requested for {:08X}",f.actor);
}
void install(){
 auto qp=GetModuleHandleW(L"QuickPocket.dll"),sp=GetModuleHandleW(L"StandingPickpocket.dll");
 if(!qp||!sp||!GetModuleHandleW(L"QuickLootIE.dll")){SKSE::log::warn("Adapter inactive: Standing Pickpocket, QuickPocket and QuickLoot IE are required");return;}
 if(!moduleMatches(qp,qpHash,qpStamp,qpSize)||!moduleMatches(sp,spHash,spStamp,spSize)){SKSE::log::warn("Adapter inactive: unsupported DLL fingerprint; companion code untouched");return;}
 auto request=reinterpret_cast<StandingPickpocketAPI::Request>(GetProcAddress(sp,"StandingPickpocket_RequestAPI"));standing=request?request(1):nullptr;if(!standing){SKSE::log::warn("Adapter inactive: Standing Pickpocket distance API unavailable");return;}
 const auto qb=reinterpret_cast<std::uintptr_t>(qp),sb=reinterpret_cast<std::uintptr_t>(sp);
 originalCanOpen=reinterpret_cast<CanOpenFn>(originalStub(qb+qpSite0));originalEnd=reinterpret_cast<EndFn>(originalStub(qb+qpSite2));originalOpen=reinterpret_cast<OpenFn>(originalStub(sb+spSite0));
 if(!originalCanOpen||!originalEnd||!originalOpen){SKSE::log::error("Adapter inactive: trampoline allocation failed; modules untouched");return;}
 auto make=[](std::uintptr_t address,auto bytes,std::uintptr_t hook){auto jump=jumpTo(hook);return Patch{address,{bytes.begin(),bytes.end()},{jump.begin(),jump.end()}};};
 std::vector<Patch> patches;
 patches.push_back(make(qb+qpSite0,qpBytes0,reinterpret_cast<std::uintptr_t>(canOpen)));
 patches.push_back({qb+qpSite1,{qpBytes1.begin(),qpBytes1.end()},std::vector<std::uint8_t>(6,0x90)});
 patches.push_back(make(qb+qpSite2,qpBytes2,reinterpret_cast<std::uintptr_t>(endSession)));
 patches.push_back(make(sb+spSite0,spBytes0,reinterpret_cast<std::uintptr_t>(open)));
 qpReady=reinterpret_cast<bool(*)()>(qb+0x20610);qpConfig=reinterpret_cast<QPConfig(*)()>(qb+0x1c1c0);qpSetTarget=reinterpret_cast<HandleFn>(qb+0x22f70);qpBegin=reinterpret_cast<HandleFn>(qb+0x22960);
 installed=apply(patches);SKSE::log::info("Adapter hook installation {}; SP 1.0.2 + QuickPocket 1.0.0; no PCR hooks",installed?"passed":"rejected");
}
void message(SKSE::MessagingInterface::Message* m){
 if(m->type==SKSE::MessagingInterface::kPostPostLoad)install();
 if(m->type==SKSE::MessagingInterface::kDataLoaded&&installed){auto* source=RE::ScriptEventSourceHolder::GetSingleton();source->AddEventSink<RE::TESCombatEvent>(&observationEvents);source->AddEventSink<RE::TESContainerChangedEvent>(&observationEvents);}

 if(m->type==SKSE::MessagingInterface::kPreLoadGame||m->type==SKSE::MessagingInterface::kNewGame||m->type==SKSE::MessagingInterface::kPostLoadGame){watchedActor=0;session.end();}
}
}
SKSEPluginLoad(const SKSE::LoadInterface* skse){
 SKSE::Init(skse);if(auto path=SKSE::log::log_directory()){*path/="StandingQuickPocket.log";auto sink=std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(),true);auto logger=std::make_shared<spdlog::logger>("StandingQuickPocket",sink);spdlog::set_default_logger(logger);spdlog::flush_on(spdlog::level::info);}
 SKSE::log::info("Standing QuickPocket compatibility 0.1.1 diagnostic; standalone optional adapter; PCR independent");
 return SKSE::GetMessagingInterface()->RegisterListener(message);
}
