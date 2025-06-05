#pragma once

#include <expected>
#include <functional>
#include <span>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

class EventBox {
    EventBox(cl_event event) noexcept;

public:
    EventBox() noexcept = default;
    EventBox(EventBox&& rhs) noexcept;
    ~EventBox() noexcept;

    [[nodiscard]] static std::expected<EventBox, Error> create() noexcept;

    [[nodiscard]] static std::expected<void, Error> wait(
        std::span<std::reference_wrapper<const EventBox>> eventBoxs) noexcept;

    [[nodiscard]] static cl_event exposeEvent(const EventBox& eventBox) noexcept;

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
