/*!
	Base64 implementation.
*/

#pragma once

#include <string>
#include <array>
#include <exception>
#include <iostream> // std::cout, debug
#include <algorithm>

namespace restinio
{

namespace impl
{

namespace sha1
{

// Block size in bytes.
const std::uint8_t BLOCK_SIZE = 64;

// Number of 32bit integers in block.
const std::uint8_t BLOCK_INTS = 16;

// Word size in digest in bytes.
const std::uint8_t WORD_SIZE = 4;

// Digest size in bytes.
const std::uint8_t DIGEST_SIZE = 20;

// Number of words in digest.
const std::size_t DIGEST_ARRAY_SIZE = DIGEST_SIZE / WORD_SIZE ;

using byte_block_t = std::array< std::uint8_t, BLOCK_SIZE >;
using int_block_t = std::array< std::uint32_t, BLOCK_INTS >;
using digest_t = std::array< std::uint32_t, DIGEST_ARRAY_SIZE >;

template< class T >
inline std::uint8_t
as_uint8( T what )
{
	return static_cast< std::uint8_t >( what );
}

template< class T >
inline std::uint32_t
as_uint32( T what )
{
	return static_cast< std::uint32_t >( what );
}

template< class T >
const std::uint8_t *
as_uint8_ptr( const T * what )
{
	return reinterpret_cast< const std::uint8_t * >( what );
}

template< unsigned int SHIFT >
std::uint32_t
rotate_left( const std::uint32_t x )
{
	return (x << SHIFT) | (x >> (32 - SHIFT));
}

template< unsigned int SHIFT >
inline std::uint8_t
rshift_then_extract( std::uint32_t x )
{
	return static_cast<std::uint8_t>( (x >> SHIFT) & 0xffu );
}

static uint32_t blk(const int_block_t & block, const size_t i)
{
	return rotate_left<1>(
		block[(i+13)&15] ^ block[(i+8)&15] ^ block[(i+2)&15] ^ block[i] );
}

inline void
R0(const int_block_t & block, const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i)
{
	z += ((w&(x^y))^y) + block[i] + 0x5a827999 + rotate_left<5>( v );
	w = rotate_left<30>( w );
}


inline void
R1(int_block_t & block, const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i)
{
	block[i] = blk(block, i);
	z += ((w&(x^y))^y) + block[i] + 0x5a827999 + rotate_left<5>(v);
	w = rotate_left<30>(w);
}


inline void
R2(int_block_t & block, const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i)
{
	block[i] = blk(block, i);
	z += (w^x^y) + block[i] + 0x6ed9eba1 + rotate_left<5>(v);
	w = rotate_left<30>(w);
}

inline void
R3(int_block_t & block, const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i)
{
	block[i] = blk(block, i);
	z += (((w|x)&y)|(w&x)) + block[i] + 0x8f1bbcdc + rotate_left<5>(v);
	w = rotate_left<30>(w);
}


inline void
R4(int_block_t & block, const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i)
{
	block[i] = blk(block, i);
	z += (w^x^y) + block[i] + 0xca62c1d6 + rotate_left<5>(v);
	w = rotate_left<30>(w);
}

inline void
transform( digest_t & digest, const byte_block_t & buf )
{
	int_block_t block;

	for (size_t i = 0; i < BLOCK_INTS; i++)
	{
		block[i] =
			as_uint32(buf[4*i+3] & 0xff) |
			as_uint32(buf[4*i+2] & 0xff)<<8 |
			as_uint32(buf[4*i+1] & 0xff)<<16 |
			as_uint32(buf[4*i+0] & 0xff)<<24;
	}

	std::uint32_t a = digest[0];
	std::uint32_t b = digest[1];
	std::uint32_t c = digest[2];
	std::uint32_t d = digest[3];
	std::uint32_t e = digest[4];

	R0(block, a, b, c, d, e,  0);
	R0(block, e, a, b, c, d,  1);
	R0(block, d, e, a, b, c,  2);
	R0(block, c, d, e, a, b,  3);
	R0(block, b, c, d, e, a,  4);
	R0(block, a, b, c, d, e,  5);
	R0(block, e, a, b, c, d,  6);
	R0(block, d, e, a, b, c,  7);
	R0(block, c, d, e, a, b,  8);
	R0(block, b, c, d, e, a,  9);
	R0(block, a, b, c, d, e, 10);
	R0(block, e, a, b, c, d, 11);
	R0(block, d, e, a, b, c, 12);
	R0(block, c, d, e, a, b, 13);
	R0(block, b, c, d, e, a, 14);
	R0(block, a, b, c, d, e, 15);
	R1(block, e, a, b, c, d,  0);
	R1(block, d, e, a, b, c,  1);
	R1(block, c, d, e, a, b,  2);
	R1(block, b, c, d, e, a,  3);
	R2(block, a, b, c, d, e,  4);
	R2(block, e, a, b, c, d,  5);
	R2(block, d, e, a, b, c,  6);
	R2(block, c, d, e, a, b,  7);
	R2(block, b, c, d, e, a,  8);
	R2(block, a, b, c, d, e,  9);
	R2(block, e, a, b, c, d, 10);
	R2(block, d, e, a, b, c, 11);
	R2(block, c, d, e, a, b, 12);
	R2(block, b, c, d, e, a, 13);
	R2(block, a, b, c, d, e, 14);
	R2(block, e, a, b, c, d, 15);
	R2(block, d, e, a, b, c,  0);
	R2(block, c, d, e, a, b,  1);
	R2(block, b, c, d, e, a,  2);
	R2(block, a, b, c, d, e,  3);
	R2(block, e, a, b, c, d,  4);
	R2(block, d, e, a, b, c,  5);
	R2(block, c, d, e, a, b,  6);
	R2(block, b, c, d, e, a,  7);
	R3(block, a, b, c, d, e,  8);
	R3(block, e, a, b, c, d,  9);
	R3(block, d, e, a, b, c, 10);
	R3(block, c, d, e, a, b, 11);
	R3(block, b, c, d, e, a, 12);
	R3(block, a, b, c, d, e, 13);
	R3(block, e, a, b, c, d, 14);
	R3(block, d, e, a, b, c, 15);
	R3(block, c, d, e, a, b,  0);
	R3(block, b, c, d, e, a,  1);
	R3(block, a, b, c, d, e,  2);
	R3(block, e, a, b, c, d,  3);
	R3(block, d, e, a, b, c,  4);
	R3(block, c, d, e, a, b,  5);
	R3(block, b, c, d, e, a,  6);
	R3(block, a, b, c, d, e,  7);
	R3(block, e, a, b, c, d,  8);
	R3(block, d, e, a, b, c,  9);
	R3(block, c, d, e, a, b, 10);
	R3(block, b, c, d, e, a, 11);
	R4(block, a, b, c, d, e, 12);
	R4(block, e, a, b, c, d, 13);
	R4(block, d, e, a, b, c, 14);
	R4(block, c, d, e, a, b, 15);
	R4(block, b, c, d, e, a,  0);
	R4(block, a, b, c, d, e,  1);
	R4(block, e, a, b, c, d,  2);
	R4(block, d, e, a, b, c,  3);
	R4(block, c, d, e, a, b,  4);
	R4(block, b, c, d, e, a,  5);
	R4(block, a, b, c, d, e,  6);
	R4(block, e, a, b, c, d,  7);
	R4(block, d, e, a, b, c,  8);
	R4(block, c, d, e, a, b,  9);
	R4(block, b, c, d, e, a, 10);
	R4(block, a, b, c, d, e, 11);
	R4(block, e, a, b, c, d, 12);
	R4(block, d, e, a, b, c, 13);
	R4(block, c, d, e, a, b, 14);
	R4(block, b, c, d, e, a, 15);

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;
}

struct builder_t
{
	public:

