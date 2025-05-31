#pragma once

#include <string>

#include "task_reporting.h"

namespace SenTools {
struct TaskReportingDummy final : public TaskReporting {
    TaskReportingDummy() : TaskReporting(false) {}
    TaskReportingDummy(const TaskReportingDummy& other) = delete;
    TaskReportingDummy(TaskReportingDummy&& other) = delete;
    TaskReportingDummy& operator=(const TaskReportingDummy& other) = delete;
    TaskReportingDummy& operator=(TaskReportingDummy&& other) = delete;
    ~TaskReportingDummy() override = default;

protected:
    void ReportMessageInternal(std::string message) override {}
};
} // namespace SenTools
