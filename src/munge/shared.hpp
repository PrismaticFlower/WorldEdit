#pragma once

#include "os/process.hpp"

namespace we::munge {

constexpr inline os::process_priority munge_process_priority =
   os::process_priority::below_normal;

}