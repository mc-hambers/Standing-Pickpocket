#include "../src/ScopedStanding.h"
#include "../src/ScopedRequest.h"
#include <stdexcept>
#include <iostream>
#include "../src/ModePolicy.h"
#include "../src/DistanceOverride.h"
struct State { struct { unsigned sneaking:1; unsigned other:31; } actorState1; };
int main() {
    int checks=0;
    auto check=[&](bool ok){++checks;if(!ok)throw std::runtime_error("Scoped stance regression");};
    State state{{1,42}};
    {ScopedStanding scope(&state,true);check(!state.actorState1.sneaking);check(state.actorState1.other==42);
     {ScopedStanding nested(&state,true);check(!state.actorState1.sneaking);}
     check(!state.actorState1.sneaking);state.actorState1.other=43;}
    check(state.actorState1.sneaking);check(state.actorState1.other==43);
    {ScopedStanding scope(&state,false);check(state.actorState1.sneaking);}
    check(state.actorState1.sneaking);
    try {ScopedStanding scope(&state,true);check(!state.actorState1.sneaking);throw 1;}catch(int){}
    check(state.actorState1.sneaking);
    state.actorState1.sneaking=0;
    {ScopedStanding scope(&state,true);check(!state.actorState1.sneaking);}
    check(!state.actorState1.sneaking);
    {ScopedStanding<State> scope(nullptr,true);check(true);}
    int targetA=1,targetB=2;int* request=nullptr;
    for(unsigned realSneak:{0u,1u}) {
        state.actorState1.sneaking=realSneak;
        {ScopedRequest guard(request,&targetA);
         check(request==&targetA);check(state.actorState1.sneaking==realSneak);
         {ScopedRequest nested(request,&targetB);check(request==&targetB);check(state.actorState1.sneaking==realSneak);}
         check(request==&targetA);check(state.actorState1.sneaking==realSneak);}
        check(request==nullptr);check(state.actorState1.sneaking==realSneak);
    }
    try {ScopedRequest guard(request,&targetA);throw 1;}catch(int){}
    check(request==nullptr);
    check(!BlockVanillaPickpocket(false,true));
    check(BlockVanillaPickpocket(true,true));
    check(BlockVanillaPickpocket(false,false));
    check(BlockVanillaPickpocket(true,false));
    DistanceOverride distance;int owner=0,other=0;
    check(!distance.Active());check(distance.Effective(0.8F)==0.8F);
    check(!distance.Set(nullptr,0.5F));check(!distance.Set(&owner,NAN));
    check(!distance.Set(&owner,0.09F));check(!distance.Set(&owner,1.01F));
    check(distance.Set(&owner,0.5F));check(distance.Active());check(distance.Effective(0.8F)==0.5F);
    check(!distance.Set(&other,0.7F));check(!distance.Release(&other));
    check(distance.Set(&owner,0.3F));check(distance.Effective(0.8F)==0.3F);
    check(distance.Release(&owner));check(distance.Effective(0.8F)==0.8F);
    check(distance.Set(&other,1.0F));check(distance.Release(&other));
    std::cout << "Passed " << checks << " stance restoration assertions\n";
}
