#include <expected>
#include <functional>
#include <ranges>
#include <span>
#include <utility>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/event.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

EventManager::EventManager(cl_event&& event) noexcept : event_(std::move(event)) {}

EventManager::EventManager(EventManager&& rhs) noexcept : event_(std::exchange(rhs.event_, nullptr)) {}

EventManager::~EventManager() noexcept {
    if (event_ == nullptr) return;
    clReleaseEvent(event_);
    event_ = nullptr;
}

std::expected<EventManager, Error> EventManager::create() noexcept { return {}; }

std::expected<void, Error> EventManager::wait(
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    auto waitEvents = waitEventMgrs | rgs::views::transform(leak) | rgs::to<std::vector>();
    const cl_int clErr = clWaitForEvents(waitEvents.size(), waitEvents.data());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

cl_event EventManager::leak(const EventManager& eventMgr) noexcept { return eventMgr.getEvent(); }

std::expected<cl_ulong, Error> EventManager::getElapsedTimeNs() const noexcept {
    cl_int clErr;

    cl_ulong time_start;
    cl_ulong time_end;

    clErr = clGetEventProfilingInfo(event_, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    clErr = clGetEventProfilingInfo(event_, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    cl_ulong timecostNs = time_end - time_start;
    return timecostNs;
}

}  // namespace clc
