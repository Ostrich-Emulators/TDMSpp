#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>
#include <memory>

#if defined(_WIN32) || defined(_Win64)
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

namespace TDMS {

  typedef unsigned long long uulong;

  class tdmsfile;
  class channel;
  class datachunk;
  class listener;

  enum endianness {
    BIG,
    LITTLE
  };

  DllExport class segment {
    friend class tdmsfile;
    friend class channel;
    friend class datachunk;
  public:
    virtual ~segment( );

  private:

    class no_segment_error : public std::runtime_error {
    public:

      no_segment_error( ) : std::runtime_error( "Not a segment" ) { }
    };

    class read_error : public std::runtime_error {
    public:

      read_error( ) : std::runtime_error( "Segment read error" ) { }
    };

    segment( uulong segment_start, segment * previous_segment, tdmsfile* file );

    void _parse_metadata( const unsigned char* data, segment * previous_segment );
    void _parse_raw_data( listener * );
    void _calculate_chunks( );

    // Probably a map using enums performs faster.
    // Will only give a little performance though.
    // Perhaps use a struct for the _toc, so we don't need
    // the std::map and don't have to do lookups.
    std::map<std::string, bool> _toc;
    size_t _next_segment_offset;
    size_t _num_chunks;
    uulong _startpos_in_file;
    uint64_t _data_offset; // bytes of data between _startpos and the raw data
    std::vector<std::unique_ptr<datachunk>> _ordered_chunks;

    tdmsfile* _parent_file;

    static const std::map<const std::string, int32_t> _toc_properties;
  };

  DllExport class datachunk {
    friend class segment;
    friend class channel;
  public:
    datachunk( const datachunk& o );

    virtual ~datachunk( ) { }
  private:
    datachunk( channel * o );
    const unsigned char* _parse_metadata( const unsigned char* data );
    size_t _read_values( const unsigned char*& data, endianness e, listener * );

    channel * _tdms_channel;
    uint64_t _number_values;
    uint64_t _data_size;
    bool _has_data;
    uint32_t _dimension;
    data_type_t _data_type;
  };
}
