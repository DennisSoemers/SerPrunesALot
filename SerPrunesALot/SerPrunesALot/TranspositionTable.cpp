#include "RNG.h"
#include "TranspositionTable.h"

bool TableData::isValid() const
{
	return valueType != EValue::Type::INVALID_TYPE;
}

TranspositionTable::TranspositionTable()
{
	table = new TableEntry[TRANSPOSITION_TABLE_NUM_ENTRIES]();
}

TranspositionTable::~TranspositionTable()
{
	delete[] table;
}

void TranspositionTable::clear()
{
	delete[] table;
	table = new TableEntry[TRANSPOSITION_TABLE_NUM_ENTRIES]();
}

const TableData& TranspositionTable::retrieve(uint64_t zobrist) const
{
	HashValue zobristHash = HashValue(zobrist);
	uint64_t index = zobristHash.hashCodes.primary;

#ifdef ALLOW_LOGGING
	// only check for errors if we can actually log them
	if (index >= TRANSPOSITION_TABLE_NUM_ENTRIES)
	{
		LOG_ERROR("TranspositionTable::retrieve(): index larger than 2^20!")
		return INVALID_TABLE_DATA;
	}
#endif

	TableEntry* entry = table + index;
	if (entry->data1.hashValue == zobristHash)			// first element is the correct data
	{
		return entry->data1;
	}
	else if (entry->data2.hashValue == zobristHash)		// second element is the correct data
	{
		return entry->data2;
	}
	else
	{
		return INVALID_TABLE_DATA;
	}
}

void TranspositionTable::storeData(Move bestMove, uint64_t zobrist, int value, EValue::Type valueType, int depth)
{
	HashValue zobristHash = HashValue(zobrist);
	uint64_t index = zobristHash.hashCodes.primary;

#ifdef ALLOW_LOGGING
	// only check for errors if we can actually log them
	if (index >= TRANSPOSITION_TABLE_NUM_ENTRIES)
	{
		LOG_ERROR("TranspositionTable::storeData(): index larger than 2^20!")
		return;
	}
#endif

	TableEntry* entry = table + index;

	// fetch references to the two chunks of data we have in this table entry
	TableData& data1 = entry->data1;
	TableData& data2 = entry->data2;

	// first check if one of them has an identical full hash value, and if so, prefer data with largest search depth
	if (data1.hashValue == zobristHash)
	{
		if (depth > data1.depth)	// only change data if we've searched to a deeper depth now
		{
			data1.bestMove = bestMove;
			data1.depth = depth;
			data1.hashValue = zobristHash;
			data1.value = value;
			data1.valueType = valueType;
		}

		return;
	}
	else if (data2.hashValue == zobristHash)
	{
		if (depth > data2.depth)	// only change data if we've searched to a deeper depth now
		{
			data2.bestMove = bestMove;
			data2.depth = depth;
			data2.hashValue = zobristHash;
			data2.value = value;
			data2.valueType = valueType;
		}

		return;
	}

	// now check if one of the slots in the entry is still empty, and if so, use that
	if (!data1.isValid())
	{
		data1.bestMove = bestMove;
		data1.depth = depth;
		data1.hashValue = zobristHash;
		data1.value = value;
		data1.valueType = valueType;

		return;
	}
	else if (!data2.isValid())
	{
		data2.bestMove = bestMove;
		data2.depth = depth;
		data2.hashValue = zobristHash;
		data2.value = value;
		data2.valueType = valueType;

		return;
	}

	// both slots already filled, so replace whichever has the lowest depth
	if (data1.depth < data2.depth)				// slot 1 searched less deep than slot 2, so replace that
	{
		data1.bestMove = bestMove;
		data1.depth = depth;
		data1.hashValue = zobristHash;
		data1.value = value;
		data1.valueType = valueType;
	}
	else if (data2.depth < data1.depth)			// slot 2 searched less deep than slot 1, so replace that
	{
		data2.bestMove = bestMove;
		data2.depth = depth;
		data2.hashValue = zobristHash;
		data2.value = value;
		data2.valueType = valueType;
	}
	else			// both existing slots have equal search depth, move data from 1 to 2 and then replace 1
	{
		data2.bestMove = data1.bestMove;
		data2.depth = data1.depth;
		data2.hashValue = data1.hashValue;
		data2.value = data1.value;
		data2.valueType = data1.valueType;

		data1.bestMove = bestMove;
		data1.depth = depth;
		data1.hashValue = zobristHash;
		data1.value = value;
		data1.valueType = valueType;
	}
}