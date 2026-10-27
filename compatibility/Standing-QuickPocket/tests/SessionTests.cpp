#include "SessionPolicy.h"
#include <iostream>
#include <stdexcept>
int main(){
 using namespace SPQP;unsigned checks=0;auto check=[&](bool v){++checks;if(!v)throw std::runtime_error("Adapter session regression");};
 SessionPolicy p;TargetFacts good{123,true,true,true,true,false,false,false,false};
 check(!p.permits(123));check(p.begin(good));check(p.permits(123));check(!p.permits(124));check(p.maintain(good));
 for(int i=0;i<9;++i){auto bad=good;switch(i){case 0:bad.actor=0;break;case 1:bad.alive=false;break;case 2:bad.loaded=false;break;case 3:bad.inRange=false;break;case 4:bad.underCrosshair=false;break;case 5:bad.combat=true;break;case 6:bad.menuBlocked=true;break;case 7:bad.companion=true;break;case 8:bad.mount=true;break;}
 check(!p.begin(bad));check(p.begin(good));check(!p.maintain(bad));check(!p.permits(123));}
 check(p.begin(good));auto different=good;different.actor=456;check(!p.maintain(different));check(!p.permits(456));check(p.begin(good));p.end();check(!p.permits(123));check(!p.maintain(good));
 std::cout<<"Passed "<<checks<<" adapter policy checks; gameplay untested\n";
}
