#pragma once

/*
	RandFS (Random Fast and Simple) single header file, by Diego Taques Pimenta Quintanilha.
	Questions and feedback are welcome (diego.quintanilha@hotmail.com).

	In order to use this file, do this:
		#define RANDFS_IMPLEMENTATION
	before you include this file in one C++ file to create the implementation.

	i.e. one of your source files should look like this:
		#define RANDFS_IMPLEMENTATION
		#include "RandFS.h"

	while all other source files that use RandFS can just do this:
		#include "RandFS.h"

	Examples of usage and benchmarks are available at https://github.com/diegoquintanilha/RandFS/ (COMING SOON!)
	The core PRNG of this library is based on Takuji Nishimura and Makoto Matsumoto's MT19937-64.
	The core hash of this library is based on an implementation of CityHash, by Google, which can be found at http://code.google.com/p/cityhash/.

EXAMPLE USAGE:
	To use the Random class to generate random values, simply create an instance of the class.
	You can give the constructor a seed, and then call any of the available member functions:

		Random rand(42);
		std::cout << rand.IntBetween(5, 10) << std::endl;
		std::cout << rand.FloatNormal()     << std::endl;

	To hash values using the Hash class, simply call any of its static member functions directly.
	As a pure static class, it requires no instantiation. Just provide the value(s) you want to hash as arguments:

		uint64_t n = 12345ULL;
		std::cout << Hash::IntBetween(n, 5, 10) << std::endl;
		std::cout << Hash::FloatNormal(n)       << std::endl;

	The example file at https://github.com/diegoquintanilha/RandFS/Examples/RandFS (COMING SOON!) demonstrates how to use most of the available functions

ADDITIONAL CONFIGURATION:
	You can disable parts of this library by doing this:
		#define RANDFS_NO_RANDOM
	or this:
		#define RANDFS_NO_HASH
	to suppress the implementation of either the Random class or the Hash class, respectively.
	
LICENSE:
	See the end of file for license information.

*/

#include <cstdint> // For data types with explicit sizes
#include <cstring> // For std::memcpy
#include <limits> // For std::numeric_limits

/*
	Some functions in this library rely on bit copy
	between int32_t values and float values.
	Therefore, in order to avoid undefined behavior,
	both types must have the same bit size,
	as well as be trivially copyable.
	
	Although systems in which the float type has
	any bit size other than 32 bits are extremely rare
	(not to say inexistent), the C++ standard
	does not guarantee any specific bit size.
	The assert prevents those unlikely scenarios.
	
	All integer types and floating-point types are
	arithmetic types, and therefore, are trivially
	copyable. Hence, there is no need to assert that.

	It also relies on a float implementation that follows
	the IEEE Standard for Floating-Point Arithmetic (IEEE 754).
	Systems that do not use such standard are, once again, extremely rare.
*/
static_assert(sizeof(int32_t) == sizeof(float), "RandFS requires int32_t and float to have the same bit size.");
static_assert(std::numeric_limits<float>::is_iec559, "RandFS requires float types to follow the IEEE 754 standard.");

#ifndef RANDFS_NO_RANDOM

#pragma region Random declaration

class Random
{
public:
	// Initialize a Mersenne Twister PRNG with the given seed
	Random(uint64_t seed = 0ULL);
	~Random() = default;

	// Generate random 64-bit integer on the interval [0, 2^64-1]
	uint64_t UInt64();
	// Generate random 64-bit integer on the interval [-2^63, 2^63-1]
	int64_t Int64();
	// Generate random 64-bit integer on the interval [0, 2^63-1]
	int64_t PosInt64();
	// Generate random 32-bit integer on the interval [0, 2^32-1]
	uint32_t UInt32();
	// Generate random 32-bit integer on the interval [-2^31, 2^31-1]
	int32_t Int32();
	// Generate random 32-bit integer on the interval [0, 2^31-1]
	int32_t PosInt32();

	// Generate random double (64-bit floating-point value) on the closed interval [0, 1]
	double DoubleC();
	// Generate random double (64-bit floating-point value) on the half-closed interval [0, 1)
	double DoubleH();
	// Generate random double (64-bit floating-point value) on the open interval (0, 1)
	double DoubleO();
	// Generate random 32-bit float on the closed interval [0, 1]
	float FloatC();
	// Generate random 32-bit float on the half-closed interval [0, 1)
	float FloatH();
	// Generate random 32-bit float on the open interval (0, 1)
	float FloatO();

	// Generate random byte
	uint8_t UInt8();
	// Generate random boolean
	bool Bool();

	// Generate random 32-bit integer on the half-closed interval [min, max)
	int32_t IntBetween(int32_t min, int32_t max);
	// Generate random 32-bit float on the closed interval [min, max]
	float FloatBetween(float min, float max);

