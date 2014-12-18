#pragma once

#include <vector>

/** Utility methods for std::vector */
namespace VectorUtils
{
	/**
	 * Removes the first occurence of an element equal to ''e'' from
	 * the given vector ''v'', by swapping the last element of ''v''
	 * into the position of the element to be removed. This method does
	 * NOT preserve the ordering of the vector!
	 */
	template<typename T> static void removeSwap(std::vector<T>& v, const T& e);
}

template<typename T> void VectorUtils::removeSwap(std::vector<T>& v, const T& e)
{
	int indexToRemove = -1;
	for (int i = 0; i < v.size(); ++i)
	{
		if (e == v[i])
		{
			indexToRemove = i;
			break;
		}
	}

	if (indexToRemove >= 0)
	{
		if (indexToRemove != v.size() - 1)
		{
			v[indexToRemove] = std::move(v.back());
		}

		v.pop_back();
	}
}