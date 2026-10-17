#pragma once
#include <cmath>
#include <mutex>
#include <algorithm>
class DistanceOverride {
    std::mutex mutex_; const void* owner_=nullptr; float value_=1.0F;
public:
    bool Set(const void* owner,float value) {
        if(!owner || !std::isfinite(value) || value<0.1F || value>1.0F) return false;
        std::lock_guard lock(mutex_);
        if(owner_ && owner_!=owner) return false;
        owner_=owner;value_=value;return true;
    }
    bool Release(const void* owner) {
        std::lock_guard lock(mutex_);
        if(!owner || owner_!=owner) return false;
        owner_=nullptr;return true;
    }
    bool Active() {std::lock_guard lock(mutex_);return owner_!=nullptr;}
    float Effective(float saved) {std::lock_guard lock(mutex_);return owner_ ? value_ : saved;}
};