		builder_t()
		{
			reset();
		}

		~builder_t()
		{
			/* Zeroize sensitive information. */
			reset();
		}

		inline builder_t &
		update( const std::uint8_t * what, std::size_t length )
		{
			while( true )
			{
				auto part_len = std::min( length, BLOCK_SIZE - m_buffer_len );

				std::copy( what, what + part_len, m_buffer.begin() + m_buffer_len );
				m_buffer_len += part_len;

				if( m_buffer_len != BLOCK_SIZE )
					break;
				else
				{
					length -= part_len;
					what += part_len;

					transform( m_digest, m_buffer );
					m_transforms_count++;
					m_buffer_len = 0;
				}
			}

			return *this;
		}

		digest_t
		finish()
		{
			const auto total_bits = calculate_total_bits_count();

			const auto original_buf_len = m_buffer_len;

			m_buffer[ m_buffer_len ++ ] = 0x80;

			while( m_buffer_len < BLOCK_SIZE )
				m_buffer[m_buffer_len++] = 0x00;

			if( original_buf_len > BLOCK_SIZE - 8 )
			{
				transform( m_digest, m_buffer );

				for( size_t i = 0 ; i < BLOCK_SIZE ; ++ i )
					m_buffer[i] = 0;
			}

			// Fill total bits count in last 8 bytes of buffer as big-endian.
			std::size_t i = BLOCK_SIZE - 8u;
			const auto push_uint_to_buffer = [&]( auto big_value ) {
				const auto v = static_cast<std::uint32_t>(big_value);
				m_buffer[ i++ ] = rshift_then_extract<24>(v);
				m_buffer[ i++ ] = rshift_then_extract<16>(v);
				m_buffer[ i++ ] = rshift_then_extract<8>(v);
				m_buffer[ i++ ] = rshift_then_extract<0>(v);
			};
			push_uint_to_buffer( total_bits >> 32 );
			push_uint_to_buffer( total_bits & 0xffffffffu );

			transform( m_digest, m_buffer );

			return m_digest;
		}