	// Generate random 32-bit float with normal distribution, with mean 0 and standard deviation 1
	float FloatNormal();
	// Generate random 32-bit float with normal distribution, with the given mean and standard deviation (stdDev)
	float FloatNormal(float mean, float stdDev);

	// Shuffle the given array
	template <typename T>
	void ShuffleArray(T* arr, uint32_t size)
	{
		// Fisher–Yates shuffle
		for (; size > 0U; size--)
		{
			uint32_t swapIndex = UInt32() % size;

			T temp = arr[size - 1U];
			arr[size - 1U] = arr[swapIndex];
			arr[swapIndex] = temp;
		}
	}
	// Return a reference to a random element in the given array
	template <typename T>
	T& Element(T* arr, uint32_t size)
	{
		return arr[UInt32() % size];
	}

private:
	uint64_t m_State[312]; // State array
	uint32_t m_Index; // Array index counter

	uint32_t m_Cache; // Stored value for next 32-bit call
	bool m_HasCache; // Does m_Cache contain a value that has not been used yet?
};

#pragma endregion

#ifdef RANDFS_IMPLEMENTATION

#pragma region Random implementation

Random::Random(uint64_t seed)
	: m_Cache(0U), m_HasCache(false)
{
	// From Takuji Nishimura and Makoto Matsumoto's MT19937-64 implementation (see license at the end of file)

	m_State[0] = seed;
	for (m_Index = 1U; m_Index < 312U; m_Index++)
	{
		m_State[m_Index] = 0x5851f42d4c957f2dULL * (m_State[m_Index - 1U] ^ (m_State[m_Index - 1U] >> 62)) + m_Index;
	}
}

uint64_t Random::UInt64  ()
{
	// From Takuji Nishimura and Makoto Matsumoto's MT19937-64 implementation (see license at the end of file)

	uint64_t x;

	if (m_Index > 311U) // Generate 312 words at one time
	{
		static constexpr uint64_t MS = 0xffffffff80000000ULL; // Most significant 33 bits
		static constexpr uint64_t LS = 0x7fffffffULL; // Least significant 31 bits

		uint32_t i = 0U;
		for (; i < 156U; i++)
		{
			x = (m_State[i] & MS) | (m_State[i + 1U] & LS);
			m_State[i] = m_State[i + 156U] ^ (x >> 1) ^ ((x & 1ULL) * 0xb5026f5aa96619e9ULL);
		}
		for (; i < 311U; i++)
		{
			x = (m_State[i] & MS) | (m_State[i + 1U] & LS);
			m_State[i] = m_State[i - 156U] ^ (x >> 1) ^ ((x & 1ULL) * 0xb5026f5aa96619e9ULL);
		}
		x = (m_State[311] & MS) | (m_State[0] & LS);
		m_State[311] = m_State[155] ^ (x >> 1) ^ ((x & 1ULL) * 0xb5026f5aa96619e9ULL);
		m_Index = 0U;
	}

	x = m_State[m_Index++];

	x ^= (x >> 29) & 0x5555555555555555ULL;
	x ^= (x << 17) & 0x71d67fffeda60000ULL;
	x ^= (x << 37) & 0xfff7eee000000000ULL;
	x ^= (x >> 43);

	return x;
}
int64_t  Random::Int64   () { return int64_t(UInt64()); }
int64_t  Random::PosInt64() { return int64_t(UInt64() >> 1); }

uint32_t Random::UInt32  ()
{
	// If m_Cache contains a value that can still be used, return the cached value
	if (m_HasCache)
	{
		m_HasCache = false;
		return m_Cache;
	}

	// Otherwise, generate a 64-bit random value, cache half of it and return the other half
	uint64_t x = UInt64();
	m_Cache = uint32_t(x >> 32);
	m_HasCache = true;
	return uint32_t(x);
}
int32_t  Random::Int32   () { return int32_t(UInt32()); }
int32_t  Random::PosInt32() { return int32_t(UInt32() >> 1); }

double Random::DoubleC() { return (UInt64() >> 11) * (1.0 / 9007199254740991.0); }
double Random::DoubleH() { return (UInt64() >> 11) * (1.0 / 9007199254740992.0); }
double Random::DoubleO() { return ((UInt64() >> 12) + 0.5) * (1.0 / 4503599627370496.0); }
float  Random::FloatC () { return (UInt32() >> 8) * (1.0f / 16777215.0f); }
float  Random::FloatH () { return (UInt32() >> 8) * (1.0f / 16777216.0f); }
float  Random::FloatO () { return ((UInt32() >> 9) + 0.5f) * (1.0f / 8388608.0f); }

uint8_t Random::UInt8() { return uint8_t(UInt32()); }
bool    Random::Bool () { return UInt32() & 1U; }

