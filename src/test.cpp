//#define LOG_DEBUG_ENABLED
#define LOG_ENABLED

#include "effective_allocator.h"

#include <vector>
#include <set>
#include <deque>
#include <list>


#include <fstream>
#include <iterator>
#include <iostream>

#include <chrono>

#define MEASURE_TIME(unit, ...)    \
		[&] {         \
	using namespace std::chrono;     \
	auto start = high_resolution_clock::now ();    \
	__VA_ARGS__;       \
	auto time = high_resolution_clock::now () - start;   \
	return duration_cast<unit> (time).count ();		 \
} ();

std::vector<std::string> load_data (const std::string& filename)
{
	std::vector<std::string> data;
	std::ifstream file (filename);

	std::copy(
			std::istream_iterator<std::string>(file),
			{},
			back_inserter(data));

	LOG("loaded", data.size(), "values from", filename);

	return data;
}

typedef std::chrono::microseconds time_unit;
typedef int data_type;

template<template<typename, typename> class Container, typename Alloc = std::allocator<data_type>>
long int measure_sequence_container(int size)
{
	return MEASURE_TIME(time_unit,
		Container<data_type, Alloc> vect;
		for (int d = 0; d < size; d++)
			vect.push_back(d);
	);
}

template<template<typename, typename, typename> class Container, typename Alloc = std::allocator<data_type>>
long int measure_associative_container(int size)
{
	return MEASURE_TIME(time_unit,
		Container<data_type, std::less<data_type>, Alloc> s;
		for (int d = 0; d < size; d++)
		s.insert(d);
	);
}

#define MEASURE_SEQUENCE(container_type, data_size, alloc_size) \
		LOG(#container_type " standard allocator: ", \
				measure_sequence_container<container_type>(data_size)); \
		LOG(#container_type " effective allocator: ", \
				measure_sequence_container<container_type, effective_allocator<data_type, alloc_size>>(data_size));

#define MEASURE_ASSOCIATIVE(container_type, data_size, alloc_size) \
		LOG(#container_type " standard allocator: ", \
				measure_associative_container<container_type>(data_size)); \
		LOG(#container_type " effective allocator: ", \
				measure_associative_container<container_type, effective_allocator<data_type, alloc_size>>(data_size));


int main (int argc, char **argv)
{
	/*if (argc < 2)
	{
		LOG("Usage:", argv[0], " <data file>");
		return 0;
	}

	auto data = load_data (argv[1]);
	*/

	int seq_size = 99999999;
	int assoc_size = 9999999;

	MEASURE_SEQUENCE(std::vector, seq_size, 1024*256)
	MEASURE_SEQUENCE(std::list, seq_size/10, 1024*256)

	MEASURE_ASSOCIATIVE(std::set, assoc_size, 1024*16)
	MEASURE_ASSOCIATIVE(std::multiset, assoc_size, 1024*16)


	return 0;
}
