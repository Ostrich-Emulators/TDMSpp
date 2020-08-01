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

namespace TDMS {

  class TDMS_EXPORT data_type_t {
  public:
    typedef std::function<void* ( ) > parse_t;

    data_type_t( const data_type_t& dt )
        : name( dt.name ),
        read_to( dt.read_to ),
        read_array_to( dt.read_array_to ),
        length( dt.length ),
        ctype_length( dt.ctype_length ) { }

    data_type_t( )
        : name( "INVALID TYPE" ),
        length( 0 ),
        ctype_length( 0 ) {
      _init_default_array_reader( );
    }

    data_type_t( const std::string& _name,
        const size_t _len,
        std::function<void (const unsigned char*, void*) > reader )
        : name( _name ),
        read_to( reader ),
        length( _len ),
        ctype_length( _len ) {
      _init_default_array_reader( );
    }

    data_type_t( const std::string& _name,
        const size_t _len,
        std::function<void (const unsigned char*, void*) > reader,
        std::function<void (const unsigned char*, void*, size_t ) > array_reader )
        : name( _name ),
        read_to( reader ),
        read_array_to( array_reader ),
        length( _len ),
        ctype_length( _len ) { }

    data_type_t( const std::string& _name,
        const size_t _len,
        const size_t _ctype_len,
        std::function<void (const unsigned char*, void*) > reader )
        : name( _name ),
        read_to( reader ),
        length( _len ),
        ctype_length( _ctype_len ) {
      _init_default_array_reader( );
    }

    bool is_valid( ) const {
      return (name != "INVALID TYPE" );
    }

    bool operator==(const data_type_t& dt ) const {
      return (name == dt.name );
    }

    bool operator!=(const data_type_t& dt ) const {
      return !( *this == dt );
    }

    std::string name;

    void* read( const unsigned char* data ) {
      void* d = malloc( ctype_length );
      read_to( data, d );
      return d;
    }
    std::function<void (const unsigned char*, void*) > read_to;
    std::function<void (const unsigned char*, void*, size_t ) > read_array_to;
    size_t length;
    size_t ctype_length;

    static const std::map<uint32_t, const data_type_t> _tds_datatypes;
  private:
    void _init_default_array_reader( );
  };
}
#endif /* DATA_TYPE_H */

