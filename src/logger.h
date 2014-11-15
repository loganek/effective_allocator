/*
 * logger.h
 *
 *  Created on: Dec 21, 2014
 *      Author: loganek
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#if defined(LOG_ENABLED) || defined(LOG_DEBUG_ENABLED)

#include <iostream>

void print() { std::cout << std::endl << std::endl; }
template<typename First, typename...Rest >
void print( First parm1, Rest...parm )
{ std::cout << parm1 << " "; print(parm...); }
#define LOG(...) print(__VA_ARGS__);
#ifdef LOG_DEBUG_ENABLED
#define LOG_D(...) print(__VA_ARGS__);
#else
#define LOG_D(...)
#endif
#else
#define LOG(...)
#define LOG_D(...)
#endif


#endif /* LOGGER_H_ */
