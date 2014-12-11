#pragma once

#include <inttypes.h>

#include "Logger.h"
#include "Move.h"

// if defined, the legality of the best move stored will be checked as an additional verification of correctness of the stored data
//#define VERIFY_MOVE_LEGALITY

/**
 * A 64-bit hash value, with 22 bits as primary hash code and 42 bits as secondary hash code
 *
 * The primary and secondary hash codes can be retrieved individually from the hashCodes field,
 * and the entire hash value (consisting of both codes) can be retrieved as the value field.
 */
union HashValue
{
	// Allows individual access to primary and secondary codes
	struct HashCodes
	{
		uint64_t primary : 22;
		uint64_t secondary : 42;
	} hashCodes;

	// the entire 64-bits value
	uint64_t value;

	HashValue(uint64_t value) : value(value){}

	/** Overloaded == operator. Considers two HashValues to be equal if the full value is equal */
	inline bool operator==(const HashValue& other) const
	{
		return (value == other.value);
	}
};

// enum for the different possible value types
namespace EValue
{
	enum Type : uint8_t
	{
		REAL,
		LOWER_BOUND,
		UPPER_BOUND,

		INVALID_TYPE
	};
}

/**
 * Contains the data for a single node of the game tree to be stored in
 * the Transposition Table
 */
struct TableData
{
public:
	TableData::TableData() : bestMove(INVALID_MOVE), hashValue(0), value(0), valueType(EValue::Type::INVALID_TYPE), depth(0)
	{}

	Move bestMove;
	HashValue hashValue;
	int value;
	uint8_t depth;
	EValue::Type valueType;

	/** 
	 * Returns true iff the data is valid. 
	 * Data is considered to be valid iff the valueType does not equal EValue::Type::INVALID_TYPE
	 */
	bool isValid() const;

private:
	// don't want accidental copying of the Table Data
	TableData(const TableData&);
	TableData& operator=(const TableData&);
};

/**
 * An entry in the Transposition Table.
 *
 * Every entry has room to store the required data for two pieces of TableData.
 * The replacement scheme Two-Deep is used, which always preserves the
 * node with the deepest search depth and the newest node.
 */
struct TableEntry
{
	TableData data1;
	TableData data2;
};

/**
 * A transposition table
 *
 * Uses 64-bit hash values, with the first 21 bits as primary hash code
 * and the remaining 43 bits as secondary hash code.
 *
 * The table has space for 2^21 entries.
 */
class TranspositionTable
{
public:
	TranspositionTable();
	~TranspositionTable();

	/** Clears the transposition table */
	void clear();

	/** 
	 * Returns the number of entries that was used. 
	 * Only returns a meaningful number if GATHER_STATISTICS is defined 
	 */
	int getNumEntriesUsed() const;

	/** 
	 * Returns the number of entries that were overwritten by data for a new game state. 
	 * Only returns a meaningful number if GATHER_STATISTICS is defined
	 */
	int getNumReplacementsRequired() const;

	/** 
	 * Prefetches the entry corresponding to the given zobrist value and loads it into L2 cache 
	 * See Section 6.3.2 of: http://www.akkadia.org/drepper/cpumemory.pdf
	 *
	 * Profiling indicated a consistent 6% performance increase using this function
	 */
	void prefetch(uint64_t zobrist) const;

	/** 
	 * Retrieves the data corresponding to the given zobrist hash value in the Transposition Table
	 * Data is returned by const reference, and cannot be copied!
	 *
	 * Will return data that returns false for isValid() if no data was found with the correct key
	 */
	const TableData& retrieve(uint64_t zobrist) const;

	/**
	 * Checkes whether or not the given new data should be stored, and if so, stores it in the table.
	 */
	void storeData(Move bestMove, uint64_t zobrist, int value, EValue::Type valueType, int depth);

private:
	TableEntry* table;

	int numEntriesUsed;
	int numReplacementsRequired;

	// don't want accidental copying of the Transposition Table
	TranspositionTable(const TranspositionTable&);
	TranspositionTable& operator=(const TranspositionTable&);
};

// invalid table data
static TableData INVALID_TABLE_DATA;