int32_t Random::IntBetween  (int32_t min, int32_t max) { return PosInt32() % (max - min) + min; }
float   Random::FloatBetween(  float min,   float max) { return FloatC  () * (max - min) + min; }

/*
	Fast quantile algorithm, by Quintanilha, Diego T. P. (2022) (see license at the end of this file)

	A normal random distribution is achieved by computing the expression sqrt(2) * erfinv(2x-1),
	where x is a uniformly distributed random number between 0 and 1, and erfinv(x) is the inverse error function.
	
	Approximates erfinv(x) with the formula t * ln(2) * log2((1+x)/(1-x)),
	where t is an empirically calculated constant near 0.4.
	
	The function log2(x) itself is approximated by the formula 2^(-23) * r(x) - n,
	where r(x) is the float-to-int reinterpretation (bit copy),
	and n is a constant that is cancelled out due to the division inside the logarithm.
	
	The final formula is sqrt(2) * t * ln(2) * 2^(-23) * (r(x) - r(1-x)),
	which can be simplified to C * (r(x) - r(1-x)), where C = 5.0003944e-8.
*/
float Random::FloatNormal()
{
	float u1 = FloatO();
	float u2 = 1.0f - u1;
	int32_t r1, r2;
	std::memcpy(&r1, &u1, sizeof(float));
	std::memcpy(&r2, &u2, sizeof(float));
	return 5.0003944e-8f * (r1 - r2);
}
float Random::FloatNormal(float mean, float stdDev) { return FloatNormal() * stdDev + mean; }

#pragma endregion

#endif // RANDFS_IMPLEMENTATION

#endif // RANDFS_NO_RANDOM

#ifndef RANDFS_NO_HASH

class Hash
{
public:
	Hash() = delete; // No instances of Hash should be created. A class is used instead of a namespace simply to hide the following "Pair" functions under the "private" access specifier

private:

#pragma region Pairing functions

	// Base pairing function is equivalent to f(x, y) = ((x + y) * (x + y + 1)) / 2 + y
	// Numbers grow proportionally to O(x * y)
	// An in-detail explanation can be found at https://mathworld.wolfram.com/PairingFunction.html

	// Pairing function base case for 64-bit integers
	static uint64_t Pair(uint64_t k0, uint64_t k1) { uint64_t sum = k0 + k1; return ((sum * sum + sum) >> 1) + k1; }

	// Compose pairing functions in such a way to keep results as small as possible
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2)                                                                  { return Pair(Pair(k0, k1), k2); }
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3)                                                     { return Pair(k0, k1, Pair(k2, k3)); }
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3, uint64_t k4)                                        { return Pair(k0, k1, k2, Pair(k3, k4)); }
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3, uint64_t k4, uint64_t k5)                           { return Pair(k0, k1, Pair(k2, k3), k4, k5); }
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3, uint64_t k4, uint64_t k5, uint64_t k6)              { return Pair(k0, Pair(k1, k2), k3, k4, k5, k6); }
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3, uint64_t k4, uint64_t k5, uint64_t k6, uint64_t k7) { return Pair(Pair(k0, k1), k2, k3, k4, k5, k6, k7); }

	// Pairing function general case for composing 8 or more 64-bit integers
	template <typename... OtherT>
	static uint64_t Pair(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t k3, uint64_t k4, uint64_t k5, uint64_t k6, uint64_t k7, uint64_t k8, OtherT... kn)
	{ return Pair(k0, k1, k2, k3, k4, k5, k6, Pair(k7, k8, kn...)); }

	// Pairing function base case for 32-bit integers
	static uint32_t Pair(uint32_t k0, uint32_t k1) { uint32_t sum = k0 + k1; return ((sum * sum + sum) >> 1) + k1; }

	// Compose pairing functions in such a way to keep results as small as possible
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2)                                                                  { return Pair(Pair(k0, k1), k2); }
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3)                                                     { return Pair(k0, k1, Pair(k2, k3)); }
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4)                                        { return Pair(k0, k1, k2, Pair(k3, k4)); }
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4, uint32_t k5)                           { return Pair(k0, k1, Pair(k2, k3), k4, k5); }
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4, uint32_t k5, uint32_t k6)              { return Pair(k0, Pair(k1, k2), k3, k4, k5, k6); }
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4, uint32_t k5, uint32_t k6, uint32_t k7) { return Pair(Pair(k0, k1), k2, k3, k4, k5, k6, k7); }

	// Pairing function general case for composing 8 or more 32-bit integers
	template <typename... OtherT>
	static uint32_t Pair(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4, uint32_t k5, uint32_t k6, uint32_t k7, uint32_t k8, OtherT... kn)
	{ return Pair(k0, k1, k2, k3, k4, k5, k6, Pair(k7, k8, kn...)); }

#pragma endregion

