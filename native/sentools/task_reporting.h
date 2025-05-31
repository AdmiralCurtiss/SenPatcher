#pragma once

#include <format>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SenTools {
struct TaskReporting {
    TaskReporting(bool active) : ReportingActive(active) {}
    TaskReporting(const TaskReporting& other) = delete;
    TaskReporting(TaskReporting&& other) = delete;
    TaskReporting& operator=(const TaskReporting& other) = delete;
    TaskReporting& operator=(TaskReporting&& other) = delete;
    virtual ~TaskReporting() = default;

    void ReportMessage(std::string_view message) {
        if (ReportingActive) {
            ReportMessageInternal(std::string(message));
        }
    }

    template<typename... ArgsT>
    void ReportMessageFmt(std::format_string<ArgsT...> fmt, ArgsT&&... args) {
        if (ReportingActive) {
            ReportMessageInternal(
                std::format<ArgsT...>(std::move(fmt), std::forward<ArgsT>(args)...));
        }
    }

protected:
    virtual void ReportMessageInternal(std::string message) = 0;

private:
    bool ReportingActive;
};

struct DummyTaskReporting : public TaskReporting {
    DummyTaskReporting() : TaskReporting(false) {}
    DummyTaskReporting(const DummyTaskReporting& other) = delete;
    DummyTaskReporting(DummyTaskReporting&& other) = delete;
    DummyTaskReporting& operator=(const DummyTaskReporting& other) = delete;
    DummyTaskReporting& operator=(DummyTaskReporting&& other) = delete;
    ~DummyTaskReporting() override = default;

protected:
    void ReportMessageInternal(std::string message) override {}
};

struct ThreadTaskReporting : public TaskReporting {
    ThreadTaskReporting(bool active = true) : TaskReporting(active) {}
    ThreadTaskReporting(const ThreadTaskReporting& other) = delete;
    ThreadTaskReporting(ThreadTaskReporting&& other) = delete;
    ThreadTaskReporting& operator=(const ThreadTaskReporting& other) = delete;
    ThreadTaskReporting& operator=(ThreadTaskReporting&& other) = delete;
    ~ThreadTaskReporting() override = default;

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
