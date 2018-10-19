#include "D3D12/D3D12DrawablePool.h"

#include "D3D12/d3dx12.h"
#include "D3D12/D3D12Macros.h"
#include "Memory/MemoryFactory.h"
#include "Memory/MonotonicAllocator.h"
#include "D3D12/D3D12ScopedImage.h"
#include <algorithm>

using namespace Microsoft::WRL;    // NOLINT
using namespace Azura::Containers; // NOLINT

namespace Azura {
namespace D3D12 {

D3D12DrawablePool::D3D12DrawablePool(const ComPtr<ID3D12Device>& device,
                                     const DrawablePoolCreateInfo& createInfo,
                                     const DescriptorCount& descriptorCount,
                                     const Vector<DescriptorSlot>& descriptorSlots,
                                     const Vector<D3D12ScopedShader>& shaders,
                                     const Vector<D3D12ScopedRenderPass>& renderPasses,
                                     Memory::Allocator& mainAllocator,
                                     Memory::Allocator& initAllocator,
                                     Log log)
  : DrawablePool(createInfo, descriptorCount, mainAllocator),
    log_D3D12RenderSystem(std::move(log)),
    m_device(device),
    m_globalDescriptorSlots(descriptorSlots),
    m_shaders(shaders),
    m_pipelines(mainAllocator),
    m_drawables(createInfo.m_numDrawables, mainAllocator),
    m_renderPasses(createInfo.m_renderPasses.GetSize(), mainAllocator),
    m_pipelineFactory(initAllocator, log_D3D12RenderSystem),
    m_descriptorsPerDrawable(0),
    m_images(mainAllocator),
    m_samplers(mainAllocator),
    m_secondaryCommandBuffers(createInfo.m_renderPasses.GetSize(), mainAllocator),
    m_allHeaps(mainAllocator) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "Creating D3D12 Drawable Pool");
  
  CreateRenderPassReferences(createInfo, renderPasses);
  CreateInputAttributes(createInfo);
  CreateDescriptorHeap(createInfo);

  // Create Buffer
  m_stagingBuffer.Create(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), createInfo.m_byteSize,
                         D3D12_RESOURCE_STATE_GENERIC_READ, log_D3D12RenderSystem);
  m_stagingBuffer.Map();
  m_mainBuffer.Create(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), createInfo.m_byteSize,
                      D3D12_RESOURCE_STATE_COPY_DEST, log_D3D12RenderSystem);
}

DrawableID D3D12DrawablePool::CreateDrawable(const DrawableCreateInfo& createInfo) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "Creating a D3D12 Drawable");

  D3D12Drawable drawable = D3D12Drawable(createInfo, m_numVertexSlots, m_numInstanceSlots,
                                         m_descriptorCount.m_numUniformSlots, GetAllocator());
  m_drawables.PushBack(std::move(drawable));
  return m_drawables.GetSize() - 1;
}

void D3D12DrawablePool::BindVertexData(DrawableID drawableId, SlotID slot, const U8* buffer, U32 size) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
    "D3D12 Drawable Pool: Binding Vertex Requested for Drawable: %d for Slot: %d of Size: %d bytes", drawableId, slot,
    size);

  assert(m_drawables.GetSize() > drawableId);

  auto& drawable = m_drawables[drawableId];

  // TODO(vasumahesh1):[INPUT]: Could be an issue with sizeof(float)
  const U32 offset = m_stagingBuffer.AppendData(buffer, size, sizeof(float), log_D3D12RenderSystem);

  BufferInfo info    = BufferInfo();
  info.m_maxByteSize = size;
  info.m_byteSize    = size;
  info.m_offset      = offset;
  info.m_binding     = slot;

  drawable.AddVertexBufferInfo(std::move(info));
}

