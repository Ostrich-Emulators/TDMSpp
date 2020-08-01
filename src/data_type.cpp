/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "data_type.h"

#include "log.hpp"
#include "data_extraction.hpp"
#include <cstring> // memcpy

namespace TDMS{

  data_type_t::data_type_t( const data_type_t& dt ) :
      _name( dt._name ),
      _length( dt._length ),
      _ctype_length( dt._ctype_length ),
      read_to( dt.read_to ),
      read_array_to( dt.read_array_to ) { }

  data_type_t::data_type_t( ) :
      _name( "INVALID TYPE" ),
      _length( 0 ),
      _ctype_length( 0 ) {
    _init_default_array_reader( );
  }

  data_type_t::data_type_t( const std::string& _n, const size_t _len,
      std::function<void (const unsigned char*, void*) > reader ) :
      _name( _n ),
      _length( _len ),
      _ctype_length( _len ),
      read_to( reader ) {
    _init_default_array_reader( );
  }

  data_type_t::data_type_t( const std::string& _n, const size_t _len,
      std::function<void (const unsigned char*, void*) > reader,
      std::function<void (const unsigned char*, void*, size_t ) > array_reader ) :
      _name( _n ),
      _length( _len ),
      _ctype_length( _len ),
      read_to( reader ),
      read_array_to( array_reader ) { }

  data_type_t::data_type_t( const std::string& _n, const size_t _len, const size_t _ctype_len,
      std::function<void (const unsigned char*, void*) > reader ) :
      _name( _n ),
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

  template<typename T>
  inline std::function<void (const unsigned char*, void*) > put_on_heap_generator( std::function<T( const unsigned char* )> f ) {
    return [f](const unsigned char* data, void* ptr ) {
      *( (T*) ptr ) = f( data );
    };
  }

  template<typename T>
  inline std::function<void (const unsigned char*, void*) > put_le_on_heap_generator( ) {
    return put_on_heap_generator<T>( &read_le<T> );
  }

  template<typename T>
  inline std::function<void (const unsigned char*, void*, size_t ) > copy_array_reader_generator( ) {
    return [](const unsigned char* source, void* tgt, size_t number_values ) {
      memcpy( tgt, source, number_values * sizeof (T ) );
    };
  }

  std::function<void (const unsigned char*, void*) > not_implemented = [](const unsigned char*, void*) {
    throw std::runtime_error{"Reading this type is not implemented. Aborting" };
  };

  const std::map<uint32_t, const data_type_t> data_type_t::_tds_datatypes = {
    { 0, data_type_t( "tdsTypeVoid", 0, not_implemented ) },
    { 1, data_type_t( "tdsTypeI8", 1, put_le_on_heap_generator<int8_t>( ), copy_array_reader_generator<int8_t>( ) ) },
    { 2, data_type_t( "tdsTypeI16", 2, put_le_on_heap_generator<int16_t>( ) ) },
    { 3, data_type_t( "tdsTypeI32", 4, put_le_on_heap_generator<int32_t>( ) ) },
    { 4, data_type_t( "tdsTypeI64", 8, put_le_on_heap_generator<int32_t>( ) ) },
    { 5, data_type_t( "tdsTypeU8", 1, put_le_on_heap_generator<uint8_t>( ) ) },
    { 6, data_type_t( "tdsTypeU16", 2, put_le_on_heap_generator<uint16_t>( ) ) },
    { 7, data_type_t( "tdsTypeU32", 4, put_le_on_heap_generator<uint32_t>( ) ) },
    { 8, data_type_t( "tdsTypeU64", 8, put_le_on_heap_generator<uint64_t>( ) ) },
    { 9, data_type_t( "tdsTypeSingleFloat", 4, put_on_heap_generator<float>( &read_le_float ), copy_array_reader_generator<float>( ) ) },
    { 10, data_type_t( "tdsTypeDoubleFloat", 8, put_on_heap_generator<double>( &read_le_double ), copy_array_reader_generator<double>( ) ) },
    { 11, data_type_t( "tdsTypeExtendedFloat", 0, not_implemented ) },
    { 12, data_type_t( "tdsTypeDoubleFloatWithUnit", 8, not_implemented ) },
    { 13, data_type_t( "tdsTypeExtendedFloatWithUnit", 0, not_implemented ) },
    { 0x19, data_type_t( "tdsTypeSingleFloatWithUnit", 4, not_implemented ) },
    { 0x20, data_type_t( "tdsTypeString", 0, not_implemented ) },
    { 0x21, data_type_t( "tdsTypeBoolean", 1, not_implemented ) },
    { 0x44, data_type_t( "tdsTypeTimeStamp", 16, put_on_heap_generator<time_t>( &read_timestamp ) ) },
    { 0xFFFFFFFF, data_type_t( "tdsTypeDAQmxRawData", 0, not_implemented ) }
  };
}