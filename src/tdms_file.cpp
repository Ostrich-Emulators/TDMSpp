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
    f = fopen( filename.c_str( ), "rb" );
    if ( !f ) {
      throw std::runtime_error( "File \"" + filename + "\" could not be opened" );
    }
    fseek( f, 0, SEEK_END );
    file_contents_size = ftell( f );
    fseek( f, 0, SEEK_SET );

    // Now parse the segments
    _parse_segments();
    file_contents_size = 0;
  }

  void tdmsfile::_parse_segments() {
    uulong offset = 0;
    size_t maxsegmentsize = 0;
    // First read the metadata of the segments
    while ( offset < file_contents_size - 8 * 4 ) {
      try {
        auto prev = ( _segments.empty( )
            ? nullptr
            : _segments[_segments.size( ) - 1].get() );

        log::debug( ) << "parsing segment " << ( _segments.size( ) + 1 ) << " from offset: " << offset << std::endl;
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
    segbuff.reserve( maxsegmentsize );
  }

  void tdmsfile::loadSegment( size_t segnum, listener * listener ) {
    this->_segments[segnum]->_parse_raw_data( listener );
  }

  channel * tdmsfile::operator[](const std::string& key ) {
    return _channelmap.at( key ).get( );
  }

  channel * tdmsfile::find_or_make_channel( const std::string& key ) {
    if ( 0 == _channelmap.count( key ) ) {
      _channelmap.insert( std::make_pair( key, std::make_unique<channel>( key ) ) );
    }
    return _channelmap.at( key ).get( );
  }

  tdmsfile::~tdmsfile( ) {
    fclose( f );
  }

  channel::channel( const std::string& path ) : _path( path ), _has_data( false ),
      _data_start( 0 ), _number_values( 0 ) { }

  channel::~channel( ) { };

  channel::property::~property( ) {
    if ( value == nullptr ) {
      log::debug( ) << "DOUBLE FREE" << std::endl;
      return;
    }
    if ( data_type.is_string() ) {
      delete (std::string* ) value;
    }
    else {
      free( value );
    }
    value = nullptr;
  }
}
