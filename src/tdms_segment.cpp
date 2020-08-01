#include <iostream> // Debugging
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <map>
#include <algorithm>
#include <string>

#include "tdms.hpp"
#include "tdms_impl.hpp"
#include "log.hpp"
#include "data_extraction.hpp"

namespace TDMS{

  const std::map<const std::string, int32_t> segment::_toc_properties = {
    {"kTocMetaData", int32_t( 1 ) << 1 },
    {"kTocRawData", int32_t( 1 ) << 3 },
    {"kTocDAQmxRawData", int32_t( 1 ) << 7 },
    {"kTocInterleavedData", int32_t( 1 ) << 5 },
    {"kTocBigEndian", int32_t( 1 ) << 6 },
    {"kTocNewObjList", int32_t( 1 ) << 2 }
  };

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
    {0xFFFFFFFF, data_type_t( "tdsTypeDAQmxRawData", 0, not_implemented ) }
  };

  segment::segment( uulong segment_start, const std::unique_ptr<segment>& previous_segment, tdmsfile* file )
  : _startpos_in_file( segment_start ), _parent_file( file ) {

    fseek( file->f, segment_start, SEEK_SET );

    unsigned char justread[8]; // biggest element we'll read here
    auto ok = fread( justread, sizeof ( char ), 4, file->f );

    const char* header = "TDSm";
    if ( !ok || memcmp( justread, header, 4 ) != 0 ) {
      throw segment::no_segment_error( );
    }

    // First four bytes are toc mask
    ok = fread( justread, sizeof ( char ), 4, file->f );
    int32_t toc_mask = read_le<int32_t>( (const unsigned char *) &justread[0] );

    log::debug() << "Properties:";
    for ( auto prop : segment::_toc_properties ) {
      _toc[prop.first] = ( toc_mask & prop.second ) != 0;

      if ( _toc[prop.first] ) {
        log::debug() << "\t" << prop.first;
      }
    }
    log::debug() << std::endl;

    // Four bytes for version number
    ok = fread( justread, sizeof ( char ), 4, file->f );
    if ( !ok ) {
      throw segment::read_error( );
    }

    int32_t version = read_le<int32_t>( (const unsigned char *) &justread[0] );
    log::debug() << "Version: " << version << std::endl;
    switch ( version ) {
      case 4712:
      case 4713:
        break;
      default:
        std::cerr << "segment: unknown version number " << version << std::endl;
        break;
    }

    // 64 bits pointer to next segment
    // and same for raw data offset
    ok = fread( justread, sizeof ( char ), 8, file->f );
    if ( !ok ) {
      throw segment::read_error( );
    }
    uint64_t next_segment_offset = read_le<uint64_t>( (const unsigned char *) &justread[0] );
    ok = fread( justread, sizeof ( char ), 8, file->f );
    if ( !ok ) {
      throw segment::read_error( );
    }
    uint64_t raw_data_offset = read_le<uint64_t>( (const unsigned char *) &justread[0] );

    // we'll add 4+4+4+8+8 = 28 bytes to our offsets
    // because we've read 28 bytes from the start of the segment
    this->_data_offset = raw_data_offset + 28; // bytes from start of the segment to data
    log::debug() << "raw data starts " << _data_offset << " bytes after the start of segment" << std::endl;


    if ( next_segment_offset == 0xFFFFFFFFFFFFFFFF ) { // That's 8 times FF, or 16 F's, aka the maximum unsigned int64_t.
      throw std::runtime_error( "Labview probably crashed, file is corrupt. Not attempting to read." );
    }
    this->_next_segment_offset = next_segment_offset + 28;

    // prepare enough space to load the metadata into memory
    auto segment_metadata = std::vector<unsigned char>( raw_data_offset );
    // read the metadata into memory (the file stream is currently pointing to
    // the start of the metadata)
    ok = fread( segment_metadata.data(), 1, raw_data_offset, file->f );
    if ( !ok ) {
      throw segment::read_error( );
    }

    if (ok != raw_data_offset) {
        log::debug() << "want to read " << raw_data_offset << ", but only got: " << ok << std::endl;

        auto newdata = std::vector<unsigned char>(raw_data_offset-ok);
        auto ok2=fread(newdata.data(), 1, raw_data_offset-ok, file->f);

        if (!ok2||ok2<raw_data_offset-ok) {
            log::debug() << "Double BAM! want to read " << raw_data_offset-ok << ", but only got: " << ok2 << std::endl;
        }
    }

    _parse_metadata( segment_metadata.data(), previous_segment );
  }

  segment::~segment( ) {
  }

  void segment::_parse_metadata( const unsigned char* data, const std::unique_ptr<segment>& previous_segment ) {
    if ( !this->_toc["kTocMetaData"] ) {
      if ( !previous_segment )
        throw std::runtime_error( "kTocMetaData is set for segment, but there is no previous segment." );
      for ( const auto& chunk : previous_segment->_ordered_chunks ) {
        this->_ordered_chunks.push_back( std::unique_ptr<datachunk>( new datachunk( *chunk ) ) );
      }
      _calculate_chunks( );
      return;
    }
    if ( !this->_toc["kTocNewObjList"] ) {
      // In this case, there can be a list of new objects that
      // are appended, or previous objects can also be repeated
      // if their properties change

      if ( !previous_segment ) {
        throw std::runtime_error( "kTocNewObjList is set for segment, but there is no previous segment." );
      }
      for ( const auto& chunk : previous_segment->_ordered_chunks ) {
        this->_ordered_chunks.push_back( std::unique_ptr<datachunk>( new datachunk( *chunk ) ) );
      }
    }

    // Read number of metadata objects
    int32_t num_chunks = read_le<int32_t>( data );
    data += 4;

    for ( int i = 0; i < num_chunks; ++i ) {
      std::string object_path = read_string( data );
      data += 4 + object_path.size( );
      log::debug() << object_path << std::endl;

      std::unique_ptr<channel>& channel = _parent_file->find_or_make_channel( object_path );
      bool updating_existing = false;

      datachunk * segment_chunk = nullptr;

      if ( !_toc["kTocNewObjList"] ) {
        // Search for the same object from the previous
        // segment object list
        for ( auto& segchunk : _ordered_chunks ) {
          if ( segchunk->_tdms_channel == channel ) {
            segment_chunk = segchunk.get( );
            updating_existing = true;
            log::debug() << "Updating object in segment list." << std::endl;
            break;
          }
        }
      }
      if ( !updating_existing ) {
        std::unique_ptr<datachunk> newchunk;
        if ( channel->_previous_segment_chunk ) {
          log::debug() << "Copying previous segment object" << std::endl;
          newchunk.reset( new datachunk( *channel->_previous_segment_chunk ) );
        }
        else {
          newchunk.reset( new datachunk( channel ) );
        }
        this->_ordered_chunks.push_back( std::move( newchunk ) );

        segment_chunk = this->_ordered_chunks[this->_ordered_chunks.size( ) - 1].get( );
      }
      data = segment_chunk->_parse_metadata( data );
      channel->_previous_segment_chunk.reset( new datachunk( *segment_chunk ) );
    }
    _calculate_chunks( );
  }

  void segment::_calculate_chunks( ) {
    // Work out the number of chunks the data is in, for cases
    // where the meta data doesn't change at all so there is no
    // lead in.
    // Also increments the number of values for objects in this
    // segment, based on the number of chunks.

    // Count the datasize
    long long data_size = 0;
    for ( const auto& chunk : _ordered_chunks ) {
      if ( chunk->_has_data ) {
        data_size += chunk->_data_size;
      }
    }
    long long total_data_size = this->_next_segment_offset - this->_data_offset;

    if ( data_size < 0 || total_data_size < 0 ) {
      throw std::runtime_error( "Negative data size" );
    }
    else if ( data_size == 0 ) {
      if ( total_data_size != data_size ) {
        throw std::runtime_error( "Zero channel data size but non-zero data length based on segment offset." );
      }
      this->_num_chunks = 0;
      return;
    }
    if ( ( total_data_size % data_size ) != 0 ) {
      throw std::runtime_error( "Data size is not a multiple of the chunk size" );
    }
    else {
      this->_num_chunks = total_data_size / data_size;
    }

    // Update data count for the overall tdms object
    // using the data count for this segment.
    for ( auto& chunk : this->_ordered_chunks ) {
      if ( chunk->_has_data ) {
        chunk->_tdms_channel->_number_values
            += ( chunk->_number_values * this->_num_chunks );
      }
    }
  }

  void segment::_parse_raw_data( std::unique_ptr<listener>& listener ) {
    if ( !this->_toc["kTocRawData"] ) {
      return;
    }

    size_t total_data_size = _next_segment_offset - _data_offset;
    if( 0 == total_data_size ){
      // no data in this segment, so nothing to do
      return;
    }

    endianness e = endianness::LITTLE;
    if ( this->_toc["kTocBigEndian"] ) {
      e = BIG;
      throw std::runtime_error( "Big endian reading not yet implemented" );
    }

    // move the file pointer to the start of this segment's data
    fseek( _parent_file->f, _startpos_in_file, SEEK_SET );
    fseek( _parent_file->f, _data_offset, SEEK_CUR );
    auto ok = fread( _parent_file->segbuff.data(), total_data_size, 1, _parent_file->f );
    if ( !ok ) {
      throw segment::read_error( );
    }
    const unsigned char * d = _parent_file->segbuff.data();

    for ( size_t chunk = 0; chunk < _num_chunks; ++chunk ) {
      if ( this->_toc["kTocInterleavedData"] ) {
        throw std::runtime_error( "Reading interleaved data not supported yet" );
      }
      else {
        for ( auto& chunk : _ordered_chunks ) {
          if ( chunk->_has_data ) {
            size_t bytes_processed = chunk->_read_values( d, e, listener );
            d += bytes_processed;
          }
        }
      }
    }
  }

  size_t datachunk::_read_values( const unsigned char*& data, endianness e,
      std::unique_ptr<listener>& earful ) {
    if ( _data_type.name == "tdsTypeString" ) {
      log::debug() << "Reading string data" << std::endl;
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

    return _number_values * _data_type.ctype_length;
  }

  /**
   * 		const std::unique_ptr<channel>& _tdms_channel;
    uint64_t _number_values;
    uint64_t _data_size;
    bool _has_data;
    uint32_t _dimension;
    data_type_t _data_type;
   */
  datachunk::datachunk( const std::unique_ptr<channel>& o ) :
  _tdms_channel( o ),
  _number_values( 0 ),
  _data_size( 0 ),
  _has_data( true ),
  _dimension( 1 ),
  _data_type( data_type_t::_tds_datatypes.at( 0 ) ) {
  }

  datachunk::datachunk( const datachunk& orig ) :
  _tdms_channel( orig._tdms_channel ),
  _number_values( orig._number_values ),
  _data_size( orig._data_size ),
  _has_data( orig._has_data ),
  _dimension( orig._dimension ),
  _data_type( orig._data_type ) {
  }

  const unsigned char* datachunk::_parse_metadata( const unsigned char* data ) {
    // Read object metadata and update object information
    uint32_t raw_data_index = read_le<uint32_t>( data );
    data += 4;

    log::debug() << "Reading metadata for object " << _tdms_channel->_path << std::endl
        << "raw_data_index: " << raw_data_index << std::endl;

    if ( raw_data_index == 0xFFFFFFFF ) {
      log::debug() << "Object has no data" << std::endl;
      _has_data = false;
    }
    else if ( raw_data_index == 0x00000000 ) {
      log::debug() << "Object has same data structure as in the previous segment" << std::endl;
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

      log::debug() << "datatype " << _data_type.name << std::endl;

      // Read data dimension
      _dimension = read_le<uint32_t>( data );
      data += 4;
      if ( _dimension != 1 ) {
        log::debug() << "Warning: dimension != 1" << std::endl;
      }

      // Read the number of values
      _number_values = read_le<uint64_t>( data );
      data += 8;

      // Variable length datatypes have total length
      if ( _data_type.name == "tdsTypeString" /*or None*/ ) {
        _data_size = read_le<uint64_t>( data );
        data += 8;
      }
      else {
        _data_size = ( _number_values * _dimension * _data_type.length );
      }
      log::debug() << "Number of elements in segment for " << _tdms_channel->_path << ": " << _number_values << std::endl;
    }
    // Read data properties
    uint32_t num_properties = read_le<uint32_t>( data );
    data += 4;
    log::debug() << "Reading " << num_properties << " properties" << std::endl;
    for ( size_t i = 0; i < num_properties; ++i ) {
      std::string prop_name = read_string( data );
      data += 4 + prop_name.size( );
      // Property data type
      auto prop_data_type = data_type_t::_tds_datatypes.at( read_le<uint32_t>( data ) );
      data += 4;
      if ( prop_data_type.name == "tdsTypeString" ) {
        std::string* property = new std::string( read_string( data ) );
        log::debug() << "Property " << prop_name << ": " << *property << std::endl;
        data += 4 + property->size( );
        _tdms_channel->_properties.emplace( prop_name,
            std::shared_ptr<channel::property>(
            new channel::property( prop_data_type, (void*) property ) ) );
      }
      else {
        void* prop_val = prop_data_type.read( data );
        if ( prop_val == nullptr ) {
          throw std::runtime_error( "Unsupported datatype " + prop_data_type.name );
        }

        data += prop_data_type.length;
        _tdms_channel->_properties.emplace( prop_name,
            std::shared_ptr<channel::property>(
            new channel::property( prop_data_type, prop_val ) ) );
        log::debug() << "Property " << prop_name << " has been read (" << prop_data_type.name << ")" << std::endl;
      }
    }

    return data;
  }
}
