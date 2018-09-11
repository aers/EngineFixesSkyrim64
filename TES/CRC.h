#pragma once

#include "../skse64_common/Relocation.h"

template <class Ty>
class CRC32Calculator
{
public:
	inline CRC32Calculator() {}
	inline CRC32Calculator(const Ty &val) {
		operator=(val);
	}

	inline operator UInt32() {
		return m_checksum;
	}

protected:
	template <std::size_t SIZE = sizeof(Ty)>
	inline CRC32Calculator & operator=(const Ty &val) {
		typedef void(*Fn)(UInt32 *, const void *, UInt32);
		RelocAddr<Fn> fn(0x00C06680); // CalculateCRC32_Size

		fn(&m_checksum, &val, SIZE);
		return *this;
	}

	template <>
	inline CRC32Calculator & operator=<4>(const Ty &val) {
		typedef void(*Fn)(UInt32 *, Ty);
		RelocAddr<Fn> fn(0x00C066E0); // CalculateCRC32_32

		fn(&m_checksum, val);
		return *this;
	}

	template <>
	inline CRC32Calculator & operator=<8>(const Ty &val) {
		typedef void(*Fn)(UInt32 *, Ty);
		RelocAddr<Fn> fn(0x00C06760); // CalculateCRC32_64

		fn(&m_checksum, val);
		return *this;
	}

	UInt32	m_checksum;
};

template <class Ty>
inline UInt32 CalcCRC32(const Ty &val) {
	CRC32Calculator<Ty> crc(val);
	return crc;
}
