#pragma once

// A native activation/text call can use standing logic without changing animation
// or leaving the player unsneaked. Restore the single bit on every return path.
template <class State>
class ScopedStanding {
    State* state_;
    bool restore_;
public:
    ScopedStanding(State* state, bool enabled) noexcept
        : state_(state), restore_(enabled && state && state->actorState1.sneaking) {
        if (restore_) state_->actorState1.sneaking = 0;
    }
    ~ScopedStanding() noexcept {
        if (restore_) state_->actorState1.sneaking = 1;
    }
    ScopedStanding(const ScopedStanding&) = delete;
    ScopedStanding& operator=(const ScopedStanding&) = delete;
};

