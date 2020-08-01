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

namespace TDMS {

	class segment;
	class datachunk;

	class TDMS_EXPORT data_type_t {
	public:
		typedef std::function<void* ( ) > parse_t;

		data_type_t( const data_type_t& dt )
		: name( dt.name ),
		read_to( dt.read_to ),
		read_array_to( dt.read_array_to ),
		length( dt.length ),
		ctype_length( dt.ctype_length ) {
		}

		data_type_t( )
		: name( "INVALID TYPE" ),
		length( 0 ),
		ctype_length( 0 ) {
			_init_default_array_reader( );
		}

		data_type_t( const std::string& _name,
				const size_t _len,
				std::function<void (const unsigned char*, void*) > reader )
		: name( _name ),
		read_to( reader ),
		length( _len ),
		ctype_length( _len ) {
			_init_default_array_reader( );
		}

		data_type_t( const std::string& _name,
				const size_t _len,
				std::function<void (const unsigned char*, void*) > reader,
				std::function<void (const unsigned char*, void*, size_t ) > array_reader )
		: name( _name ),
		read_to( reader ),
		read_array_to( array_reader ),
		length( _len ),
		ctype_length( _len ) {
		}

		data_type_t( const std::string& _name,
				const size_t _len,
				const size_t _ctype_len,
				std::function<void (const unsigned char*, void*) > reader )
		: name( _name ),
		read_to( reader ),
		length( _len ),
		ctype_length( _ctype_len ) {
			_init_default_array_reader( );
		}

		bool is_valid( ) const {
			return (name != "INVALID TYPE" );
		}

		bool operator==(const data_type_t& dt ) const {
			return (name == dt.name );
		}

		bool operator!=(const data_type_t& dt ) const {
			return !( *this == dt );
		}

		std::string name;

		void* read( const unsigned char* data ) {
			void* d = malloc( ctype_length );
			read_to( data, d );
			return d;
		}
		std::function<void (const unsigned char*, void*) > read_to;
		std::function<void (const unsigned char*, void*, size_t ) > read_array_to;
		size_t length;
		size_t ctype_length;

		static const std::map<uint32_t, const data_type_t> _tds_datatypes;
	private:
		void _init_default_array_reader( );
	};

	class TDMS_EXPORT channel {
		friend class tdmsfile;
		friend class segment;
		friend class datachunk;
	public:
		channel( const std::string& path );
    channel( const channel& ) = delete;
    channel& operator=( const channel& ) = delete;
		~channel( );

		struct property {

			property( const data_type_t& dt, void* val )
			: data_type( dt ),
			value( val ) {
			}
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

		const std::string data_type( ) {
			return _data_type.name;
		}

		size_t bytes( ) {
			return _data_type.ctype_length * _number_values;
		}

		size_t number_values( ) {
			return _number_values;
		}

		const std::string get_path( ) {
			return _path;
		}

		const std::map<std::string, std::shared_ptr<property>> get_properties( ) {
			return _properties;
		}
	private:
		std::unique_ptr<datachunk> _previous_segment_chunk;

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
    tdmsfile& operator=( const tdmsfile& ) = delete;
    tdmsfile( const tdmsfile& ) = delete;
		virtual ~tdmsfile( );

		std::unique_ptr<channel>& operator[](const std::string& key );
		std::unique_ptr<channel>& find_or_make_channel( const std::string& key );

		const size_t segments( ) const {
			return _segments.size( );
		}

		void loadSegment( size_t segnum, std::unique_ptr<listener>& );

		class iterator {
			friend class tdmsfile;
		public:

			std::unique_ptr<channel>& operator*( ) {
				return _it->second;
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
			: _it( it ) {
			}
			std::map<std::string, std::unique_ptr<channel>>::iterator _it;
		};

		iterator begin( ) {
			return iterator( _channelmap.begin( ) );
		}

		iterator end( ) {
			return iterator( _channelmap.end( ) );
		}

	private:
		void _parse_segments( FILE * );

		size_t file_contents_size;
		std::vector<std::unique_ptr<segment>> _segments;
		std::string filename;
		FILE * f;

		std::unique_ptr<segment> nosegment;

		std::map<std::string, std::unique_ptr<channel>> _channelmap;

		// a memory buffer for loading segment data
		unsigned char * segbuff;
	};
}