public:

#pragma region Hash declaration (no seed)

	// Hash a 64-bit integer to another 64-bit integer on the interval [0, 2^64-1]
	static uint64_t UInt64(uint64_t n);
	// Hash a 64-bit integer to another 64-bit integer on the interval [-2^63, 2^63-1]
	static int64_t Int64(uint64_t n);
	// Hash a 64-bit integer to another 64-bit integer on the interval [0, 2^63-1]
	static int64_t PosInt64(uint64_t n);
	// Hash a 32-bit integer to another 32-bit integer on the interval [0, 2^32-1]
	static uint32_t UInt32(uint32_t n);
	// Hash a 32-bit integer to another 32-bit integer on the interval [-2^31, 2^31-1]
	static int32_t Int32(uint32_t n);
	// Hash a 32-bit integer to another 32-bit integer on the interval [0, 2^31-1]
	static int32_t PosInt32(uint32_t n);

	// Hash a 64-bit integer to a double (64-bit floating-point value) on the closed interval [0, 1]
	static double DoubleC(uint64_t n);
	// Hash a 64-bit integer to a double (64-bit floating-point value) on the half-closed interval [0, 1)
	static double DoubleH(uint64_t n);
	// Hash a 64-bit integer to a double (64-bit floating-point value) on the open interval (0, 1)
	static double DoubleO(uint64_t n);
	// Hash a 32-bit integer to a 32-bit float on the closed interval [0, 1]
	static float FloatC(uint32_t n);
	// Hash a 32-bit integer to a 32-bit float on the half-closed interval [0, 1)
	static float FloatH(uint32_t n);
	// Hash a 32-bit integer to a 32-bit float on the open interval (0, 1)
	static float FloatO(uint32_t n);

	// Hash a 32-bit integer to an 8-bit value
	static uint8_t UInt8(uint32_t n);
	// Hash a 32-bit integer to a boolean
	static bool Bool(uint32_t n);

	// Hash a 32-bit integer to another 32-bit integer on the half-closed interval [min, max)
	static int32_t IntBetween(uint32_t n, int32_t min, int32_t max);
	// Hash a 32-bit integer to a 32-bit float on the closed interval [min, max]
	static float FloatBetween(uint32_t n, float min, float max);

	// Hash a 32-bit integer to a 32-bit float with normal distribution, with mean 0 and standard deviation 1
	static float FloatNormal(uint32_t n);
	// Hash a 32-bit integer to a 32-bit float with normal distribution, with the given mean and standard deviation (stdDev)
	static float FloatNormal(uint32_t n, float mean, float stdDev);

	// Hash an arbitrary type to a 64-bit integer on [0, 2^64-1]-interval
	template <typename T>
	static uint64_t Type64(T n)
	{
		// Copy contents of n into a uint64_t array
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint64_t) - 1U) / sizeof(uint64_t); // Same as sizeof(T) / sizeof(uint64_t) but rounded up
		uint64_t contents[contentSize] = { 0 };
		std::memcpy(contents, &n, sizeof(T));

		uint64_t x = contents[0];

		for (uint32_t i = 1U; i < contentSize; i++)
		{
			x = Pair(x, contents[i]);
		}

		return UInt64(x);
	}
	// Hash an arbitrary type to a 32-bit integer on [0, 2^32-1]-interval
	template <typename T>
	static uint32_t Type32(T n)
	{
		// Copy contents of n into a uint32_t array
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint32_t) - 1U) / sizeof(uint32_t); // Same as sizeof(T) / sizeof(uint32_t) but rounded up
		uint32_t contents[contentSize] = { 0 };
		std::memcpy(contents, &n, sizeof(T));

		uint32_t x = contents[0];

		for (uint32_t i = 1U; i < contentSize; i++)
		{
			x = Pair(x, contents[i]);
		}

		return UInt32(x);
	}
	
	// Hash the given arbitrary type array to a single 64-bit integer on [0, 2^64-1]-interval
	template <typename T>
	static uint64_t Array64(T* arr, uint32_t size)
	{
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint64_t) - 1U) / sizeof(uint64_t); // Same as sizeof(T) / sizeof(uint64_t) but rounded up
		uint64_t contents[contentSize] = { 0 };

		uint64_t x = 0ULL;

		// Copy contents of each element of arr into a uint64_t array and pair them into x
		for (uint32_t i = 0U; i < size; i++)
		{
			std::memcpy(contents, &arr[i], sizeof(T));

			for (uint32_t j = 0U; j < contentSize; j++)
			{
				x = Pair(x, contents[j]);
			}
		}

		return UInt64(x);
	}
	// Hash the given arbitrary type array to a single 32-bit integer on [0, 2^32-1]-interval
	template <typename T>
	static uint32_t Array32(T* arr, uint32_t size)
	{
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint32_t) - 1U) / sizeof(uint32_t); // Same as sizeof(T) / sizeof(uint32_t) but rounded up
		uint32_t contents[contentSize] = { 0 };

		uint32_t x = 0U;

		// Copy contents of each element of arr into a uint32_t array and pair them into x
		for (uint32_t i = 0U; i < size; i++)
		{
			std::memcpy(contents, &arr[i], sizeof(T));

			for (uint32_t j = 0U; j < contentSize; j++)
			{
				x = Pair(x, contents[j]);
			}
		}

		return UInt32(x);
	}

	// Hash the given string of characters to a 64-bit integer on [0, 2^64-1]-interval
	static uint64_t String64(const char* string);
	// Hash the given string of characters to a 32-bit integer on [0, 2^32-1]-interval
	static uint32_t String32(const char* string);

