#pragma once

#include "geometry/grassmann.hpp"
#include "geometry/pair.hpp"
#include "geometry/point.hpp"
#include "geometry/serialization.hpp"

namespace principia {
namespace geometry {

template<typename T1, typename T2>
Pair<T1, T2>::Pair(T1 const& t1, T2 const& t2)
    : t1_(t1),
      t2_(t2) {}

template<typename T1, typename T2>
Pair<T1, T2> Pair<T1, T2>::operator+(
    typename vector_of<Pair>::type const& right) const {
  return Pair<T1, T2>(t1_ + right.t1_, t2_ + right.t2_);
}

template<typename T1, typename T2>
Pair<T1, T2> Pair<T1, T2>::operator-(
    typename vector_of<Pair>::type const& right) const {
  return Pair<T1, T2>(t1_ - right.t1_, t2_ - right.t2_);
}

template<typename T1, typename T2>
Pair<T1, T2>& Pair<T1, T2>::operator+=(
    typename vector_of<Pair>::type const& right) {
  t1_ += right.t1_;
  t2_ += right.t2_;
  return *this;
}

template<typename T1, typename T2>
Pair<T1, T2>& Pair<T1, T2>::operator-=(
    typename vector_of<Pair>::type const& right) {
  t1_ -= right.t1_;
  t2_ -= right.t2_;
  return *this;
}

template<typename T1, typename T2>
bool Pair<T1, T2>::operator==(Pair const& right) const {
  return t1_ == right.t1_ && t2_ == right.t2_;
}

template<typename T1, typename T2>
bool Pair<T1, T2>::operator!=(Pair const& right) const {
  return t1_ != right.t1_ || t2_ != right.t2_;
}

template<typename T1, typename T2>
void Pair<T1, T2>::WriteToMessage(
    not_null<serialization::Pair*> const message) const {
  PointOrMultivectorSerializer<T1, serialization::Pair::Element>::
      WriteToMessage(t1_, message->mutable_t1());
  PointOrMultivectorSerializer<T2, serialization::Pair::Element>::
      WriteToMessage(t2_, message->mutable_t2());
}

template<typename T1, typename T2>
Pair<T1, T2> Pair<T1, T2>::ReadFromMessage(serialization::Pair const& message) {
  T1 const t1 = PointOrMultivectorSerializer<T1, serialization::Pair::Element>::
                    ReadFromMessage(message.t1());
  T2 const t2 = PointOrMultivectorSerializer<T2, serialization::Pair::Element>::
                    ReadFromMessage(message.t2());
  return {t1, t2};
}

template<typename T1, typename T2>
template<typename Weight>
void Pair<T1, T2>::BarycentreCalculator<Weight>::Add(Pair const& pair,
                                                     Weight const& weight) {
  auto const t1_weighted_sum_diff = (pair.t1_ - reference_t1_) * weight;
  auto const t2_weighted_sum_diff = (pair.t2_ - reference_t2_) * weight;
  if (empty_) {
    t1_weighted_sum_ = t1_weighted_sum_diff;
    t2_weighted_sum_ = t2_weighted_sum_diff;
    weight_ = weight;
    empty_ = false;
  } else {
    t1_weighted_sum_ += t1_weighted_sum_diff;
    t2_weighted_sum_ += t2_weighted_sum_diff;
    weight_ += weight;
  }
}

template<typename T1, typename T2>
template<typename Weight>
Pair<T1, T2> Pair<T1, T2>::BarycentreCalculator<Weight>::Get() const {
  CHECK(!empty_) << "Empty BarycentreCalculator";
  return Pair<T1, T2>(reference_t1_ + (t1_weighted_sum_ / weight_),
                      reference_t2_ + (t2_weighted_sum_ / weight_));
}

template<typename T1, typename T2>
template<typename Weight>
T1 const Pair<T1, T2>::BarycentreCalculator<Weight>::reference_t1_;

template<typename T1, typename T2>
template<typename Weight>
T2 const Pair<T1, T2>::BarycentreCalculator<Weight>::reference_t2_;

template<typename T1, typename T2>
typename vector_of<Pair<T1, T2>>::type operator-(
    typename enable_if_affine<Pair<T1, T2>>::type const& left,
    Pair<T1, T2> const& right) {
  return typename vector_of<Pair<T1, T2>>::type(left.t1_ - right.t1_,
                                                left.t2_ - right.t2_);
}

template<typename T1, typename T2>
typename enable_if_vector<Pair<T1, T2>>::type operator+(
  Pair<T1, T2> const& right) {
  return right;
}

template<typename T1, typename T2>
typename enable_if_vector<Pair<T1, T2>>::type operator-(
  Pair<T1, T2> const& right) {
  return Pair<T1, T2>(-right.t1_, -right.t2_);
}

template<typename Scalar, typename T1, typename T2>
typename enable_if_vector<
    Pair<T1, T2>,
    Pair<decltype(std::declval<Scalar>() * std::declval<T1>()),
         decltype(std::declval<Scalar>() * std::declval<T2>())>>::type
operator*(Scalar const left, Pair<T1, T2> const& right) {
  return Pair<decltype(std::declval<Scalar>() * std::declval<T1>()),
              decltype(std::declval<Scalar>() * std::declval<T2>())>(
      left * right.t1_, left * right.t2_);
}

template<typename Scalar, typename T1, typename T2>
typename enable_if_vector<
    Pair<T1, T2>,
    Pair<decltype(std::declval<T1>() * std::declval<Scalar>()),
         decltype(std::declval<T2>() * std::declval<Scalar>())>>::type
operator*(Pair<T1, T2> const& left, Scalar const right) {
  return Pair<decltype(std::declval<T1>() * std::declval<Scalar>()),
              decltype(std::declval<T2>() * std::declval<Scalar>())>(
      left.t1_ * right, left.t2_ * right);
}

template<typename Scalar, typename T1, typename T2>
typename enable_if_vector<
    Pair<T1, T2>,
    Pair<decltype(std::declval<T1>() / std::declval<Scalar>()),
         decltype(std::declval<T2>() / std::declval<Scalar>())>>::type
operator/(Pair<T1, T2> const& left, Scalar const right) {
  return Pair<decltype(std::declval<T1>() / std::declval<Scalar>()),
              decltype(std::declval<T2>() / std::declval<Scalar>())>(
      left.t1_ / right, left.t2_ / right);
}

template<typename T1, typename T2>
typename enable_if_vector<Pair<T1, T2>>::type& operator*=(
    Pair<T1, T2>& left,  // NOLINT(runtime/references)
    double const right) {
  left.t1_ *= right;
  left.t2_ *= right;
  return left;
}

template<typename T1, typename T2>
typename enable_if_vector<Pair<T1, T2>>::type& operator/=(
    Pair<T1, T2>& left,  // NOLINT(runtime/references)
    double const right) {
  left.t1_ /= right;
  left.t2_ /= right;
  return left;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, Pair<T1, T2> const& pair) {
  out << "{" << pair.t1_ << ", " << pair.t2_ << "}";
  return out;
}

}  // namespace geometry

namespace base {

template<typename Functor, typename T1, typename T2>
typename Mappable<Functor,
                geometry::Pair<T1, T2>,
                typename geometry::enable_if_vector<
                    geometry::Pair<T1, T2>, void>::type>::type
Mappable<Functor,
       geometry::Pair<T1, T2>,
       typename geometry::enable_if_vector<
          geometry::Pair<T1, T2>, void>::type>::Do(
    Functor const& functor,
    geometry::Pair<T1, T2> const& pair) {
  return type(functor(pair.t1_), functor(pair.t2_));
}

}  // namespace base

}  // namespace principia
