/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "data_type.h"

#include "log.hpp"
namespace TDMS{

  data_type_t::data_type_t( const data_type_t& dt )
      : _name( dt._name ),
      _length( dt._length ),
      _ctype_length( dt._ctype_length ),
      read_to( dt.read_to ),
      read_array_to( dt.read_array_to ) { }

  data_type_t::data_type_t( )
      : _name( "INVALID TYPE" ),
      _length( 0 ),
      _ctype_length( 0 ) {
    _init_default_array_reader( );
  }

  data_type_t::data_type_t( const std::string& _n,
      const size_t _len,
      std::function<void (const unsigned char*, void*) > reader )
      : _name( _n ),
      _length( _len ),
      _ctype_length( _len ),
      read_to( reader ) {
    _init_default_array_reader( );
  }

  data_type_t::data_type_t( const std::string& _n,
      const size_t _len,
      std::function<void (const unsigned char*, void*) > reader,
      std::function<void (const unsigned char*, void*, size_t ) > array_reader )
      : _name( _n ),
      _length( _len ),
      _ctype_length( _len ),
      read_to( reader ),
      read_array_to( array_reader ) { }

  data_type_t::data_type_t( const std::string& _n,
      const size_t _len,
      const size_t _ctype_len,
      std::function<void (const unsigned char*, void*) > reader )
      : _name( _n ),
      _length( _len ),
      _ctype_length( _ctype_len ),
      read_to( reader ) {
    _init_default_array_reader( );
  }

  void data_type_t::_init_default_array_reader( ) {
    auto read = this->read_to;
    auto ctype = this->ctype_length( );
    read_array_to = [read, ctype](const unsigned char* source, void* target, size_t number_values ) {
      log::debug( ) << "Doing iterative reading" << std::endl;
      for ( size_t i = 0; i < number_values; ++i ) {
        read( source + ( i * ctype ), (void*) ( ( (char*) target ) + ( i * ctype ) ) );
      }
    };
  }
}