#pragma once

/**
 * Compile-time options
 */


// If defined, statistics such as number of nodes visited and time spent will be gathered by AI Engines
#define GATHER_STATISTICS

#ifdef GATHER_STATISTICS

// If defined, AI engine statistics will be logged at the end of every turn played by AI
//#define LOG_STATS_PER_TURN
// If defined, AI engine statistics will be logged at the end of a match
//#define LOG_STATS_END_OF_MATCH

#endif // GATHER_STATISTICS