

#ifndef EXTRACTION_H
#define EXTRACTION_H

#include <cstdint>
#include <ctime>
#include <string>
#include "log.hpp"

namespace TDMS {

  template<typename T>
  T read_le( const unsigned char* p ) {
    T sum = p[0];
    for ( size_t i = 1; i < sizeof (T ); ++i ) {
      sum |= T( p[i] ) << ( 8 * i );
    }
    return sum;
  }


  std::string read_string( const unsigned char* p );

  double read_le_double( const unsigned char* p );

  float read_le_float( const unsigned char* p );

  time_t read_timestamp( const unsigned char* p );
}

#endif