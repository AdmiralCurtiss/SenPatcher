#pragma once

#include <atomic>
#include <cassert>
#include <functional>
#include <future>
#include <optional>
#include <thread>
#include <utility>

#include "util/scope.h"

namespace SenTools::GUI {
template<typename ResultT, typename... ArgsT>
struct BackgroundTask {
    BackgroundTask(std::function<ResultT(ArgsT...)> workFunction)
      : WorkFunction(std::move(workFunction)) {}
    BackgroundTask(const BackgroundTask& other) = delete;
    BackgroundTask(BackgroundTask&& other) = delete;
    BackgroundTask& operator=(const BackgroundTask& other) = delete;
    BackgroundTask& operator=(BackgroundTask&& other) = delete;
    ~BackgroundTask() {
        if (Engaged()) {
            if (Data->Thread.joinable()) {
                Data->Thread.join();
            }
            Data.reset();
        }
    }

    // Returns true if Task is in progress or has result available.
    bool Engaged() const {
        return Data.has_value();
    }

    // Returns true if result is available.
    bool ResultAvailable() const {
        return Engaged() && Data->Done.load(std::memory_order_acquire);
    }

    // Start the task
    void Engage(ArgsT... args) {
        assert(!Engaged());
        Data.emplace();
        Data->Thread =
            std::thread(std::bind(&BackgroundTask::ThreadFunc, this, std::move(args)...));
    }

    // Fetches the result and returns to the un-engaged state.
    // Will block until the result is ready if the result is not ready yet!
    // If the task has ended with a thrown exception it will be rethrown.
    ResultT FetchResultAndDisengage() {
        assert(Engaged());
        auto resetGuard = HyoutaUtils::MakeScopeGuard([this] {
            if (Data->Thread.joinable()) {
                Data->Thread.join();
            }
            Data.reset();
        });
        return Data->Future.get();
    }

private:
    struct ThreadData {
        std::atomic<bool> Done;
        std::promise<ResultT> Promise;
        std::future<ResultT> Future;
        std::thread Thread;

        ThreadData() : Done(false), Promise(), Future(Promise.get_future()) {}
    };
    std::function<ResultT(ArgsT...)> WorkFunction;
    std::optional<ThreadData> Data;

    void ThreadFunc(ArgsT... args) {
        auto doneGuard = HyoutaUtils::MakeScopeGuard(
            [this] { Data->Done.store(true, std::memory_order_release); });
        try {
            Data->Promise.set_value(WorkFunction(std::move(args)...));
        } catch (...) {
            Data->Promise.set_exception(std::current_exception());
        }
    }
};
} // namespace SenTools::GUI
