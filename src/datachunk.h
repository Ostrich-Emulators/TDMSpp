/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   datachunk.h
 * Author: ryan
 *
 * Created on August 1, 2020, 12:22 PM
 */

#ifndef DATACHUNK_H
#define DATACHUNK_H

#include "data_type.h"
#include "exports.h"
#include "tdms_listener.h"

namespace TDMS {
  class segment;
  class channel;

  enum class endianness {
    BIG,
    LITTLE
  };

  class datachunk {
    friend class segment;
    friend class channel;
  public:
    TDMS_EXPORT datachunk( const datachunk& o );
    TDMS_EXPORT datachunk( channel * o = nullptr );

  private:
    const unsigned char* _parse_metadata( const unsigned char* data );
    size_t _read_values( const unsigned char*& data, endianness e, listener * );

    channel * _tdms_channel;
    uint64_t _number_values;
    uint64_t _data_size;
    bool _has_data;
    uint32_t _dimension;
    data_type_t _data_type;
  };

}

#endif /* DATACHUNK_H */

