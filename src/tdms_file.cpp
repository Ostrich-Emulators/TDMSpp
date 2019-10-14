#include <iostream> // Debugging
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <map>

#include "tdms.hpp"
#include "log.hpp"
#include "tdms_impl.hpp"

namespace TDMS{
  typedef unsigned long long uulong;

  file::file( const std::string& filename ) : filename( filename ) {
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

  void file::_parse_segments( FILE * f ) {
    uulong offset = 0;
    segment* prev = nullptr;
    // First read the metadata of the segments
    size_t count = 0;
    while ( offset < file_contents_size - 8 * 4 ) {
      try {
        log::debug << "parsing segment " << ( count++ ) << " from offset: " << offset << log::endl;
        segment* s = new segment( offset, prev, this );
        offset += s->_next_segment_offset;
        _segments.push_back( s );
        prev = s;
      }
      catch ( segment::no_segment_error& e ) {
        // Last segment was parsed.
        break;
      }
    }
    //    for(auto obj: this->_objects)
    //    {
    //        obj.second->_initialise_data();
    //    }
    //    for(auto seg: this->_segments)
    //    {
    //        seg->_parse_raw_data();
    //    }
  }

  void file::loadSegment( size_t segnum, listener* listener ) {
    this->_segments[segnum]->_parse_raw_data( listener );
  }

  const channel* file::operator[](const std::string& key ) {
    return _objects.at( key );
  }

  file::~file( ) {
    fclose( f );
    for ( segment* _s : _segments )
      delete _s;
    for ( auto _o : _objects )
      delete _o.second;
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
