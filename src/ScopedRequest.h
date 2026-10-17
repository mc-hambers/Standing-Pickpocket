#pragma once
// A thread-local request identifies only the intended reference. No actor state changes.
template<class T> class ScopedRequest {
 T& slot_;T previous_;
public:
 ScopedRequest(T& slot,T value) noexcept:slot_(slot),previous_(slot){slot_=value;}
 ~ScopedRequest() noexcept{slot_=previous_;}
 ScopedRequest(const ScopedRequest&)=delete;
 ScopedRequest& operator=(const ScopedRequest&)=delete;
};
