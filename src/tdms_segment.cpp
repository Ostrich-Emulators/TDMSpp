#include <iostream> // Debugging
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <map>
#include <algorithm>
#include <string>

#include "tdms_file.hpp"
#include "tdms_segment.hpp"
#include "log.hpp"
#include "data_extraction.hpp"
#include "tdms_exceptions.h"
#include "data_type.h"
#include "tdms_channel.h"

namespace TDMS{

  const std::map<const std::string, int32_t> segment::_toc_properties = {
    {"kTocMetaData", int32_t( 1 ) << 1 },
    {"kTocRawData", int32_t( 1 ) << 3 },
    {"kTocDAQmxRawData", int32_t( 1 ) << 7 },
    {"kTocInterleavedData", int32_t( 1 ) << 5 },
    {"kTocBigEndian", int32_t( 1 ) << 6 },
    {"kTocNewObjList", int32_t( 1 ) << 2 }
  };

  segment::segment( uulong segment_start, segment * previous_segment, tdmsfile * file )
      : _startpos_in_file( segment_start ), _parent_file( file ) {

    fseek( file->f, segment_start, SEEK_SET );

    unsigned char justread[8]; // biggest element we'll read here
    auto ok = fread( justread, sizeof ( char ), 4, file->f );

    const char* header = "TDSm";
    if ( !ok || memcmp( justread, header, 4 ) != 0 ) {
      throw no_segment_error( );
    }

    // First four bytes are toc mask
    ok = fread( justread, sizeof ( char ), 4, file->f );
    int32_t toc_mask = read_le<int32_t>( (const unsigned char *) &justread[0] );

    log::debug( ) << "Properties:";
    for ( auto prop : segment::_toc_properties ) {
      _toc[prop.first] = ( toc_mask & prop.second ) != 0;

      if ( _toc[prop.first] ) {
        log::debug( ) << "\t" << prop.first;
      }
    }
    log::debug( ) << std::endl;

    // Four bytes for version number
    ok = fread( justread, sizeof ( char ), 4, file->f );
    if ( !ok ) {
      throw read_error( );
    }

    int32_t version = read_le<int32_t>( (const unsigned char *) &justread[0] );
    log::debug( ) << "Version: " << version << std::endl;
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
      throw read_error( );
    }
    uint64_t next_segment_offset = read_le<uint64_t>( (const unsigned char *) &justread[0] );
    ok = fread( justread, sizeof ( char ), 8, file->f );
    if ( !ok ) {
      throw read_error( );
    }
    uint64_t raw_data_offset = read_le<uint64_t>( (const unsigned char *) &justread[0] );

    // we'll add 4+4+4+8+8 = 28 bytes to our offsets
    // because we've read 28 bytes from the start of the segment
    this->_data_offset = raw_data_offset + 28; // bytes from start of the segment to data
    log::debug( ) << "raw data starts " << _data_offset << " bytes after the start of segment" << std::endl;


    if ( next_segment_offset == 0xFFFFFFFFFFFFFFFF ) { // That's 8 times FF, or 16 F's, aka the maximum unsigned int64_t.
      throw std::runtime_error( "Labview probably crashed, file is corrupt. Not attempting to read." );
    }
    this->_next_segment_offset = next_segment_offset + 28;

    // prepare enough space to load the metadata into memory
    auto segment_metadata = std::vector<unsigned char>( raw_data_offset );
    // read the metadata into memory (the file stream is currently pointing to
    // the start of the metadata)
    ok = fread( segment_metadata.data( ), 1, raw_data_offset, file->f );
    if ( !ok ) {
      throw read_error( );
    }

    if ( ok != raw_data_offset ) {
      log::debug( ) << "want to read " << raw_data_offset << ", but only got: " << ok << std::endl;

      auto newdata = std::vector<unsigned char>( raw_data_offset - ok );
      auto ok2 = fread( newdata.data( ), 1, raw_data_offset - ok, file->f );

      if ( !ok2 || ok2 < raw_data_offset - ok ) {
        log::debug( ) << "Double BAM! want to read " << raw_data_offset - ok << ", but only got: " << ok2 << std::endl;
      }
    }

    _parse_metadata( segment_metadata.data( ), previous_segment );
  }

  segment::~segment( ) { }

  void segment::_parse_metadata( const unsigned char* data,segment * previous_segment ) {
    if ( !this->_toc["kTocMetaData"] ) {
      if ( !previous_segment )
        throw std::runtime_error( "kTocMetaData is set for segment, but there is no previous segment." );
      for ( const auto& chunki : previous_segment->_ordered_chunks ) {
        this->_ordered_chunks.push_back( chunki );
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
      for ( const auto& chunky : previous_segment->_ordered_chunks ) {
        this->_ordered_chunks.push_back( chunky );
      }
    }

    // Read number of metadata objects
    int32_t num_chunks = read_le<int32_t>( data );
    data += 4;

    for ( int i = 0; i < num_chunks; ++i ) {
      std::string object_path = read_string( data );
      data += 4 + object_path.size( );
      log::debug( ) << object_path << std::endl;

      auto channel = _parent_file->find_or_make_channel( object_path );
      bool updating_existing = false;

      datachunk * segment_chunk = nullptr;

      if ( !_toc["kTocNewObjList"] ) {
        // Search for the same object from the previous
        // segment object list
        for ( auto& segchunk : _ordered_chunks ) {
          if ( segchunk._tdms_channel == channel ) {
            segment_chunk = &segchunk;
            updating_existing = true;
            log::debug( ) << "Updating object in segment list." << std::endl;
            break;
          }
        }
      }
      if ( !updating_existing ) {
        auto newchunk = datachunk( );

        if ( channel->has_previous() ) {
          log::debug( ) << "Copying previous segment object" << std::endl;
          newchunk = channel->_previous_segment_chunk;
        }
        else {
          newchunk = datachunk{ channel };
        }
        this->_ordered_chunks.push_back( newchunk );

        segment_chunk = &this->_ordered_chunks[this->_ordered_chunks.size( ) - 1];
      }
      data = segment_chunk->_parse_metadata( data );
      channel->_previous_segment_chunk = *segment_chunk;
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
    for ( const auto& chunky : _ordered_chunks ) {
      if ( chunky._has_data ) {
        data_size += chunky._data_size;
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
    for ( auto& chunki : this->_ordered_chunks ) {
      if ( chunki._has_data ) {
        chunki._tdms_channel->_number_values
            += ( chunki._number_values * this->_num_chunks );
      }
    }
  }

  void segment::_parse_raw_data( listener * listener ) {
    if ( !this->_toc["kTocRawData"] ) {
      return;
    }

    size_t total_data_size = _next_segment_offset - _data_offset;
    if ( 0 == total_data_size ) {
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
    auto ok = fread( _parent_file->segbuff.data( ), total_data_size, 1, _parent_file->f );
    if ( !ok ) {
      throw read_error( );
    }
    const unsigned char * d = _parent_file->segbuff.data( );

    for ( size_t chunk = 0; chunk < _num_chunks; ++chunk ) {
      if ( this->_toc["kTocInterleavedData"] ) {
        throw std::runtime_error( "Reading interleaved data not supported yet" );
      }
      else {
        for ( auto& chunky : _ordered_chunks ) {
          if ( chunky._has_data ) {
            size_t bytes_processed = chunky._read_values( d, e, listener );
            d += bytes_processed;
          }
        }
      }
    }
  }
}
