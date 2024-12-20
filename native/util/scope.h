#pragma once

#include <utility>

namespace HyoutaUtils {
template<typename FuncT>
struct ScopeGuard {
private:
    FuncT Func;

public:
    ScopeGuard(FuncT&& f) : Func(std::forward<FuncT>(f)) {}

    ScopeGuard(const ScopeGuard& other) = delete;
    ScopeGuard(ScopeGuard&& other) = delete;
    ScopeGuard& operator=(const ScopeGuard& other) = delete;
    ScopeGuard& operator=(ScopeGuard&& other) = delete;

    ~ScopeGuard() {
        Func();
    }
};

template<typename FuncT>
ScopeGuard<FuncT> MakeScopeGuard(FuncT&& f) {
    return ScopeGuard<FuncT>(std::forward<FuncT>(f));
};
} // namespace HyoutaUtils
