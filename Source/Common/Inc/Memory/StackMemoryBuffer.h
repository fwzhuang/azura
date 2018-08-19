#pragma once

#include <cassert>

#include "Types.h"
#include "MemoryBuffer.h"
#include "Utils/Macros.h"

namespace Azura {
namespace Memory {

template<U32 Size>
class StackMemoryBuffer final : public MemoryBuffer {
  using Byte = U8;

public:
  StackMemoryBuffer();
  ~StackMemoryBuffer() = default;

  // Disabled Copy Ctor
  StackMemoryBuffer(const StackMemoryBuffer& other) = delete;
  // Disabled Copy Assignment
  StackMemoryBuffer& operator=(const StackMemoryBuffer& other) = delete;

  StackMemoryBuffer(StackMemoryBuffer&& other) noexcept = default;
  StackMemoryBuffer& operator=(StackMemoryBuffer&& other) noexcept = default;

  void* Allocate(SizeType size) override;
  void Deallocate(void* address) override;

private:
  UPTR AllocateRaw(SizeType size);
  U8 m_buffer[Size];
};

template <U32 Size>
StackMemoryBuffer<Size>::StackMemoryBuffer()
  : MemoryBuffer(Size, reinterpret_cast<AddressPtr>(m_buffer)),
    m_buffer{} {
}

template <U32 Size>
void* StackMemoryBuffer<Size>::Allocate(SizeType size) {
  const AddressPtr addr = AllocateRaw(size);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<void*>(addr);
}

template <U32 Size>
void StackMemoryBuffer<Size>::Deallocate(void* address) {
  UNUSED(address);
}

template <U32 Size>
UPTR StackMemoryBuffer<Size>::AllocateRaw(SizeType size) {
  // Available < Size, Can't allocate
  if (mSize - (m_currentPosition - m_memoryBlock) < size)
  {
    return 0u;
  }

  const AddressPtr addr = m_currentPosition;
  m_currentPosition += size;

  return addr;
}
} // namespace Memory
} // namespace Azura