void D3D12DrawablePool::BindInstanceData(DrawableID drawableId, SlotID slot, const U8* buffer, U32 size) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
    "D3D12 Drawable Pool: Binding Instance Requested for Drawable: %d for Slot: %d of Size: %d bytes", drawableId, slot,
    size);

  assert(m_drawables.GetSize() > drawableId);

  auto& drawable = m_drawables[drawableId];

  // TODO(vasumahesh1):[INPUT]: Could be an issue with sizeof(float)
  const U32 offset = m_stagingBuffer.AppendData(buffer, size, sizeof(float), log_D3D12RenderSystem);

  BufferInfo info    = BufferInfo();
  info.m_maxByteSize = size;
  info.m_byteSize    = size;
  info.m_offset      = offset;
  info.m_binding     = slot;

  drawable.AddInstanceBufferInfo(std::move(info));
}

void D3D12DrawablePool::BindUniformData(DrawableID drawableId, SlotID slot, const U8* buffer, U32 size) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
    "D3D12 Drawable Pool: Binding Uniform Requested for Drawable: %d for Slot: %d of Size: %d bytes", drawableId, slot,
    size);

  assert(m_drawables.GetSize() > drawableId);

  auto& drawable = m_drawables[drawableId];

  size = (size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(
           D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

  // TODO(vasumahesh1):[INPUT]: Could be an issue with sizeof(float)
  const U32 offset = m_stagingBuffer.AppendData(buffer, size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                                                log_D3D12RenderSystem);

  const auto& descriptorSlot = m_globalDescriptorSlots[slot];

  UniformBufferInfo info = UniformBufferInfo();
  info.m_byteSize        = size;
  info.m_offset          = offset;
  info.m_binding         = descriptorSlot.m_bindIdx;
  info.m_set             = descriptorSlot.m_setIdx;

  drawable.AddUniformBufferInfo(std::move(info));
}

void D3D12DrawablePool::SetIndexData(DrawableID drawableId, const U8* buffer, U32 size) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
    "D3D12 Drawable Pool: Binding Index Requested for Drawable: %d of Size: %d bytes", drawableId, size);

  auto& drawable = m_drawables[drawableId];

  const U32 offset = m_stagingBuffer.AppendData(buffer, size, sizeof(U32), log_D3D12RenderSystem);

  BufferInfo info = BufferInfo();
  info.m_byteSize = size;
  info.m_offset   = offset;

  drawable.SetIndexBufferInfo(std::move(info));
}

void D3D12DrawablePool::AddShader(U32 shaderId) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Binding Shader Requested, ID: %d", shaderId);
  m_pipelineFactory.AddShaderStage(m_shaders[shaderId]);
}

void D3D12DrawablePool::BindTextureData(SlotID slot, const TextureDesc& desc, const U8* buffer) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
    "D3D12 Drawable Pool: Binding Texture Requested for Slot: %d of Size: %d bytes", slot, desc.m_size);

  const U32 size = desc.m_size;

  const auto& descriptorSlot = m_globalDescriptorSlots[slot];

  // TODO(vasumahesh1):[INPUT]: Could be an issue with sizeof(float)
  const U32 offset = m_stagingBuffer.AppendData(buffer, size, sizeof(float), log_D3D12RenderSystem);

  TextureBufferInfo info = TextureBufferInfo();
  info.m_byteSize        = size;
  info.m_offset          = offset;
  info.m_desc            = desc;
  info.m_binding         = descriptorSlot.m_bindIdx;
  info.m_set             = descriptorSlot.m_setIdx;

  m_textureBufferInfos.PushBack(info);
}

void D3D12DrawablePool::BindSampler(SlotID slot, const SamplerDesc& desc) {
  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Binding Sampler Requested for Slot: %d", slot);
  UNUSED(desc);

  const auto& descriptorSlot = m_globalDescriptorSlots[slot];
  assert(descriptorSlot.m_type == DescriptorType::Sampler);

  SamplerInfo sInfo = {};
  sInfo.m_set       = descriptorSlot.m_setIdx;
  sInfo.m_binding   = descriptorSlot.m_bindIdx;

  m_samplerInfos.PushBack(sInfo);
}

