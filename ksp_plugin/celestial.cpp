#include "ksp_plugin/celestial.hpp"

namespace principia {
namespace ksp_plugin {

template<typename... Args>
Celestial::Celestial(Args&&... args)  // NOLINT(build/c++11)
    : body_(new Body<Barycentric>(
                    std::forward<Args>(args)...)) {}  // NOLINT(build/c++11)

Body<Barycentric> const& Celestial::body() const {
  return *body_;
}

bool Celestial::has_parent() const {
  return parent_ != nullptr;
}

Celestial const& Celestial::parent() const {
  return *CHECK_NOTNULL(parent_);
}

Trajectory<Barycentric> const& Celestial::history() const {
  return *history_;
}

Trajectory<Barycentric> const& Celestial::prolongation() const {
  return *prolongation_;
}

Trajectory<Barycentric>* Celestial::mutable_history() {
  return history_.get();
}

Trajectory<Barycentric>* Celestial::mutable_prolongation() {
  return prolongation_;
}

void Celestial::set_parent(Celestial const* parent) {
  parent_ = CHECK_NOTNULL(parent);
}

void Celestial::CreateHistoryAndForkProlongation(
    Instant const& time,
    DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
  history_ = std::make_unique<Trajectory<Barycentric>>(*body_);
  history_->Append(time, degrees_of_freedom);
  prolongation_ = history_->Fork(time);
}

void Celestial::ResetProlongation(Instant const& time) {
  history_->DeleteFork(&prolongation_);
  prolongation_ = history_->Fork(time);
}

}  // namespace ksp_plugin
}  // namespace principia
