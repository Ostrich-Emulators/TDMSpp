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

  class tdmsfile {
    friend class segment;
  public:
      TDMS_EXPORT tdmsfile( const std::string& filename );
      TDMS_EXPORT tdmsfile& operator=(const tdmsfile&) = delete;
      TDMS_EXPORT tdmsfile( const tdmsfile& ) = delete;
      TDMS_EXPORT virtual ~tdmsfile( );

      TDMS_EXPORT channel * operator[](const std::string& key );
      TDMS_EXPORT channel *  find_or_make_channel( const std::string& key );

      TDMS_EXPORT const size_t segments( ) const {
      return _segments.size( );
    }

      TDMS_EXPORT void loadSegment( size_t segnum, listener * );

    class iterator {
      friend class tdmsfile;
    public:

        TDMS_EXPORT channel * operator*( ) {
        return _it->second.get( );
      }

        TDMS_EXPORT const iterator& operator++( ) {
        ++_it;

        return *this;
      }

        TDMS_EXPORT bool operator!=(const iterator& other ) {
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
