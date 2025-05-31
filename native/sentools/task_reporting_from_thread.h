#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "task_reporting.h"

namespace SenTools {
struct TaskReportingFromThread final : public TaskReporting {
    TaskReportingFromThread(bool active = true) : TaskReporting(active) {}
    TaskReportingFromThread(const TaskReportingFromThread& other) = delete;
    TaskReportingFromThread(TaskReportingFromThread&& other) = delete;
    TaskReportingFromThread& operator=(const TaskReportingFromThread& other) = delete;
    TaskReportingFromThread& operator=(TaskReportingFromThread&& other) = delete;
    ~TaskReportingFromThread() override = default;

    std::optional<std::string> DrainAndGetLast() {
        std::lock_guard lock(Mutex);
        if (Messages.empty()) {
            return std::nullopt;
        }
        std::string m = std::move(Messages.back());
        Messages.clear();
        return m;
    }

    void Reset() {
        std::lock_guard lock(Mutex);
        Messages.clear();
    }

protected:
    void ReportMessageInternal(std::string message) override {
        std::lock_guard lock(Mutex);
        Messages.emplace_back(std::move(message));
    }

private:
    std::mutex Mutex;
    std::vector<std::string> Messages;
};
} // namespace SenTools
