#include "RNG.h"

#include <time.h>

RNG::Generator_32::Generator_32()
	: gen()
{
	gen.seed(time(nullptr));
}

RNG::Generator_64::Generator_64()
	: gen()
{
	gen.seed(time(nullptr));
}

uint32_t RNG::randomUint_32()
{
	static RNG::Generator_32 gen_32;
	return gen_32.randomUint_32();
}

uint64_t RNG::randomUint_64()
{
	static RNG::Generator_64 gen_64;
	return gen_64.randomUint_64();
}