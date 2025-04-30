#pragma once

#include "clc/device/context.hpp"
#include "clc/device/devices.hpp"
#include "clc/device/helper.hpp"
#include "clc/device/manager.hpp"
#include "clc/device/props.hpp"
#include "clc/device/score.hpp"

namespace clc {

static_assert(CDeviceProps<DeviceProps>);

using DeviceWithProps = DeviceWithProps_<DeviceProps>;
using Devices = Devices_<DeviceProps>;

}  // namespace clc