void D3D12DrawablePool::SetTextureData(ID3D12GraphicsCommandList* oneTimeCommandList) {
  if (m_textureBufferInfos.GetSize() == 0) {
    return;
  }

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Texture Data Found");

  const CD3DX12_CPU_DESCRIPTOR_HANDLE textureCPUHandle(m_descriptorDrawableHeap->GetCPUDescriptorHandleForHeapStart());

  m_images.Reserve(m_textureBufferInfos.GetSize());

  U32 idx = 0;
  for (const auto& textBufInfo : m_textureBufferInfos) {

    D3D12ScopedImage image = {};
    image.Create(m_device, D3D12_RESOURCE_STATE_COPY_DEST, textBufInfo.m_desc, log_D3D12RenderSystem);
    image.CopyFromBuffer(m_device, oneTimeCommandList, m_mainBuffer, textBufInfo.m_offset);
    image.Transition(oneTimeCommandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_images.PushBack(std::move(image));

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(cpuHandle, textureCPUHandle, m_cbvSrvDescriptorElementSize * idx);

    const auto srvDesc = D3D12ScopedImage::
      GetSRV(textBufInfo.m_desc.m_format, ImageViewType::ImageView2D, log_D3D12RenderSystem);
    m_device->CreateShaderResourceView(m_images.Last().Real(), &srvDesc, cpuHandle);
    ++idx;
  }

  const CD3DX12_CPU_DESCRIPTOR_HANDLE samplerCPUHandle(m_descriptorSamplerHeap->GetCPUDescriptorHandleForHeapStart());

  m_samplers.Reserve(m_samplerInfos.GetSize());

  idx = 0;
  for (const auto& samplerInfo : m_samplerInfos) {
    D3D12ScopedSampler sampler = {};
    sampler.Create();

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(cpuHandle, samplerCPUHandle, m_cbvSrvDescriptorElementSize * idx);

    m_device->CreateSampler(&sampler.GetDesc(), cpuHandle);

    ++idx;
  }
}

void D3D12DrawablePool::Submit() {
  std::sort(m_textureBufferInfos.Begin(), m_textureBufferInfos.End(), [](const TextureBufferInfo& a, const TextureBufferInfo& b) -> bool
  {
    if (a.m_set == b.m_set)
    {
      return a.m_binding < b.m_binding;
    }

    return a.m_set < b.m_set;
  });

  std::sort(m_samplerInfos.Begin(), m_samplerInfos.End(), [](const SamplerInfo& a, const SamplerInfo& b) -> bool
  {
    if (a.m_set == b.m_set)
    {
      return a.m_binding < b.m_binding;
    }

    return a.m_set < b.m_set;
  });

  for(U32 idx = 0; idx < m_renderPasses.GetSize(); ++idx)
  {
    D3D12ScopedCommandBuffer cmdBuffer(m_device, D3D12_COMMAND_LIST_TYPE_BUNDLE, log_D3D12RenderSystem);
    m_secondaryCommandBuffers.PushBack(cmdBuffer);
  }

  m_pipelines.Reserve(m_renderPasses.GetSize());

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Submitting");
  m_pipelineFactory.Submit(m_device, m_renderPasses, m_pipelines);

  // Create a Command Buffer to submit data for drawable
  // Describe and create the command queue.
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

  ComPtr<ID3D12CommandQueue> commandQueue;
  VERIFY_D3D_OP(log_D3D12RenderSystem, m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)),
    "Failed to create command Queue");

  auto oneTimeSubmitBuffer = D3D12ScopedCommandBuffer(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, log_D3D12RenderSystem);
  oneTimeSubmitBuffer.CreateGraphicsCommandList(m_device, m_pipelines[0].GetState(), log_D3D12RenderSystem);

  auto oneTimeCommandList = oneTimeSubmitBuffer.GetGraphicsCommandList();

  const UINT64 stagingBufferSize = GetRequiredIntermediateSize(m_stagingBuffer.Real(), 0, 1);
  D3D12Core::CopyBuffer(oneTimeCommandList, m_mainBuffer, m_stagingBuffer, stagingBufferSize);

  m_mainBuffer.Transition(oneTimeCommandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);

  SetTextureData(oneTimeCommandList);

  m_mainBuffer.Transition(oneTimeCommandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

  oneTimeCommandList->Close();

  oneTimeSubmitBuffer.Execute(m_device, commandQueue.Get(), log_D3D12RenderSystem);
  oneTimeSubmitBuffer.WaitForComplete(commandQueue.Get(), log_D3D12RenderSystem);

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Created Pipelines");

  m_cbvSrvDescriptorElementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  m_samplerDescriptorElementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  const U32 drawableHeapOffset      = m_descriptorsPerDrawable * m_cbvSrvDescriptorElementSize;

  CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_descriptorDrawableHeap->GetCPUDescriptorHandleForHeapStart(), m_offsetToDrawableHeap, m_cbvSrvDescriptorElementSize);

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Creating Resource Views");

  for (auto& drawable : m_drawables) {
    drawable.CreateResourceViews(m_device, m_mainBuffer.Real(), m_vertexDataSlots, heapHandle,
                                 m_cbvSrvDescriptorElementSize, log_D3D12RenderSystem);
    heapHandle.Offset(drawableHeapOffset);
  }

  // Record Command Buffer

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Begin Recording");

  U32 idx = 0;

  U32 sampledImageRecorded = 0;
  U32 samplersRecorded = 0;
  for(const auto& renderPassRef : m_renderPasses)
  {
    const auto& renderPass = renderPassRef.get();
    const auto& renderPassDescriptorCount = renderPass.GetDescriptorCount();
    const auto& renderPassRootSignatureTable = renderPass.GetRootSignatureTable();
    m_secondaryCommandBuffers[idx].CreateGraphicsCommandList(m_device, m_pipelines[idx].GetState(), log_D3D12RenderSystem);
    auto bundleCommandList = m_secondaryCommandBuffers[idx].GetGraphicsCommandList();

    // Define Heap Handles
    CD3DX12_GPU_DESCRIPTOR_HANDLE textureGPUHandle(m_descriptorDrawableHeap->GetGPUDescriptorHandleForHeapStart(), sampledImageRecorded, m_cbvSrvDescriptorElementSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE samplerGPUHandle(m_descriptorSamplerHeap->GetGPUDescriptorHandleForHeapStart(), samplersRecorded, m_samplerDescriptorElementSize);

    CD3DX12_GPU_DESCRIPTOR_HANDLE uboGPUHandle(m_descriptorDrawableHeap->GetGPUDescriptorHandleForHeapStart(), m_offsetToDrawableHeap, m_cbvSrvDescriptorElementSize);


    bundleCommandList->SetDescriptorHeaps(UINT(m_allHeaps.GetSize()), m_allHeaps.Data());
    bundleCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    bundleCommandList->SetGraphicsRootSignature(renderPass.GetRootSignature());

    U32 tableIdx = 0;
    for(const auto& tableEntry : renderPassRootSignatureTable)
    {
      if (tableEntry.m_type == DescriptorType::SampledImage)
      {
        bundleCommandList->SetGraphicsRootDescriptorTable(tableIdx, textureGPUHandle);
        textureGPUHandle.Offset(tableEntry.m_count, m_cbvSrvDescriptorElementSize);
      }
      else if (tableEntry.m_type == DescriptorType::Sampler)
      {
        bundleCommandList->SetGraphicsRootDescriptorTable(tableIdx, samplerGPUHandle);
        samplerGPUHandle.Offset(tableEntry.m_count, m_samplerDescriptorElementSize);
      }

      ++tableIdx;
    }

    samplersRecorded += renderPassDescriptorCount.m_numSamplerSlots;
    sampledImageRecorded += renderPassDescriptorCount.m_numSampledImageSlots;

    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Recording Commands For Drawables");

    for (auto& drawable : m_drawables) {
      drawable.RecordCommands(bundleCommandList, uboGPUHandle, m_cbvSrvDescriptorElementSize, renderPassRootSignatureTable, log_D3D12RenderSystem);
      uboGPUHandle.Offset(drawableHeapOffset);
    }

    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Drawable Pool: Closing Bundle Command Buffer");
    VERIFY_D3D_OP(log_D3D12RenderSystem, bundleCommandList->Close(), "Failed to close bundle Command Buffer");

    ++idx;
  }
}

