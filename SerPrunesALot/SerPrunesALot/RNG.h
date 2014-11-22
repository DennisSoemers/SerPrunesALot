#pragma once

#include <inttypes.h>
#include <random>

/**
 * Utility class for random number generation.
 *
 * Capable of generating 32-bits and 64-bits numbers. Generators seeded based on time of program launch.
 */
namespace RNG
{
	class Generator_32
	{
	public:
		Generator_32();

		inline uint32_t randomUint_32()
		{
			std::uniform_int_distribution<uint32_t> distribution;
			return distribution(gen);
		}

	private:
		// generator for 32-bits numbers
		std::mt19937 gen;
	};

	class Generator_64
	{
	public:
		Generator_64();

		inline uint64_t randomUint_64()
		{
			std::uniform_int_distribution<uint64_t> distribution;
			return distribution(gen);
		}

	private:
		// generator for 64-bits numbers
		std::mt19937_64 gen;
	};

	// Generates a random 32-bits unsigned int
	uint32_t randomUint_32();
	// Generates a random 64-bits unsigned int
	uint64_t randomUint_64();
}