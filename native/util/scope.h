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
struct DisposableScopeGuard {
private:
    FuncT Func;
    bool Disposed = false;

public:
    DisposableScopeGuard(FuncT&& f, bool disposed = false)
      : Func(std::forward<FuncT>(f)), Disposed(disposed) {}

    DisposableScopeGuard(const DisposableScopeGuard& other) = delete;
    DisposableScopeGuard(DisposableScopeGuard&& other) = delete;
    DisposableScopeGuard& operator=(const DisposableScopeGuard& other) = delete;
    DisposableScopeGuard& operator=(DisposableScopeGuard&& other) = delete;

    void Dispose() {
        Disposed = true;
    }

    ~DisposableScopeGuard() {
        if (!Disposed) {
            Func();
        }
    }
};

template<typename FuncT>
ScopeGuard<FuncT> MakeScopeGuard(FuncT&& f) {
    return ScopeGuard<FuncT>(std::forward<FuncT>(f));
};

template<typename FuncT>
DisposableScopeGuard<FuncT> MakeDisposableScopeGuard(FuncT&& f, bool disposed = false) {
    return DisposableScopeGuard<FuncT>(std::forward<FuncT>(f), disposed);
};
} // namespace HyoutaUtils
