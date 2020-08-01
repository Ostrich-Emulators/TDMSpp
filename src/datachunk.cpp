/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "datachunk.h"
#include "tdms_channel.h"
#include "log.hpp"
#include "data_extraction.hpp"


namespace TDMS{

  size_t datachunk::_read_values( const unsigned char*& data, endianness e, listener * earful ) {
    if ( _data_type.is_string( ) ) {
      log::debug( ) << "Reading string data" << std::endl;
      throw std::runtime_error( "Reading string data not yet implemented" );
      // TODO ^
    }

    //unsigned char* read_data = ()(_tdms_object->_data_start + _tdms_object->_data_insert_position);

    //_data_type.read_array_to(data, read_data, _number_values);

    //_tdms_object->_data_insert_position += (_number_values*_data_type.ctype_length);
    //data += (_number_values*_data_type.ctype_length);
    //std::cout << "reading " << _number_values << " values (not really :) for " << _tdms_object->_path << std::endl;
    if ( earful ) {
      earful->data( _tdms_channel->_path, data, _data_type, _number_values );
    }

    return _number_values * _data_type.ctype_length( );
  }

  datachunk::datachunk( channel * o ) :
      _tdms_channel( o ),
      _number_values( 0 ),
      _data_size( 0 ),
      _has_data( nullptr != o ),
      _dimension( 1 ),
      _data_type( data_type_t::_tds_datatypes.at( 0 ) ) { }

  datachunk::datachunk( const datachunk& orig ) :
      _tdms_channel( orig._tdms_channel ),
      _number_values( orig._number_values ),
      _data_size( orig._data_size ),
      _has_data( orig._has_data ),
      _dimension( orig._dimension ),
      _data_type( orig._data_type ) { }

  const unsigned char* datachunk::_parse_metadata( const unsigned char* data ) {
    // Read object metadata and update object information
    uint32_t raw_data_index = read_le<uint32_t>( data );
    data += 4;

    log::debug( ) << "Reading metadata for object " << _tdms_channel->_path << std::endl
        << "raw_data_index: " << raw_data_index << std::endl;

    if ( raw_data_index == 0xFFFFFFFF ) {
      log::debug( ) << "Object has no data" << std::endl;
      _has_data = false;
    }
    else if ( raw_data_index == 0x00000000 ) {
      log::debug( ) << "Object has same data structure as in the previous segment" << std::endl;
      _has_data = true;
    }
    else {
      // raw_data_index gives the length of the index information.
      _tdms_channel->_has_data = _has_data = true;
      // Read the datatype
      uint32_t datatype = read_le<uint32_t>( data );
      data += 4;

      try {
        _data_type = data_type_t::_tds_datatypes.at( datatype );
      }
      catch ( std::out_of_range& e ) {
        throw std::out_of_range( "Unrecognized datatype in file" );
      }
      if ( _tdms_channel->_data_type.is_valid( ) && _tdms_channel->_data_type != _data_type ) {
        throw std::runtime_error( "Segment object doesn't have the same data type as previous segments" );
      }
      else {
        _tdms_channel->_data_type = _data_type;
      }

      log::debug( ) << "datatype " << _data_type.name( ) << std::endl;

      // Read data dimension
      _dimension = read_le<uint32_t>( data );
      data += 4;
      if ( _dimension != 1 ) {
        log::debug( ) << "Warning: dimension != 1" << std::endl;
      }

      // Read the number of values
      _number_values = read_le<uint64_t>( data );
      data += 8;

      // Variable length datatypes have total length
      if ( _data_type.is_string( ) ) {
        _data_size = read_le<uint64_t>( data );
        data += 8;
      }
      else {
        _data_size = ( _number_values * _dimension * _data_type.length( ) );
      }
      log::debug( ) << "Number of elements in segment for " << _tdms_channel->_path << ": " << _number_values << std::endl;
    }
    // Read data properties
    uint32_t num_properties = read_le<uint32_t>( data );
    data += 4;
    log::debug( ) << "Reading " << num_properties << " properties" << std::endl;
    for ( size_t i = 0; i < num_properties; ++i ) {
      std::string prop_name = read_string( data );
      data += 4 + prop_name.size( );
      // Property data type
      auto prop_data_type = data_type_t::_tds_datatypes.at( read_le<uint32_t>( data ) );
      data += 4;
      if ( prop_data_type.is_string( ) ) {
        std::string* property = new std::string( read_string( data ) );
        log::debug( ) << "Property " << prop_name << ": " << *property << std::endl;
        data += 4 + property->size( );
        _tdms_channel->_properties.emplace( prop_name,
            std::shared_ptr<channel::property>(
            new channel::property( prop_data_type, (void*) property ) ) );
      }
      else {
        void* prop_val = prop_data_type.read( data );
        if ( prop_val == nullptr ) {
          throw std::runtime_error( "Unsupported datatype " + prop_data_type.name( ) );
        }

        data += prop_data_type.length( );
        _tdms_channel->_properties.emplace( prop_name,
            std::shared_ptr<channel::property>(
            new channel::property( prop_data_type, prop_val ) ) );
        log::debug( ) << "Property " << prop_name << " has been read (" << prop_data_type.name( ) << ")" << std::endl;
      }
    }

    return data;
  }
}