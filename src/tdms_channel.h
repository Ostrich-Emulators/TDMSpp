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
  
  class channel {
    friend class tdmsfile;
    friend class segment;
    friend class datachunk;
  public:
      TDMS_EXPORT channel( const std::string& path );
      TDMS_EXPORT channel( const channel& ) = delete;
      TDMS_EXPORT channel& operator=(const channel&) = delete;
      TDMS_EXPORT virtual ~channel( );

    struct property {

      TDMS_EXPORT property( const data_type_t& dt, void* val ) : data_type( dt ), value( val ) { }
      TDMS_EXPORT property( const property& p ) = delete;
      TDMS_EXPORT virtual ~property( );

      TDMS_EXPORT double asDouble( ) const {
        return *( (double*) value );
      }

      TDMS_EXPORT int asInt( ) const {
        return *( (int*) value );
      }

      TDMS_EXPORT const std::string& asString( ) const {
        return *( ( std::string* ) value );
      }

      TDMS_EXPORT time_t asUTCTimestamp( ) const {
        return *( (time_t*) value );
      }

      const data_type_t data_type;
      void* value;
    };

    TDMS_EXPORT std::string data_type( ) const {
      return _data_type.name( );
    }

    TDMS_EXPORT size_t bytes( ) const {
      return _data_type.ctype_length( ) * _number_values;
    }

    TDMS_EXPORT size_t number_values( ) const {
      return _number_values;
    }

    TDMS_EXPORT const std::string& get_path( ) const {
      return _path;
    }

    TDMS_EXPORT std::map<std::string, std::shared_ptr<property>> get_properties( ) const {
      return _properties;
    }

    TDMS_EXPORT bool has_previous( ) const {
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