#pragma endregion

#pragma region Hash declaration (with seed)

	// Hash a 64-bit integer and a 64-bit seed to a 64-bit integer on the interval [0, 2^64-1]
	static uint64_t UInt64(uint64_t n, uint64_t seed);
	// Hash a 64-bit integer and a 64-bit seed to a 64-bit integer on the interval [-2^63, 2^63-1]
	static int64_t Int64(uint64_t n, uint64_t seed);
	// Hash a 64-bit integer and a 64-bit seed to a 64-bit integer on the interval [0, 2^63-1]
	static int64_t PosInt64(uint64_t n, uint64_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit integer on the interval [0, 2^32-1]
	static uint32_t UInt32(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit integer on the interval [-2^31, 2^31-1]
	static int32_t Int32(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit integer on the interval [0, 2^31-1]
	static int32_t PosInt32(uint32_t n, uint32_t seed);

	// Hash a 64-bit integer and a 64-bit seed to a double (64-bit floating-point value) on the closed interval [0, 1]
	static double DoubleC(uint64_t n, uint64_t seed);
	// Hash a 64-bit integer and a 64-bit seed to a double (64-bit floating-point value) on the half-closed interval [0, 1)
	static double DoubleH(uint64_t n, uint64_t seed);
	// Hash a 64-bit integer and a 64-bit seed to a double (64-bit floating-point value) on the open interval (0, 1)
	static double DoubleO(uint64_t n, uint64_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float on the closed interval [0, 1]
	static float FloatC(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float on the half-closed interval [0, 1)
	static float FloatH(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float on the open interval (0, 1)
	static float FloatO(uint32_t n, uint32_t seed);

	// Hash a 32-bit integer and a 32-bit seed to an 8-bit value
	static uint8_t UInt8(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a boolean
	static bool Bool(uint32_t n, uint32_t seed);

	// Hash a 32-bit integer and a 32-bit seed to a 32-bit integer on the half-closed interval [min, max)
	static int32_t IntBetween(uint32_t n, uint32_t seed, int32_t min, int32_t max);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float on the closed interval [min, max]
	static float FloatBetween(uint32_t n, uint32_t seed, float min, float max);
	
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float with normal distribution, with mean 0 and standard deviation 1
	static float FloatNormal(uint32_t n, uint32_t seed);
	// Hash a 32-bit integer and a 32-bit seed to a 32-bit float with normal distribution, with the given mean and standard deviation (stdDev)
	static float FloatNormal(uint32_t n, uint32_t seed, float mean, float stdDev);

	// Hash multiple 64-bit integers to a single 64-bit integer on [0, 2^64-1]-interval
	template <typename... Args>
	static uint64_t UInt64(uint64_t n1, uint64_t n2, Args... n) { return UInt64(Pair(n1, n2, n...)); }
	// Hash multiple 32-bit integers to a single 32-bit integer on [0, 2^32-1]-interval
	template <typename... Args>
	static uint32_t UInt32(uint32_t n1, uint32_t n2, Args... n) { return UInt32(Pair(n1, n2, n...)); }

	// Hash an arbitrary type and a 64-bit seed to a 64-bit integer on [0, 2^64-1]-interval
	template <typename T>
	static uint64_t Type64(T n, uint64_t seed)
	{
		// Copy contents of n into a uint64_t array
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint64_t) - 1U) / sizeof(uint64_t); // Same as sizeof(T) / sizeof(uint64_t) but rounded up
		uint64_t contents[contentSize] = { 0 };
		std::memcpy(contents, &n, sizeof(T));

		uint64_t x = contents[0];

		for (uint32_t i = 1U; i < contentSize; i++)
		{
			x = Pair(x, contents[i]);
		}

		return UInt64(x, seed);
	}
	// Hash an arbitrary type and a 32-bit seed to a 32-bit integer on [0, 2^32-1]-interval
	template <typename T>
	static uint32_t Type32(T n, uint32_t seed)
	{
		// Copy contents of n into a uint32_t array
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint32_t) - 1U) / sizeof(uint32_t); // Same as sizeof(T) / sizeof(uint32_t) but rounded up
		uint32_t contents[contentSize] = { 0 };
		std::memcpy(contents, &n, sizeof(T));

		uint32_t x = contents[0];

		for (uint32_t i = 1U; i < contentSize; i++)
		{
			x = Pair(x, contents[i]);
		}

		return UInt32(x, seed);
	}
	
	// Hash the given arbitrary type array and a 64-bit seed to a single 64-bit integer on [0, 2^64-1]-interval
	template <typename T>
	static uint64_t Array64(T* arr, uint32_t size, uint64_t seed)
	{
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint64_t) - 1U) / sizeof(uint64_t); // Same as sizeof(T) / sizeof(uint64_t) but rounded up
		uint64_t contents[contentSize] = { 0 };

		uint64_t x = 0ULL;

		// Copy contents of each element of arr into a uint64_t array and pair them into x
		for (uint32_t i = 0U; i < size; i++)
		{
			std::memcpy(contents, &arr[i], sizeof(T));

			for (uint32_t j = 0U; j < contentSize; j++)
			{
				x = Pair(x, contents[j]);
			}
		}

		return UInt64(x, seed);
	}
	// Hash the given arbitrary type array and a 32-bit seed to a single 32-bit integer on [0, 2^32-1]-interval
	template <typename T>
	static uint32_t Array32(T* arr, uint32_t size, uint32_t seed)
	{
		static constexpr uint32_t contentSize = (sizeof(T) + sizeof(uint32_t) - 1U) / sizeof(uint32_t); // Same as sizeof(T) / sizeof(uint32_t) but rounded up
		uint32_t contents[contentSize] = { 0 };

		uint32_t x = 0U;

		// Copy contents of each element of arr into a uint32_t array and pair them into x
		for (uint32_t i = 0U; i < size; i++)
		{
			std::memcpy(contents, &arr[i], sizeof(T));

			for (uint32_t j = 0U; j < contentSize; j++)
			{
				x = Pair(x, contents[j]);
			}
		}

		return UInt32(x, seed);
	}

	// Hash the given string of characters and a 64-bit seed to a 64-bit integer on [0, 2^64-1]-interval
	static uint64_t String64(const char* string, uint64_t seed);
	// Hash the given string of characters and a 32-bit seed to a 32-bit integer on [0, 2^32-1]-interval
	static uint32_t String32(const char* string, uint32_t seed);

	// Shuffle the given array from a 64-bit hash seed
	template <typename T>
	static void ShuffleArray64(T* arr, uint64_t size, uint64_t seed)
	{
		// Fisher–Yates shuffle
		for (; size > 0ULL; size--)
		{
			uint64_t swapIndex = UInt64(size, seed) % size;

			T temp = arr[size - 1U];
			arr[size - 1U] = arr[swapIndex];
			arr[swapIndex] = temp;
		}
	}
	// Shuffle the given array from a 32-bit hash seed
	template <typename T>
	static void ShuffleArray32(T* arr, uint32_t size, uint32_t seed)
	{
		// Fisher–Yates shuffle
		for (; size > 0U; size--)
		{
			uint32_t swapIndex = UInt32(size, seed) % size;

			T temp = arr[size - 1U];
			arr[size - 1U] = arr[swapIndex];
			arr[swapIndex] = temp;
		}
	}
	// Return a reference to an element in the given array selected from a 64-bit hash seed
	template <typename T>
	static T& Element64(T* arr, uint64_t size, uint64_t seed)
	{
		return arr[UInt64(seed) % size];
	}
	// Return a reference to an element in the given array selected from a 32-bit hash seed
	template <typename T>
	static T& Element32(T* arr, uint32_t size, uint32_t seed)
	{
		return arr[UInt32(seed) % size];
	}

#pragma endregion

};

