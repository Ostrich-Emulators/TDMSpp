#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "data_type.h"
#include "exports.h"
#include "datachunk.h"

namespace TDMS {

  typedef unsigned long long uulong;

  class tdmsfile;
  class listener;

  class segment {
    friend class tdmsfile;
    friend class datachunk;

  public:
    TDMS_EXPORT segment( uulong segment_start, segment * previous_segment, tdmsfile * file );
    TDMS_EXPORT virtual ~segment( );
  private:

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
    std::vector<datachunk> _ordered_chunks;

    tdmsfile * _parent_file;

    static const std::map<const std::string, int32_t> _toc_properties;
  };
}
