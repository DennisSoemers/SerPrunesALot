#pragma once

#include <inttypes.h>

#include "Logger.h"

/**
 * Utility functions for bitboards.
 * The functions can easily be inlined by the compiler, so they can safely be used for improved code readability without a loss of performance.
 *
 * All functions assume 64-bits unsigned integers!
 *
 * Functions that take a ''bitIndex'' argument have undefined behavior for arguments outside [0, 63]!
 */
namespace Bitboards
{
	/** Constant with all bits set to 0 */
	const uint64_t ALL_ZERO = 0ULL;
	/** Constant with all bits set to 1 */
	const uint64_t ALL_ONES = -1;

	/** A constant representing 1's on the top row (labelled ''8'' in GUI) */
	const uint64_t ROW_8 = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;	// first 8 bits set

	// To compute constants for the other rows, simply keep shifting the sequences down (= 8 left shifts)

	/** A constant representing 1's on the second row (labelled ''7'' in GUI) */
	const uint64_t ROW_7 = ROW_8 << 8;
	/** A constant representing 1's on the third row (labelled ''6'' in GUI) */
	const uint64_t ROW_6 = ROW_7 << 8;
	/** A constant representing 1's on the fourth row (labelled ''5'' in GUI) */
	const uint64_t ROW_5 = ROW_6 << 8;
	/** A constant representing 1's on the fifth row (labelled ''4'' in GUI) */
	const uint64_t ROW_4 = ROW_5 << 8;
	/** A constant representing 1's on the sixth row (labelled ''3'' in GUI) */
	const uint64_t ROW_3 = ROW_4 << 8;
	/** A constant representing 1's on the seventh row (labelled ''2'' in GUI) */
	const uint64_t ROW_2 = ROW_3 << 8;
	/** A constant representing 1's on the eighth row (labelled ''1'' in GUI) */
	const uint64_t ROW_1 = ROW_2 << 8;

	/** If a black piece is in this zone, and black is to move, he can win instantly */
	const uint64_t DANGER_ZONE_BOTTOM = ROW_2 & ROW_3;
	/** If a white piece is in this zone, and white is to move, he can win instantly */
	const uint64_t DANGER_ZONE_TOP = ROW_6 & ROW_7;

	/**
	 * Returns the index of the first bit that is set to 1 in the given bitset.
	 *
	 * Implementation uses a De Bruijn Sequence (see link below for references)
	 * Implementation adapted from: https://chessprogramming.wikispaces.com/BitScan#DeBruijnMultiplation
	 */
	inline int bitScanForward(uint64_t bitset)
	{
		static const int bitScanForwardIndices[64] = {
			0, 1, 48, 2, 57, 49, 28, 3,
			61, 58, 50, 42, 38, 29, 17, 4,
			62, 55, 59, 36, 53, 51, 43, 22,
			45, 39, 33, 30, 24, 18, 12, 5,
			63, 47, 56, 27, 60, 41, 37, 16,
			54, 35, 52, 21, 44, 32, 23, 11,
			46, 26, 40, 15, 34, 20, 31, 10,
			25, 14, 19, 9, 13, 8, 7, 6
		};

		static const uint64_t deBruijn64 = 0x03f79d71b4cb0a89ULL;

		return bitScanForwardIndices[((bitset & -bitset) * deBruijn64) >> 58];
	}

	/**
	 * Returns the index of the last bit that is set to 1 in the given bitset
	 *
	 * Implementation uses a De Bruijn Sequence (see link below for references)
	 * Implementation adapted from: https://chessprogramming.wikispaces.com/BitScan#Bitscan%20reverse-De%20Bruijn%20Multiplication
	 */
	inline int bitScanReverse(uint64_t bitset)
	{
		static const int bitScanReverseIndices[64] = {
			0, 47, 1, 56, 48, 27, 2, 60,
			57, 49, 41, 37, 28, 16, 3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11, 4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30, 9, 24,
			13, 18, 8, 12, 7, 6, 5, 63
		};

		static const uint64_t deBruijn64 = 0x03f79d71b4cb0a89ULL;

		bitset |= bitset >> 1;
		bitset |= bitset >> 2;
		bitset |= bitset >> 4;
		bitset |= bitset >> 8;
		bitset |= bitset >> 16;
		bitset |= bitset >> 32;

		return bitScanReverseIndices[(bitset * deBruijn64) >> 58];
	}

	/** 
	 * Initializes an array of 64 bitsets where bitset[i] is the set with only bit i set. 
	 * Array is allocated on heap, memory is not de-allocated!
	 */
	inline uint64_t* initSingleBits()
	{
		uint64_t* singleBits = new uint64_t[64];

		for(int i = 0; i < 64; ++i)
		{
			singleBits[i] = 1ULL << i;
		}

		LOG_MESSAGE("Initializing Single Bits!")

		return singleBits;
	}

	/** Returns a 64 bits unsigned int with only the given bit set to 1, and all others set to 0 */
	inline uint64_t singleBit(int bitIndex)
	{
		static uint64_t* singleBits = initSingleBits();

		return singleBits[bitIndex];
	}

	/** Returns true iff the given bitIndex is set in the given bitset */
	inline bool isBitSet(uint64_t bitset, int bitIndex)
	{
		return (bitset & singleBit(bitIndex));
	}

	/** Sets the bit at the given index in the given bitset and returns the result */
	inline uint64_t setBit(uint64_t bitset, int bitIndex)
	{
		static uint64_t* singleBits = initSingleBits();

		return (bitset | singleBit(bitIndex));
	}
}