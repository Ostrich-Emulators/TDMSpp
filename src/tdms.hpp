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
#include "tdms_impl.hpp"

namespace TDMS {

  class segment;
  class datachunk;

  class TDMS_EXPORT channel {
    friend class tdmsfile;
    friend class segment;
    friend class datachunk;
  public:
    channel( const std::string& path );
    channel( const channel& ) = delete;
    channel& operator=(const channel&) = delete;
    ~channel( );

    struct property {

      property( const data_type_t& dt, void* val )
          : data_type( dt ),
          value( val ) { }
      property( const property& p ) = delete;
      const data_type_t data_type;
      void* value;
      virtual ~property( );

      double asDouble( ) const {
        return *( (double*) value );
      }

      int asInt( ) const {
        return *( (int*) value );
      }

      const std::string& asString( ) const {
        return *( ( std::string* ) value );
      }

      time_t asUTCTimestamp( ) const {
        return *( (time_t*) value );
      }
    };

    const std::string data_type( ) const {
      return _data_type.name();
    }

    size_t bytes( ) const {
      return _data_type.ctype_length() * _number_values;
    }

    size_t number_values( ) const {
      return _number_values;
    }

    const std::string& get_path( ) const {
      return _path;
    }

    std::map<std::string, std::shared_ptr<property>> get_properties( ) const {
      return _properties;
    }

    bool has_previous( ) const {
      return ( nullptr != _previous_segment_chunk._tdms_channel );
    }
  private:
    datachunk _previous_segment_chunk;

    const std::string _path;
    bool _has_data;

    data_type_t _data_type;

    uint64_t _data_start;

    std::map<std::string, std::shared_ptr<property>> _properties;

    size_t _number_values;
  };

  class listener {
  public:
    virtual void data( const std::string& channelname, const unsigned char* rawdata,
        data_type_t, size_t num_vals ) = 0;
  };

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