const Vector<ID3D12DescriptorHeap*>& D3D12DrawablePool::GetAllDescriptorHeaps() const {
  return m_allHeaps;
}

ID3D12PipelineState* D3D12DrawablePool::GetPipelineState(U32 renderPassId) const {
  return m_pipelines[renderPassId].GetState();
}

ID3D12GraphicsCommandList* D3D12DrawablePool::GetSecondaryCommandList(U32 renderPassId) const {
  return m_secondaryCommandBuffers[renderPassId].GetGraphicsCommandList();
}

void D3D12DrawablePool::GetRecordEntries(Vector<std::pair<U32, D3D12RenderPassRecordEntry>>& recordList) const {
  recordList.Reserve(m_renderPasses.GetSize());
  
  U32 idx = 0;
  for(const auto& renderPass : m_renderPasses)
  {
    D3D12RenderPassRecordEntry entry = {};
    entry.m_bundle = GetSecondaryCommandList(idx);
    entry.m_pso = GetPipelineState(idx);

    recordList.PushBack(std::make_pair(renderPass.get().GetId(), entry));
  }

  ++idx;
}

void D3D12DrawablePool::CreateRenderPassReferences(const DrawablePoolCreateInfo& createInfo, const Containers::Vector<D3D12ScopedRenderPass>& renderPasses) {

  U32 idx = 0;
  for(const auto& renderPass : renderPasses)
  {
    auto it = std::find_if(createInfo.m_renderPasses.Begin(), createInfo.m_renderPasses.End(), [&](U32 passId)
    {
      return renderPass.GetId() == passId;
    });

    if (it != createInfo.m_renderPasses.End())
    {
      m_renderPasses.PushBack(std::reference_wrapper<D3D12ScopedRenderPass>(renderPasses[idx]));
    }

    ++idx;
  }
}