#ifdef RANDFS_IMPLEMENTATION

#pragma region Hash implementation (no seed)

uint64_t Hash::UInt64  (uint64_t n)
{
	// Google's implementation of Murmur3 hash (see license at the end of file)
	uint64_t x = n;
	x *= 0x9ddfea08eb382d69ULL;
	x ^= (x >> 47);
	x ^= n;
	x *= 0x9ddfea08eb382d69ULL;
	x ^= (x >> 47);
	x *= 0x9ddfea08eb382d69ULL;
	return x;
}
int64_t  Hash::Int64   (uint64_t n) { return int64_t(UInt64(n)); }
int64_t  Hash::PosInt64(uint64_t n) { return int64_t(UInt64(n) >> 1); }

uint32_t Hash::UInt32  (uint32_t n)
{
	// Google's implementation of Murmur3 hash (see license at the end of file)
	n ^= (n >> 16);
	n *= 0x85ebca6bU;
	n ^= (n >> 13);
	n *= 0xc2b2ae35U;
	n ^= (n >> 16);
	return n;
}
int32_t  Hash::Int32   (uint32_t n) { return int32_t(UInt32(n)); }
int32_t  Hash::PosInt32(uint32_t n) { return int32_t(UInt32(n) >> 1); }

double Hash::DoubleC(uint64_t n) { return (UInt64(n) >> 11) * (1.0 / 9007199254740991.0); }
double Hash::DoubleH(uint64_t n) { return (UInt64(n) >> 11) * (1.0 / 9007199254740992.0); }
double Hash::DoubleO(uint64_t n) { return ((UInt64(n) >> 12) + 0.5) * (1.0 / 4503599627370496.0); }
float  Hash::FloatC (uint32_t n) { return (UInt32(n) >> 8) * (1.0f / 16777215.0f); }
float  Hash::FloatH (uint32_t n) { return (UInt32(n) >> 8) * (1.0f / 16777216.0f); }
float  Hash::FloatO (uint32_t n) { return ((UInt32(n) >> 9) + 0.5f) * (1.0f / 8388608.0f); }

