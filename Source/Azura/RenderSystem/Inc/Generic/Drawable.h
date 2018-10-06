﻿#pragma once
#include "Containers/Vector.h"
#include "GenericTypes.h"
#include "Types.h"
#include "Core/RawStorageFormat.h"

namespace Azura {
using DrawableID = U32;

class Shader;

struct DrawableCreateInfo {
  U32 m_vertexCount{0};
  U32 m_instanceCount{0};
  U32 m_indexCount{0};
  RawStorageFormat m_indexType{RawStorageFormat::R32_UINT};
};

struct DrawablePoolSlotInfo {
  U32 m_numVertexSlots{0};
  U32 m_numInstanceSlots{0};
};

class Drawable {
public:
  Drawable(const DrawableCreateInfo& info, U32 numVertexSlots, U32 numInstanceSlots, U32 numUniformSlots, Memory::Allocator& allocator);
  virtual ~Drawable() = default;

  Drawable(const Drawable& other) = delete;
  Drawable(Drawable&& other) noexcept = default;
  Drawable& operator=(const Drawable& other) = delete;
  Drawable& operator=(Drawable&& other) noexcept = delete;

  void AddVertexBufferInfo(BufferInfo&& info);
  void AddInstanceBufferInfo(BufferInfo&& info);
  void AddUniformBufferInfo(BufferInfo&& info);
  void SetIndexBufferInfo(BufferInfo&& info);

  U32 GetVertexCount() const;
  U32 GetIndexCount() const;
  U32 GetInstanceCount() const;
  RawStorageFormat GetIndexType() const;

  const Containers::Vector<BufferInfo>& GetVertexBufferInfos() const;
  const Containers::Vector<BufferInfo>& GetInstanceBufferInfos() const;
  const BufferInfo& GetIndexBufferInfo() const;

  virtual void Submit() = 0;

protected:
  Memory::Allocator& GetAllocator() const;

  Containers::Vector<BufferInfo> m_vertexBufferInfos;
  Containers::Vector<BufferInfo> m_instanceBufferInfos;
  Containers::Vector<BufferInfo> m_uniformBufferInfos;
  BufferInfo m_indexBufferInfo;
  DrawableCreateInfo m_createInfo;

private:
  // Shared as they are editable by APIs
  const U32 m_vertexCount;
  const U32 m_indexCount;
  const RawStorageFormat m_indexType;

  U32 m_instanceCount;

  std::reference_wrapper<Memory::Allocator> m_allocator;
};

struct UniformBufferDesc
{
  U32 m_size;
  U32 m_count;
};
  
struct SamplerDesc
{
  SamplerType m_type;
};

struct DrawablePoolCreateInfo {
  U32 m_byteSize{0};
  U32 m_numDrawables{0};
  U32 m_numShaders{0};
  DrawablePoolSlotInfo m_slotInfo{};
  DrawType m_drawType{DrawType::InstancedIndexed};
  Containers::Vector<std::pair<Slot, UniformBufferDesc>> m_uniformBuffers;
  Containers::Vector<std::pair<Slot, SamplerDesc>> m_samplers;

  ~DrawablePoolCreateInfo() = default;

  DrawablePoolCreateInfo(const DrawablePoolCreateInfo& other) = delete;
  DrawablePoolCreateInfo(DrawablePoolCreateInfo&& other) noexcept = default;
  DrawablePoolCreateInfo& operator=(const DrawablePoolCreateInfo& other) = delete;
  DrawablePoolCreateInfo& operator=(DrawablePoolCreateInfo&& other) noexcept = default;
  
  DrawablePoolCreateInfo(Memory::Allocator& alloc);
};

class DrawablePool {
public:
  explicit DrawablePool(const DrawablePoolCreateInfo& createInfo, Memory::Allocator& allocator);
  virtual ~DrawablePool() = default;

  DrawablePool(const DrawablePool& other) = delete;
  DrawablePool(DrawablePool&& other) noexcept = default;
  DrawablePool& operator=(const DrawablePool& other) = delete;
  DrawablePool& operator=(DrawablePool&& other) noexcept = default;

  virtual DrawableID CreateDrawable(const DrawableCreateInfo& createInfo) = 0;

  virtual void AddShader(const Shader& shader) = 0;

  virtual void AddBufferBinding(Slot slot, const Containers::Vector<RawStorageFormat>& strides) = 0;

  void BindVertexData(DrawableID drawableId, const Slot& slot, const Containers::Vector<U8>& buffer);
  virtual void BindVertexData(DrawableID drawableId, const Slot& slot, const U8* buffer, U32 size) = 0;

  void BindInstanceData(DrawableID drawableId, const Slot& slot, const Containers::Vector<U8>& buffer);
  virtual void BindInstanceData(DrawableID drawableId, const Slot& slot, const U8* buffer, U32 size) = 0;

  void BindUniformData(DrawableID drawableId, const Slot& slot, const Containers::Vector<U8>& buffer);
  virtual void BindUniformData(DrawableID drawableId, const Slot& slot, const U8* buffer, U32 size) = 0;

  void SetIndexData(DrawableID drawableId, const Containers::Vector<U8>& buffer);
  virtual void SetIndexData(DrawableID drawableId, const U8* buffer, U32 size) = 0;

  virtual void Submit() = 0;

  U32 GetSize() const;

protected:
  Memory::Allocator& GetAllocator() const;
  DrawType GetDrawType() const;

  U32 m_numVertexSlots;
  U32 m_numInstanceSlots;
  U32 m_numUniformSlots;
  U32 m_numSamplerSlots;

private:
  U32 m_byteSize;
  DrawType m_drawType;
 

  std::reference_wrapper<Memory::Allocator> m_allocator;
};

} // namespace Azura
