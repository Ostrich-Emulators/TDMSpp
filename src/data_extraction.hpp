#include <cstdint>
#include <ctime>
#include <string>
#include <string.h> // For memcpy
#include "log.hpp"

namespace TDMS
{

template<typename T>
T read_le(const unsigned char* p)
{
    T sum = p[0];
    for(size_t i = 1; i < sizeof(T); ++i)
    {
        sum |= T(p[i]) << (8*i);
    }
    return sum;
}

time_t read_timestamp(const unsigned char* p)
{
		unsigned long long fraction = read_le<uint64_t>( p ); // time_t doesn't support ms resolution
		long long secsSince1904 = read_le<int64_t>( p + sizeof (uint64_t ) );
		time_t result = secsSince1904 - 2082844800; // tdms epoch is 1/1/1904, not 1/1/1970
		return result;
}

std::string read_string(const unsigned char* p)
{
    uint32_t len = read_le<uint32_t>(p);
    return std::string((const char*)p + 4, len);
}

double read_le_double(const unsigned char* p)
{
    double a;
    char* b = (char*)(double*)&a;
    // TODO: doesn't work on LE systems.
    // Solve with read_le template?
    memcpy(b, p, sizeof(double));
    return a;
}
float read_le_float(const unsigned char* p)
{
    float a;
    char* b = (char*)(float*)&a;
    // TODO: doesn't work on LE systems.
    // Solve with read_le template?
    memcpy(b, p, sizeof(float));
    return a;
}
}