	private:

		std::uint_fast64_t
		calculate_total_bits_count() const
		{
			return (static_cast<std::uint_fast64_t>(m_transforms_count)
					* BLOCK_SIZE + m_buffer_len) * 8;
		}

		void
		store_total_bits_to_buffer()
		{
		}

		void
		reset()
		{
			m_buffer_len = 0;
			m_transforms_count = 0;

			m_digest[0] = 0x67452301;
			m_digest[1] = 0xEFCDAB89;
			m_digest[2] = 0x98BADCFE;
			m_digest[3] = 0x10325476;
			m_digest[4] = 0xC3D2E1F0;

			std::fill( std::begin(m_buffer), std::end(m_buffer), 0 );
		}

		digest_t m_digest;

		size_t m_buffer_len;
		size_t m_transforms_count;

		byte_block_t m_buffer;
};

inline std::string
to_hex_string( const digest_t & what )
{
	static const char digits[] = "0123456789abcdef";

	std::string result;
	result.reserve( DIGEST_ARRAY_SIZE * 8);

	for( const auto c : what )
	{
		result += digits[(c >> 28) & 0xF];
		result += digits[(c >> 24) & 0xF];
		result += digits[(c >> 20) & 0xF];
		result += digits[(c >> 16) & 0xF];
		result += digits[(c >> 12) & 0xF];
		result += digits[(c >> 8) & 0xF];
		result += digits[(c >> 4) & 0xF];
		result += digits[c & 0xF];
	}

	return result;
}



inline digest_t
make_digest( const std::uint8_t * what, std::size_t length )
{
	return builder_t{}.update( what, length ).finish();
}

template< class T >
inline digest_t
make_digest( const T * begin, const T * end )
{
	const std::uint8_t * const start = as_uint8_ptr( begin );
	const std::uint8_t * const finish = as_uint8_ptr( end );
	const auto length = static_cast< std::size_t >( finish - start );

	return make_digest( start, length );
}

inline digest_t
make_digest( const char * what, std::size_t length )
{
	return builder_t{}.update(
		as_uint8_ptr( what ), length ).finish();
}

inline digest_t
make_digest( const std::string & str )
{
	return make_digest( str.data(), str.size() );
}

} /* namespace sha1 */

} /* namespace impl */

} /* namespace restinio */
