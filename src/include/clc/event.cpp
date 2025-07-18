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

EventBox::EventBox(cl_event event) noexcept : event_(event) {}

EventBox::EventBox(EventBox&& rhs) noexcept : event_(std::exchange(rhs.event_, nullptr)) {}

EventBox& EventBox::operator=(EventBox&& rhs) noexcept {
    event_ = std::exchange(rhs.event_, nullptr);
    return *this;
}

EventBox::~EventBox() noexcept {
    if (event_ == nullptr) return;
    clReleaseEvent(event_);
    event_ = nullptr;
}

std::expected<EventBox, Error> EventBox::create() noexcept { return {}; }

std::expected<void, Error> EventBox::wait(std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto waitEvents = waitEventBoxs | rgs::views::transform(exposeEvent) | rgs::to<std::vector>();
    const cl_int clErr = clWaitForEvents(waitEvents.size(), waitEvents.data());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};
    return {};
}

cl_event EventBox::exposeEvent(const EventBox& eventBox) noexcept { return eventBox.getEvent(); }

std::expected<cl_ulong, Error> EventBox::getElapsedTimeNs() const noexcept {
    cl_int clErr;

    cl_ulong time_start;
    cl_ulong time_end;

    clErr = clGetEventProfilingInfo(event_, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};
    clErr = clGetEventProfilingInfo(event_, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    cl_ulong timecostNs = time_end - time_start;
    return timecostNs;
}

}  // namespace clc