uint8_t Hash::UInt8(uint32_t n) { return uint8_t(UInt32(n)); }
bool    Hash::Bool (uint32_t n) { return UInt32(n) & 1U; }

int32_t Hash::IntBetween  (uint32_t n, int32_t min, int32_t max) { return PosInt32(n) % (max - min) + min; }
float   Hash::FloatBetween(uint32_t n,   float min,   float max) { return FloatC  (n) * (max - min) + min; }

// See comment above Random::FloatNormal implementation for an explanation of the algorithm
float Hash::FloatNormal(uint32_t n)
{
	float u1 = FloatO(n);
	float u2 = 1.0f - u1;
	int32_t r1, r2;
	std::memcpy(&r1, &u1, sizeof(float));
	std::memcpy(&r2, &u2, sizeof(float));
	return 5.0003944e-8f * (r1 - r2);
}
float Hash::FloatNormal(uint32_t n, float mean, float stdDev) { return FloatNormal(n) * stdDev + mean; }

// Hash the given array of 64-bit integers to a single 64-bit integer on [0, 2^64-1]-interval
template <>
uint64_t Hash::Array64(uint64_t* arr, uint32_t size)
{
	uint64_t x = arr[0];

	for (uint32_t i = 1U; i < size; i++)
	{
		x = Pair(x, arr[i]);
	}

	return UInt64(x);
}
// Hash the given array of 32-bit integers to a single 32-bit integer on [0, 2^32-1]-interval
template <>
uint32_t Hash::Array32(uint32_t* arr, uint32_t size)
{
	uint32_t x = arr[0];

	for (uint32_t i = 1U; i < size; i++)
	{
		x = Pair(x, arr[i]);
	}

	return UInt32(x);
}

uint64_t Hash::String64(const char* string)
{
	uint64_t x = 0ULL;
	
	for (uint32_t i = 0U; string[i]; i++)
	{
		x = Pair(x, uint64_t(string[i]));
	}
	
	return UInt64(x);
}
uint32_t Hash::String32(const char* string)
{
	uint32_t x = 0U;

	for (uint32_t i = 0U; string[i]; i++)
	{
		x = Pair(x, uint32_t(string[i]));
	}

	return UInt32(x);
}

#pragma endregion

#pragma region Hash implementation (with seed)

uint64_t Hash::UInt64  (uint64_t n, uint64_t seed) { return UInt64(Pair(n, seed)); }
int64_t  Hash::Int64   (uint64_t n, uint64_t seed) { return int64_t(UInt64(n, seed)); }
int64_t  Hash::PosInt64(uint64_t n, uint64_t seed) { return int64_t(UInt64(n, seed) >> 1); }

uint32_t Hash::UInt32  (uint32_t n, uint32_t seed) { return UInt32(Pair(n, seed)); }
int32_t  Hash::Int32   (uint32_t n, uint32_t seed) { return int32_t(UInt32(n, seed)); }
int32_t  Hash::PosInt32(uint32_t n, uint32_t seed) { return int32_t(UInt32(n, seed) >> 1); }

double Hash::DoubleC(uint64_t n, uint64_t seed) { return (UInt64(n, seed) >> 11) * (1.0 / 9007199254740991.0); }
double Hash::DoubleH(uint64_t n, uint64_t seed) { return (UInt64(n, seed) >> 11) * (1.0 / 9007199254740992.0); }
double Hash::DoubleO(uint64_t n, uint64_t seed) { return ((UInt64(n, seed) >> 12) + 0.5) * (1.0 / 4503599627370496.0); }
float  Hash::FloatC (uint32_t n, uint32_t seed) { return (UInt32(n, seed) >> 8) * (1.0f / 16777215.0f); }
float  Hash::FloatH (uint32_t n, uint32_t seed) { return (UInt32(n, seed) >> 8) * (1.0f / 16777216.0f); }
float  Hash::FloatO (uint32_t n, uint32_t seed) { return ((UInt32(n, seed) >> 9) + 0.5f) * (1.0f / 8388608.0f); }

