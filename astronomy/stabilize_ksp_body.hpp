﻿#pragma once

#include "astronomy/stabilize_ksp.hpp"

#include "physics/kepler_orbit.hpp"
#include "quantities/numbers.hpp"
#include "quantities/si.hpp"

namespace principia {
namespace astronomy {
namespace stabilize_ksp_internal {

using geometry::Position;
using quantities::si::Degree;

template<typename Frame>
void StabilizeKSP(SolarSystem<Frame>& solar_system) {
  KeplerianElements<Frame> laythe_elements =
      solar_system.MakeKeplerianElements(
          solar_system.keplerian_initial_state_message("Laythe").elements());
  KeplerianElements<Frame> vall_elements =
      solar_system.MakeKeplerianElements(
          solar_system.keplerian_initial_state_message("Vall").elements());
  KeplerianElements<Frame> tylo_elements =
      solar_system.MakeKeplerianElements(
          solar_system.keplerian_initial_state_message("Tylo").elements());
  KeplerianElements<Frame> bop_elements =
      solar_system.MakeKeplerianElements(
          solar_system.keplerian_initial_state_message("Bop").elements());
  KeplerianElements<Frame> pol_elements =
      solar_system.MakeKeplerianElements(
          solar_system.keplerian_initial_state_message("Pol").elements());

  // Instead of putting the moons in a 1:2:4 resonance, put them in a
  // 1:4/φ:16/φ^2 dissonance.
  vall_elements.mean_motion = *laythe_elements.mean_motion / (4.0 / φ);
  *tylo_elements.mean_motion =
      *laythe_elements.mean_motion / (16.0 / (φ * φ));

  // All hail Retrobop!
  bop_elements.inclination = 180 * Degree - bop_elements.inclination;
  *bop_elements.mean_motion = *pol_elements.mean_motion / 0.7;

  solar_system.ReplaceElements("Vall", vall_elements);
  solar_system.ReplaceElements("Tylo", tylo_elements);
  solar_system.ReplaceElements("Bop", bop_elements);
}

}  // namespace stabilize_ksp_internal
}  // namespace astronomy
}  // namespace principia
