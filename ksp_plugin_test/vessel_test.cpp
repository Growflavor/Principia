﻿
#include "ksp_plugin/vessel.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "physics/ephemeris.hpp"
#include "physics/solar_system.hpp"

namespace principia {

using physics::Ephemeris;
using physics::SolarSystem;
using quantities::si::Kilo;
using quantities::si::Kilogram;
using quantities::si::Metre;
using quantities::si::Second;

namespace ksp_plugin {

// TODO(egg): We would want to use a real ephemeris to properly exercise the
// limit cases.
class VesselTest : public testing::Test {
 protected:
  VesselTest()
      : adaptive_parameters_(
            DormandElMikkawyPrince1986RKN434FM<Position<Barycentric>>(),
            /*max_steps=*/1000,
            /*length_integration_tolerance=*/1 * Metre,
            /*speed_integration_tolerance=*/1 * Metre / Second),
        ephemeris_fixed_parameters_(
            McLachlanAtela1992Order5Optimal<Position<Barycentric>>(),
            /*step=*/10 * Second),
        history_fixed_parameters_(
            McLachlanAtela1992Order5Optimal<Position<Barycentric>>(),
            /*step=*/1 * Second) {
    solar_system_.Initialize(
        SOLUTION_DIR / "astronomy" / "gravity_model_two_bodies_test.proto.txt",
        SOLUTION_DIR / "astronomy" / "initial_state_two_bodies_test.proto.txt");
    ephemeris_ = solar_system_.MakeEphemeris(
        /*fitting_tolerance=*/1 * Metre, ephemeris_fixed_parameters_);
    earth_ = std::make_unique<Celestial>(
        solar_system_.massive_body(*ephemeris_, "Earth"));
    vessel_ = std::make_unique<Vessel>(earth_.get(),
                                       ephemeris_.get(),
                                       adaptive_parameters_,
                                       history_fixed_parameters_);
    t0_ = solar_system_.epoch();
    t1_ = t0_ + 11.1 * Second;
    t2_ = t1_ + 22.2 * Second;
    t3_ = t2_ + 33.3 * Second;
  }

  SolarSystem<Barycentric> solar_system_;
  std::unique_ptr<Ephemeris<Barycentric>> ephemeris_;
  std::unique_ptr<Celestial> earth_;
  Ephemeris<Barycentric>::AdaptiveStepParameters const adaptive_parameters_;
  Ephemeris<Barycentric>::FixedStepParameters const ephemeris_fixed_parameters_;
  Ephemeris<Barycentric>::FixedStepParameters const history_fixed_parameters_;
  std::unique_ptr<Vessel> vessel_;
  DegreesOfFreedom<Barycentric> d1_ = {
      Barycentric::origin +
          Displacement<Barycentric>(
              {1 * Kilo(Metre), 2 * Kilo(Metre), 3 * Kilo(Metre)}),
      Velocity<Barycentric>({4 * Kilo(Metre) / Second,
                             5 * Kilo(Metre) / Second,
                             6 * Kilo(Metre) / Second})};
  DegreesOfFreedom<Barycentric> d2_ = {
      Barycentric::origin +
          Displacement<Barycentric>(
              {11 * Kilo(Metre), 12 * Kilo(Metre), 13 * Kilo(Metre)}),
      Velocity<Barycentric>({14 * Kilo(Metre) / Second,
                             15 * Kilo(Metre) / Second,
                             16 * Kilo(Metre) / Second})};
  Instant t0_;
  Instant t1_;
  Instant t2_;
  Instant t3_;
};

using VesselDeathTest = VesselTest;

TEST_F(VesselDeathTest, Uninitialized) {
  EXPECT_DEATH({
    vessel_->history();
  }, "is_initialized");
  EXPECT_DEATH({
    vessel_->prolongation();
  }, "is_initialized");
}

TEST_F(VesselTest, Initialization) {
  EXPECT_FALSE(vessel_->is_initialized());
  vessel_->CreateHistoryAndForkProlongation(t2_, d2_);
  EXPECT_TRUE(vessel_->is_initialized());
  auto const& prolongation = vessel_->prolongation();
  EXPECT_EQ(t2_, prolongation.last().time());
  auto const& history = vessel_->history();
  EXPECT_EQ(t2_, history.last().time());
  EXPECT_FALSE(vessel_->has_flight_plan());
  EXPECT_FALSE(vessel_->has_prediction());
}

TEST_F(VesselTest, Dirty) {
  EXPECT_FALSE(vessel_->is_dirty());
  vessel_->set_dirty();
  EXPECT_TRUE(vessel_->is_dirty());
}

TEST_F(VesselTest, Parent) {
  Celestial celestial(earth_->body());
  EXPECT_EQ(earth_.get(), vessel_->parent());
  vessel_->set_parent(&celestial);
  EXPECT_EQ(&celestial, vessel_->parent());
}

TEST_F(VesselTest, AdvanceTimeInBubble) {
  vessel_->CreateHistoryAndForkProlongation(t1_, d1_);
  vessel_->AdvanceTimeInBubble(t2_, d2_);
  EXPECT_EQ(t2_ - 0.2 * Second, vessel_->history().last().time());
  EXPECT_EQ(t2_, vessel_->prolongation().last().time());
  EXPECT_EQ(d2_, vessel_->prolongation().last().degrees_of_freedom());
  EXPECT_TRUE(vessel_->is_dirty());
}

TEST_F(VesselTest, AdvanceTimeNotInBubble) {
  vessel_->CreateHistoryAndForkProlongation(t1_, d1_);
  vessel_->AdvanceTimeNotInBubble(t2_);
  EXPECT_EQ(t2_ - 0.2 * Second, vessel_->history().last().time());
  EXPECT_EQ(t2_, vessel_->prolongation().last().time());
  EXPECT_NE(d2_, vessel_->prolongation().last().degrees_of_freedom());
  EXPECT_FALSE(vessel_->is_dirty());
}

TEST_F(VesselTest, Prediction) {
  vessel_->CreateHistoryAndForkProlongation(t1_, d1_);
  vessel_->AdvanceTimeNotInBubble(t2_);
  EXPECT_FALSE(vessel_->has_prediction());
  vessel_->UpdatePrediction(t3_, adaptive_parameters_);
  EXPECT_TRUE(vessel_->has_prediction());
  EXPECT_LE(t3_, vessel_->prediction().last().time());
  vessel_->DeletePrediction();
  EXPECT_FALSE(vessel_->has_prediction());
}

TEST_F(VesselDeathTest, SerializationError) {
  EXPECT_DEATH({
    serialization::Vessel message;
    vessel_->WriteToMessage(&message);
  }, "is_initialized");
  EXPECT_DEATH({
    serialization::Vessel message;
    Vessel::ReadFromMessage(message, ephemeris_.get(), earth_.get());
  }, "message.has_history");
}

TEST_F(VesselTest, SerializationSuccess) {
  serialization::Vessel message;
  EXPECT_FALSE(message.has_history());
  vessel_->CreateHistoryAndForkProlongation(t2_, d2_);
  vessel_->AdvanceTimeNotInBubble(t2_);
  EXPECT_FALSE(vessel_->has_prediction());
  vessel_->UpdatePrediction(t3_, adaptive_parameters_);

  vessel_->WriteToMessage(&message);
  EXPECT_TRUE(message.has_history());
  EXPECT_TRUE(message.has_prediction_last_time());
  vessel_ = Vessel::ReadFromMessage(message, ephemeris_.get(), earth_.get());
  EXPECT_TRUE(vessel_->is_initialized());
}

}  // namespace ksp_plugin
}  // namespace principia
