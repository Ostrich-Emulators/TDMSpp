#include <iostream> // Debugging
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <map>

#include "tdms.hpp"
#include "log.hpp"
#include "tdms_impl.hpp"

namespace TDMS{
  typedef unsigned long long uulong;

  tdmsfile::tdmsfile( const std::string& filename ) : filename( filename ) {
    f = fopen( filename.c_str( ), "r" );
    if ( !f ) {
      throw std::runtime_error( "File \"" + filename + "\" could not be opened" );
    }
    fseek( f, 0, SEEK_END );
    file_contents_size = ftell( f );
    fseek( f, 0, SEEK_SET );

    // Now parse the segments
    _parse_segments( f );
    file_contents_size = 0;
  }

  void tdmsfile::_parse_segments( FILE * f ) {
    uulong offset = 0;
    size_t maxsegmentsize = 0;
    // First read the metadata of the segments
    while ( offset < file_contents_size - 8 * 4 ) {
      try {
        std::unique_ptr<segment>& prev = ( _segments.empty( )
            ? nosegment
            : _segments[_segments.size( ) - 1] );

        log::debug << "parsing segment " << ( _segments.size( ) + 1 ) << " from offset: " << offset << log::endl;
        std::unique_ptr<segment> s( new segment( offset, prev, this ) );

        maxsegmentsize = std::max( maxsegmentsize, s->_next_segment_offset );

        offset += s->_next_segment_offset;
        _segments.push_back( std::move( s ) );
      }
      catch ( segment::no_segment_error& e ) {
        // Last segment was parsed.
        break;
      }
    }
    segbuff = (unsigned char*) malloc( maxsegmentsize );
  }

  void tdmsfile::loadSegment( size_t segnum, std::unique_ptr<listener>& listener ) {
    this->_segments[segnum]->_parse_raw_data( listener );
  }

  std::unique_ptr<channel>& tdmsfile::operator[](const std::string& key ) {
    return _channelmap.at( key );
  }

  std::unique_ptr<channel>& tdmsfile::find_or_make_channel(const std::string& key){
    if( 0 == _channelmap.count(key)){
      _channelmap.insert(std::make_pair(key, std::unique_ptr<channel>( new channel( key))));
    }
    return _channelmap.at(key);
  }

  tdmsfile::~tdmsfile( ) {
    fclose( f );
    free( segbuff );
  }

  channel::property::~property( ) {
    if ( value == nullptr ) {
      log::debug << "DOUBLE FREE" << log::endl;
      return;
    }
    if ( data_type.name == "tdsTypeString" ) {
      delete (std::string* ) value;
    }
    else {
      free( value );
    }
    value = nullptr;
  }

  void data_type_t::_init_default_array_reader( ) {
    auto read_to = this->read_to;
    auto ctype_length = this->ctype_length;
    read_array_to = [read_to, ctype_length](const unsigned char* source, void* target, size_t number_values ) {
      log::debug << "Doing iterative reading" << log::endl;
      for ( size_t i = 0; i < number_values; ++i ) {
        read_to( source + ( i * ctype_length ), (void*) ( ( (char*) target ) + ( i * ctype_length ) ) );
      }
    };
  }
}
