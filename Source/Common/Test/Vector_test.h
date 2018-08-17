#pragma once
#include "gtest/gtest.h"
#include "Memory/HeapMemoryBuffer.h"
#include "Memory/MonotonicAllocator.h"
#include "Containers/Vector.h"

using namespace Azura::Containers;
using namespace Azura::Memory;

class VectorTest : public ::testing::Test {
 protected:

  VectorTest() : buffer(Azura::Memory::HeapMemoryBuffer(1024)), alloc(Azura::Memory::MonotonicAllocator(buffer, 1024)) {
  }

  virtual ~VectorTest() {
  }

  Azura::Memory::HeapMemoryBuffer buffer;
  Azura::Memory::MonotonicAllocator alloc;
};
