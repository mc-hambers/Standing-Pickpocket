// Copyright (C) 2026 Maxmc. GPL-3.0-or-later.
#pragma once
#include <Windows.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
namespace SPQP::Memory {
std::array<std::uint8_t,15> jumpTo(std::uintptr_t target){std::array<std::uint8_t,15> b{};b[0]=0xff;b[1]=0x25;std::memcpy(b.data()+6,&target,8);b[14]=0x90;return b;}
void* originalStub(std::uintptr_t address){
 auto* code=static_cast<std::uint8_t*>(VirtualAlloc(nullptr,32,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));if(!code)return nullptr;
 std::memcpy(code,reinterpret_cast<void*>(address),15);auto jump=jumpTo(address+15);std::memcpy(code+15,jump.data(),14);DWORD old=0;
 if(!VirtualProtect(code,32,PAGE_EXECUTE_READ,&old)){VirtualFree(code,0,MEM_RELEASE);return nullptr;}
 FlushInstructionCache(GetCurrentProcess(),code,32);return code;
}
struct Patch {std::uintptr_t address;std::vector<std::uint8_t> expected,replacement;DWORD protection=0;};
bool apply(std::vector<Patch>& patches){
 for(auto& p:patches)if(std::memcmp(reinterpret_cast<void*>(p.address),p.expected.data(),p.expected.size()))return false;
 // Called before DataLoaded, before QuickPocket starts its targeting worker.
 std::size_t count=0;
 for(auto& p:patches){if(!VirtualProtect(reinterpret_cast<void*>(p.address),p.expected.size(),PAGE_EXECUTE_READWRITE,&p.protection))break;++count;}
 if(count!=patches.size()){for(std::size_t i=0;i<count;++i){DWORD ignored;VirtualProtect(reinterpret_cast<void*>(patches[i].address),patches[i].expected.size(),patches[i].protection,&ignored);}return false;}
 for(auto& p:patches){std::memcpy(reinterpret_cast<void*>(p.address),p.replacement.data(),p.replacement.size());FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(p.address),p.replacement.size());}
 // Sites on the same page inherit the first site's original protection.
 for(auto it=patches.rbegin();it!=patches.rend();++it){DWORD ignored;VirtualProtect(reinterpret_cast<void*>(it->address),it->expected.size(),it->protection,&ignored);}return true;
}
}
