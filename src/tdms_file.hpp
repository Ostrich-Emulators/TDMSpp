#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>
#include <cstring>
#include <memory>
#include "log.hpp"

#include "exports.h"
#include "tdms_segment.hpp"

namespace TDMS {

  class segment;
  class datachunk;
  class channel;

  class TDMS_EXPORT tdmsfile {
    friend class segment;
  public:
    tdmsfile( const std::string& filename );
    tdmsfile& operator=(const tdmsfile&) = delete;
    tdmsfile( const tdmsfile& ) = delete;
    virtual ~tdmsfile( );

    channel * operator[](const std::string& key );
    channel * find_or_make_channel( const std::string& key );

    const size_t segments( ) const {
      return _segments.size( );
    }

    void loadSegment( size_t segnum, listener * );

    class iterator {
      friend class tdmsfile;
    public:

      channel * operator*( ) {
        return _it->second.get( );
      }

      const iterator& operator++( ) {
        ++_it;

        return *this;
      }

      bool operator!=(const iterator& other ) {
        return other._it != _it;
      }
    private:

      iterator( std::map<std::string, std::unique_ptr<channel>>::iterator it )
          : _it( it ) { }
      std::map<std::string, std::unique_ptr<channel>>::iterator _it;
    };

    iterator begin( ) {
      return iterator( _channelmap.begin( ) );
    }

    iterator end( ) {
      return iterator( _channelmap.end( ) );
    }

  private:
    void _parse_segments();

    size_t file_contents_size;
    std::vector<std::unique_ptr<segment>> _segments;
    std::string filename;
    FILE * f;

    std::map<std::string, std::unique_ptr<channel>> _channelmap;

    // a memory buffer for loading segment data
    std::vector<unsigned char> segbuff;
  };
}
