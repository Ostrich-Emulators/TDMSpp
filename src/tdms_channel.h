/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   channel.h
 * Author: ryan
 *
 * Created on August 1, 2020, 12:20 PM
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include "exports.h"
#include "data_type.h"
#include "datachunk.h"
#include <memory>

namespace TDMS {
  class tdmsfile;
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

      property( const data_type_t& dt, void* val ) : data_type( dt ), value( val ) { }
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
      return _data_type.name( );
    }

    size_t bytes( ) const {
      return _data_type.ctype_length( ) * _number_values;
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
}

#endif /* CHANNEL_H */