void D3D12DrawablePool::CreateInputAttributes(const DrawablePoolCreateInfo& createInfo) {
  U32 idx = 0;
  for (const auto& vertexSlot : createInfo.m_vertexDataSlots) {
    m_pipelineFactory.BulkAddAttributeDescription(vertexSlot, idx);
    ++idx;
  }
}

void D3D12DrawablePool::CreateDescriptorHeap(const DrawablePoolCreateInfo& createInfo) {
  m_allHeaps.Reserve(2);

  m_offsetToDrawableHeap = 0;
  m_offsetToRenderPassInputs = 0;

  U32 numSamplers = 0;

  for(const auto& renderPass : m_renderPasses)
  {
    const auto& count = renderPass.get().GetDescriptorCount();
    m_offsetToDrawableHeap += count.m_numSampledImageSlots;
    m_offsetToDrawableHeap += renderPass.get().GetInputInfo().GetSize(); // RTV as inputs

    m_offsetToRenderPassInputs += count.m_numSampledImageSlots;

    m_descriptorsPerDrawable += count.m_numUniformSlots;
    numSamplers += count.m_numSamplerSlots;
  }

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors             = m_offsetToDrawableHeap + createInfo.m_numDrawables * m_descriptorsPerDrawable;
  heapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  VERIFY_D3D_OP(log_D3D12RenderSystem, m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorDrawableHeap)
  ), "Failed to create Drawable Descriptor Heap");
  m_allHeaps.PushBack(m_descriptorDrawableHeap.Get());

  if (numSamplers > 0) {
    D3D12_DESCRIPTOR_HEAP_DESC samplerDesc = {};
    samplerDesc.NumDescriptors             = numSamplers;
    samplerDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    VERIFY_D3D_OP(log_D3D12RenderSystem, m_device->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(&
      m_descriptorSamplerHeap)), "Failed to create Sampler Descriptor Heap");

    m_allHeaps.PushBack(m_descriptorSamplerHeap.Get());
  }
}
} // namespace D3D12
} // namespace Azura
