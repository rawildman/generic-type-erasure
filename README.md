# generic-type-erasure
A prototype for a generic type erasure wrapper in c++.

The purpose of this wrapper is to help alleviate some of the boiler-plate code necessary for some implementations of type erasure.
The wrapper `gte::TypeErased` can be constructed with any object that has member functions that match the signatures provided to the wrapper declaration.
Member functions are identified with type tags, and called using tag dispatch.

For example, a wrapper for an object with a single const `Speak` member is defined as:

```cpp
struct Speak{};
using SpeakFunction = gte::TagAndConstSignature<Speak, void()>;
using Speaker = gte::TypeErased<SpeakFunction>;
```

This example defines a TypeErased type specialized for a const member function that may be called with the `Speak` struct using tag dispatch.
The wrapper can be then used as:

```cpp
struct Dog
{
  void speak() const { std::cout << "Woof!\n"; }
};

const auto dog = Speaker{Dog{}, &Dog::speak};
dog.call<Speak>();
```

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

const auto cat = Speaker{Cat{}, &Cat::purr};
```

nor will

```cpp
struct GiveTreat{};
using GiveTreatFunction = gte::TagAndSignature<GiveTreat, void(int)>;
struct Weight{};
using WeightFunction = gte::TagAndConstSignature<Weight, int()>;
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

