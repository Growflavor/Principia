#pragma once

#include "astronomy/frames.hpp"
#include "base/macros.hpp"
#include "ksp_plugin/frames.hpp"

#include "physics/body.hpp"
#include "physics/massive_body.hpp"
#include "physics/massless_body.hpp"
#include "physics/oblate_body.hpp"
#include "physics/rotating_body.hpp"
#include "physics/ephemeris.hpp"
#include "physics/continuous_trajectory.hpp"

namespace principia {
namespace physics {

#if OS_WIN
PHYSICS_DLL void LogPhysicsDLLBaseAddress();
#endif

PHYSICS_DLL_TEMPLATE_CLASS
    internal_rotating_body::RotatingBody<ksp_plugin::Barycentric>;
PHYSICS_DLL_TEMPLATE_CLASS
    internal_oblate_body::OblateBody<ksp_plugin::Barycentric>;
PHYSICS_DLL_TEMPLATE_CLASS internal_continuous_trajectory::ContinuousTrajectory<
    ksp_plugin::Barycentric>;
PHYSICS_DLL_TEMPLATE_CLASS
    internal_ephemeris::Ephemeris<ksp_plugin::Barycentric>;

// For the plugin tests.
// TODO(egg): ifdef this out for the actual plugin.
PHYSICS_DLL_TEMPLATE_CLASS
    internal_rotating_body::RotatingBody<astronomy::ICRFJ2000Equator>;
PHYSICS_DLL_TEMPLATE_CLASS
    internal_oblate_body::OblateBody<astronomy::ICRFJ2000Equator>;

}  // namespace physics
}  // namespace principia