uint8_t Hash::UInt8(uint32_t n, uint32_t seed) { return uint8_t(UInt32(n, seed)); }
bool    Hash::Bool (uint32_t n, uint32_t seed) { return UInt32(n, seed) & 1U; }

int32_t Hash::IntBetween  (uint32_t n, uint32_t seed, int32_t min, int32_t max) { return PosInt32(n, seed) % (max - min) + min; }
float   Hash::FloatBetween(uint32_t n, uint32_t seed,   float min,   float max) { return FloatC  (n, seed) * (max - min) + min; }

// See comment above Random::FloatNormal implementation for an explanation of the algorithm
float Hash::FloatNormal(uint32_t n, uint32_t seed)
{
	float u1 = FloatO(n, seed);
	float u2 = 1.0f - u1;
	int32_t r1, r2;
	std::memcpy(&r1, &u1, sizeof(float));
	std::memcpy(&r2, &u2, sizeof(float));
	return 5.0003944e-8f * (r1 - r2);
}
float Hash::FloatNormal(uint32_t n, uint32_t seed, float mean, float stdDev) { return FloatNormal(n, seed) * stdDev + mean; }

// Hash the given array of 64-bit integers and a 64-bit seed to a single 64-bit integer on [0, 2^64-1]-interval
template <>
uint64_t Hash::Array64(uint64_t* arr, uint32_t size, uint64_t seed)
{
	uint64_t x = arr[0];

	for (uint32_t i = 1U; i < size; i++)
	{
		x = Pair(x, arr[i]);
	}

	return UInt64(x, seed);
}
// Hash the given array of 32-bit integers and a 32-bit seed to a single 32-bit integer on [0, 2^32-1]-interval
template <>
uint32_t Hash::Array32(uint32_t* arr, uint32_t size, uint32_t seed)
{
	uint32_t x = arr[0];

	for (uint32_t i = 1U; i < size; i++)
	{
		x = Pair(x, arr[i]);
	}

	return UInt32(x, seed);
}

uint64_t Hash::String64(const char* string, uint64_t seed)
{
	uint64_t x = 0ULL;
	
	for (uint32_t i = 0U; string[i]; i++)
	{
		x = Pair(x, uint64_t(string[i]));
	}
	
	return UInt64(x, seed);
}
uint32_t Hash::String32(const char* string, uint32_t seed)
{
	uint32_t x = 0U;

	for (uint32_t i = 0U; string[i]; i++)
	{
		x = Pair(x, uint32_t(string[i]));
	}

	return UInt32(x, seed);
}

#pragma endregion

#endif // RANDFS_IMPLEMENTATION

#endif // RANDFS_NO_HASH

/*
MAIN LICENSE:
	The RandFS.h file as a whole, specially the fast quantile algorithm implemented in the functions
		Random::FloatNormal()
		Hash::FloatNormal(uint32_t n)
		Hash::FloatNormal(uint32_t n, uint32_t seed)
	are under MIT license, which can be found at https://github.com/diegoquintanilha/RandFS/blob/main/LICENSE (COMING SOON!)

VENDOR LICENSES:
	Both the 64-bit and 32-bit core hash functions
		Hash::UInt64(uint64_t n)
		Hash::UInt32(uint32_t n)
	are an implementation of the Murmur3 hash by Google.
	The license can be found at https://github.com/google/cityhash/?tab=MIT-1-ov-file

	The core PRNG implemented in the functions
		Random::Random(uint64_t seed)
		Random::UInt64()
	is an implementation of Mersenne Twister by Takuji Nishimura and Makoto Matsumoto.
	The license below was copied from the source:
	
	Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	
	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	
	3. The names of its contributors may not be used to endorse or promote
	   products derived from this software without specific prior written
	   permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
	References:
	T. Nishimura, "Tables of 64-bit Mersenne Twisters"
	 ACM Transactions on Modeling and
	 Computer Simulation 10. (2000) 348--357.
	M. Matsumoto and T. Nishimura,
	 "Mersenne Twister: a 623-dimensionally equidistributed
	 uniform pseudorandom number generator"
	 ACM Transactions on Modeling and
	 Computer Simulation 8. (Jan. 1998) 3--30.
	
	Any feedback is very welcome.
	http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
	email: m-mat@math.sci.hiroshima-u.ac.jp
*/

