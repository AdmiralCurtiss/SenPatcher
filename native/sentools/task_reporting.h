#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>

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
} // namespace SenTools
