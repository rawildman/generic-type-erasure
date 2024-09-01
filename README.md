# TypeErased: A prototype for a generic type erasure wrapper in c++.

## Introduction


The purpose of this wrapper is to help alleviate some of the boiler-plate code necessary for some implementations of type erasure.
The wrapper `gte::TypeErased` can be constructed with any object that has member functions that match the signatures provided to the wrapper declaration.
Member functions are identified with type tags, and called using tag dispatch.

## Usage

As an example, a wrapper for an object with a single const `Speak` member is defined as:

```cpp
struct Speak{};
using SpeakFunction = gte::ConstMemberSignature<Speak, void()>;
using Speaker = gte::TypeErased<SpeakFunction>;
```

This example defines a `TypeErased` type specialized for a const member function that may be called with the `Speak` struct using tag dispatch.
The wrapper can be then used as:

```cpp
struct Dog
{
  void speak() const { std::cout << "Woof!\n"; }
};

const auto dog = Speaker{Dog{}, &Dog::speak};
dog.call<Speak>();
```

The structs `gte::ConstMemberSignature` and `gte::MemberSignature` are helper structs to provide the tag struct and function signature to the wrapper.
`gte::ConstMemberSignature` defines the corresponding `call` member as a const member function and `gte::MemberSignature` defines `call` as a non-const member.

A type-erased wrapper object is constructed from an instance of a type that has a member matching the signature of the wrapper.
In this case, the struct `Dog` has a const member `speak`, a pointer to which is used on construction.

Constness is enforced on construction and in the call member: a const wrapper member must call a const member of the erased type and a const object cannot call a non-const member.
For example, the following will not compile:

```cpp
struct Cat
{
  void purr() { std::cout << "purr\n"; }
  void meow() const { std::cout << "meow\n"; }
};

const auto cat1 = Speaker{Cat{}, &Cat::meow}; // OK
const auto cat2 = Speaker{Cat{}, &Cat::purr}; // Will not compile, constness of member functions does not match
```

nor will

```cpp
struct GiveTreat{};
using GiveTreatFunction = gte::MemberSignature<GiveTreat, void(int)>;
struct Weight{};
using WeightFunction = gte::ConstMemberSignature<Weight, int()>;
using TreatEater = gte::TypeErased<GiveTreatFunction, WeightFunction>;

struct Cat
{
  int m_weight = 10;
  void take_treat(const int num_treats) { m_weight += num_treats; }
  int weight() const { return m_weight; }
};

const auto cat = TreatEater{Cat{20}, &Cat::take_treat, &Cat::weight};
cat.call<GiveTreat>(1); // Cannot call non-const function with a const object
```

A wrapper cannot be constructed by mixing types and member functions.
For example, the following will not compile:

```cpp
const auto dog_cat_hybrid = Speaker{Dog{}, &Cat::meow};
```

## Full example

The following demonstrates a type-erased `Pet` wrapper, to which `Dog` and `Cat` objects are assigned.
The objects are stored in a vector and each member function is called to demonstrate the usage of the wrapper object.
Also, helper functions are defined for creating the type-erased wrapper from each object to avoid repeating the member function pointer arguments to the wrapper constructor.


```cpp
struct Speak{};
using SpeakFunction = gte::ConstMemberSignature<Speak, void()>;

struct GiveTreat{};
using GiveTreatFunction = gte::MemberSignature<GiveTreat, void(int)>;

struct TakeForAWalk{};
using TakeForAWalkFunction = gte::MemberSignature<TakeForAWalk, void()>;

struct Weight{};
using WeightFunction = gte::ConstMemberSignature<Weight, int()>;
using Pet = gte::TypeErased<SpeakFunction, GiveTreatFunction, TakeForAWalkFunction, WeightFunction>;

struct Cat
{
  int m_weight = 10;

  void meow() const { std::cout << "Meow!\n"; }
  void take_treat(const int num_treats) { m_weight += num_treats; }
  void walk() { m_weight -= 1; }
  int weight() const { return m_weight; }
};

auto pet_from_cat(Cat cat)
{
  return Pet{std::move(cat), &Cat::meow, &Cat::take_treat, &Cat::walk, &Cat::weight};
}

struct Dog
{
  int m_weight = 10;

  void bark() const { std::cout << "Woof!\n"; }
  void give_treat(const int num_treats) { m_weight += 2 * num_treats; }
  void walk() { m_weight -= 2; }
  int weight() const { return m_weight; }
};

auto pet_from_dog(Dog dog)
{
  return Pet{std::move(dog), &Dog::bark, &Dog::give_treat, &Dog::walk, &Dog::weight};
}

int main()
{
  auto my_pets = std::vector<Pet>{pet_from_cat(Cat{10}), pet_from_cat(Cat{20}), pet_from_dog(Dog{50})};

  for(auto& pet : my_pets)
  {
    pet.call<TakeForAWalk>();
    pet.call<GiveTreat>(10); // What a good boy!
    std::cout << "Current weight: " << pet.call<Weight>() << "\n";
    pet.call<Speak>();
  }
}

```

