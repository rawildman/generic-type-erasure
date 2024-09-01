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
  // const auto cat2 = Speaker{Cat{}, &Cat::purr}; // Will not compile,
  // constness of member functions does not match
}

TEST_CASE("Treat eater", "[examples]") {
  const auto cat = TreatEater{Cat{20}, &Cat::take_treat, &Cat::weight};
  // cat.call<GiveTreat>(1); // Cannot call non-const function with a const
  // object
}

TEST_CASE("No hybrids", "[examples]") {
  //  const auto dog_cat_hybrid = Speaker{Dog{}, &Cat::meow}; // Cannot mix
  //  types and member functions
}

namespace full_example {
struct Speak {};
using SpeakFunction = gte::TagAndConstSignature<Speak, void()>;

struct GiveTreat {};
using GiveTreatFunction = gte::TagAndSignature<GiveTreat, void(int)>;

struct TakeForAWalk {};
using TakeForAWalkFunction = gte::TagAndSignature<TakeForAWalk, void()>;

struct Weight {};
using WeightFunction = gte::TagAndConstSignature<Weight, int()>;
using Pet = gte::TypeErased<SpeakFunction, GiveTreatFunction,
                            TakeForAWalkFunction, WeightFunction>;

struct Cat {
  int m_weight = 10;

  void meow() const { std::cout << "Meow!\n"; }
  void take_treat(const int num_treats) { m_weight += num_treats; }
  void walk() { m_weight -= 1; }
  int weight() const { return m_weight; }
};

auto pet_from_cat(Cat cat) {
  return Pet{std::move(cat), &Cat::meow, &Cat::take_treat, &Cat::walk,
             &Cat::weight};
}

struct Dog {
  int m_weight = 10;

  void bark() const { std::cout << "Woof!\n"; }
  void give_treat(const int num_treats) { m_weight += 2 * num_treats; }
  void walk() { m_weight -= 2; }
  int weight() const { return m_weight; }
};

auto pet_from_dog(Dog dog) {
  return Pet{std::move(dog), &Dog::bark, &Dog::give_treat, &Dog::walk,
             &Dog::weight};
}
}  // namespace full_example

TEST_CASE("Full pet example", "[examples]") {
  namespace fe = full_example;
  auto my_pets = std::vector<fe::Pet>{fe::pet_from_cat(fe::Cat{10}),
                                      fe::pet_from_cat(fe::Cat{20}),
                                      fe::pet_from_dog(fe::Dog{50})};

  for (auto& pet : my_pets) {
    pet.call<fe::TakeForAWalk>();
    pet.call<fe::GiveTreat>(10);  // What a good boy!
    std::cout << "Current weight: " << pet.call<fe::Weight>() << "\n";
    pet.call<fe::Speak>();
  }
}
