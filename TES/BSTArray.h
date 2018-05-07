#pragma once

// using read only implementation for now since we don't need to manipulate arrays
// also exists in skse source as tArray
// https://raw.githubusercontent.com/Nukem9/SykrimSETest/master/skyrim64_test/src/patches/TES/BSTArray.h

#include <cstdint>

class BSTArrayHeapAllocator
{
	friend class __BSTArrayCheckOffsets;

private:
	void *m_Buffer;
	uint32_t m_AllocSize;

public:
	BSTArrayHeapAllocator() : m_Buffer(nullptr), m_AllocSize(0)
	{
	}

	void *QBuffer()
	{
		return m_Buffer;
	}

	uint32_t QAllocSize()
	{
		return m_AllocSize;
	}
};

class BSTArrayBase
{
	friend class __BSTArrayCheckOffsets;

private:
	uint32_t m_Size;

public:
	BSTArrayBase() : m_Size(0)
	{
	}

	uint32_t QSize()
	{
		return m_Size;
	}

	bool QEmpty()
	{
		return m_Size == 0;
	}
};

template <class _Ty, class _Alloc = BSTArrayHeapAllocator>
class BSTArray : public _Alloc, public BSTArrayBase
{
	friend class __BSTArrayCheckOffsets;

public:
	using value_type = _Ty;
	using allocator_type = _Alloc;
	using reference = _Ty & ;
	using const_reference = const _Ty&;
	using size_type = uint32_t;

	BSTArray()
	{
	}

	reference operator[](const size_type Pos)
	{
		return (this->_Myfirst()[Pos]);
	}

	const_reference operator[](const size_type Pos) const
	{
		return (this->_Myfirst()[Pos]);
	}

	reference at(const size_type Pos)
	{
		if (Pos < 0 || Pos >= QSize())
			throw std::out_of_range;

		return (this->_Myfirst()[Pos]);
	}

	const_reference at(const size_type Pos) const
	{
		if (Pos < 0 || Pos >= QSize())
			throw std::out_of_range;

		return (this->_Myfirst()[Pos]);
	}

	reference front()
	{
		return (*this->_Myfirst());
	}

	const_reference front() const
	{
		return (*this->_Myfirst());
	}

	reference back()
	{
		return (this->_Mylast()[-1]);
	}

	const_reference back() const
	{
		return (this->_Mylast()[-1]);
	}

private:
	_Ty * _Myfirst()
	{
		return (_Ty *)QBuffer();
	}

	_Ty *_Mylast()
	{
		return ((_Ty *)QBuffer()) + QSize();
	}
};

class __BSTArrayCheckOffsets
{
	static_assert(offsetof(BSTArrayHeapAllocator, m_Buffer) == 0x0, "");
	static_assert(offsetof(BSTArrayHeapAllocator, m_AllocSize) == 0x8, "");

	static_assert(offsetof(BSTArrayBase, m_Size) == 0x0, "");

	static_assert(offsetof(BSTArray<int>, m_Buffer) == 0x0, "");
	static_assert(offsetof(BSTArray<int>, m_AllocSize) == 0x8, "");
	static_assert(offsetof(BSTArray<int>, m_Size) == 0x10, "");
};