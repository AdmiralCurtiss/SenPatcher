#pragma once

#include <atomic>

namespace SenTools {
struct TaskCancellation {
    TaskCancellation() = default;
    TaskCancellation(const TaskCancellation& other) = delete;
    TaskCancellation(TaskCancellation&& other) = delete;
    TaskCancellation& operator=(const TaskCancellation& other) = delete;
    TaskCancellation& operator=(TaskCancellation&& other) = delete;
    ~TaskCancellation() = default;

    bool IsCancellationRequested() const {
        return CancellationRequested.load(std::memory_order_relaxed);
    }

    void CancelTask() {
        CancellationRequested.store(true, std::memory_order_relaxed);
    }

    void Reset() {
        CancellationRequested.store(false, std::memory_order_relaxed);
    }

private:
    std::atomic<bool> CancellationRequested = false;
};
} // namespace SenTools
