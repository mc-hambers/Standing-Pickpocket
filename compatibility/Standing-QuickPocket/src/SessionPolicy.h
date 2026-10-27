// Copyright (C) 2026 Maxmc. GPL-3.0-or-later.
#pragma once
#include <cstdint>
namespace SPQP {
struct TargetFacts {
 std::uint32_t actor=0;
 bool alive=false,loaded=false,inRange=false,underCrosshair=false;
 bool combat=false,menuBlocked=false,companion=false,mount=false;
 bool eligible() const {
  return actor&&alive&&loaded&&inRange&&underCrosshair&&!combat&&!menuBlocked&&!companion&&!mount;
 }
};
class SessionPolicy {
 std::uint32_t selected=0;
public:
 bool begin(const TargetFacts& facts) {if(!facts.eligible())return false;selected=facts.actor;return true;}
 void end(){selected=0;}
 bool maintain(const TargetFacts& facts){if(selected!=facts.actor||!facts.eligible())end();return selected!=0;}
 bool permits(std::uint32_t actor) const{return selected&&selected==actor;}
};
}
