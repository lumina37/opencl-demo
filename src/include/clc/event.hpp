#pragma once

#include <expected>
#include <functional>
#include <span>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

class EventManager {
    EventManager(cl_event event) noexcept;

public:
    EventManager() noexcept = default;
    EventManager(EventManager&& rhs) noexcept;
    ~EventManager() noexcept;

    [[nodiscard]] static std::expected<EventManager, Error> create() noexcept;

    [[nodiscard]] static std::expected<void, Error> wait(
        std::span<std::reference_wrapper<const EventManager>> eventMgrs) noexcept;

    [[nodiscard]] static cl_event exposeEvent(const EventManager& eventMgr) noexcept;

    [[nodiscard]] cl_event getEvent() const noexcept { return event_; }
    [[nodiscard]] cl_event* getPEvent() noexcept { return &event_; }

    [[nodiscard]] std::expected<cl_ulong, Error> getElapsedTimeNs() const noexcept;

private:
    cl_event event_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/event.cpp"
#endif
