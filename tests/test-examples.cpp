#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "generic-type-erasure.hpp"

namespace {
struct Speak {};
using SpeakFunction = gte::TagAndConstSignature<Speak, void()>;
using Speaker = gte::TypeErased<SpeakFunction>;

struct Dog {
  void speak() const { std::cout << "Woof!\n"; }
};

struct Cat {
  void purr() { std::cout << "purr\n"; }
  void meow() const { std::cout << "meow\n"; }

  int m_weight = 10;
  void take_treat(const int num_treats) { m_weight += num_treats; }
  int weight() const { return m_weight; }
};

struct GiveTreat {};
using GiveTreatFunction = gte::TagAndSignature<GiveTreat, void(int)>;
struct Weight {};
using WeightFunction = gte::TagAndConstSignature<Weight, int()>;

using TreatEater = gte::TypeErased<GiveTreatFunction, WeightFunction>;
}  // namespace

TEST_CASE("Speaker", "[examples]") {
  const auto dog = Speaker{Dog{}, &Dog::speak};
  dog.call<Speak>();

  // const auto cat = Speaker{Cat{}, &Cat::purr}; // Constness does not match
  auto cat = Speaker{Cat{}, &Cat::meow};
  cat.call<Speak>();  // Calling const member with a non-const object ok
}

TEST_CASE("Treat eater", "[examples]") {
  const auto cat = TreatEater{Cat{20}, &Cat::take_treat, &Cat::weight};
  // cat.call<GiveTreat>(1); // Cannot call non-const function with a const
  // object
}

TEST_CASE("No hybrids", "[examples]") {
  const auto dog_cat_hybrid = Speaker{Dog{}, &Cat::meow};
}
