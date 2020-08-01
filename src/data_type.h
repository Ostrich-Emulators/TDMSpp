/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   data_type.h
 * Author: ryan
 *
 * Created on August 1, 2020, 10:22 AM
 */

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include "exports.h"
#include <functional>
#include <map>

namespace TDMS {

  class data_type_t {
  public:
    typedef std::function<void* ( ) > parse_t;

    TDMS_EXPORT data_type_t( );

    TDMS_EXPORT data_type_t( const data_type_t& dt );

    TDMS_EXPORT data_type_t( const std::string& _name, const size_t _len,
        std::function<void (const unsigned char*, void*) > reader );

    TDMS_EXPORT data_type_t( const std::string& _name,
        const size_t _len,
        std::function<void (const unsigned char*, void*) > reader,
        std::function<void (const unsigned char*, void*, size_t ) > array_reader );

    TDMS_EXPORT data_type_t( const std::string& _name, const size_t _len, const size_t _ctype_len,
        std::function<void (const unsigned char*, void*) > reader );

    TDMS_EXPORT bool is_valid( ) const {
      return (_name != "INVALID TYPE" );
    }

    TDMS_EXPORT bool operator==(const data_type_t& dt ) const {
      return (_name == dt._name );
    }

    TDMS_EXPORT bool operator!=(const data_type_t& dt ) const {
      return !( *this == dt );
    }

    TDMS_EXPORT void* read( const unsigned char* data ) {
      void* d = malloc( _ctype_length );
      read_to( data, d );
      return d;
    }

    TDMS_EXPORT static const std::map<uint32_t, const data_type_t> _tds_datatypes;

    TDMS_EXPORT const std::string& name( ) const {
      return _name;
    }

    TDMS_EXPORT size_t length( ) const {
      return _length;
    }

    TDMS_EXPORT size_t ctype_length( ) const {
      return _ctype_length;
    }

    TDMS_EXPORT bool is_string( ) const {
      return ( "tdsTypeString" == _name );
    }
  private:
    std::string _name;
    size_t _length;
    size_t _ctype_length;
    void _init_default_array_reader( );
    std::function<void (const unsigned char*, void*) > read_to;
    std::function<void (const unsigned char*, void*, size_t ) > read_array_to;
  };
}
#endif /* DATA_TYPE_H */

