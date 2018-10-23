#pragma once
#include "Types.h"

#include "Log/Log.h"
#include "D3D12/D3D12Core.h"
#include "D3D12/D3D12ScopedImage.h"
#include "D3D12/D3D12ScopedShader.h"
#include "D3D12/D3D12ScopedCommandBuffer.h"
#include "D3D12/D3D12ScopedSwapChain.h"

namespace Azura {
namespace Memory {
class Allocator;
}
}

namespace Azura {
namespace D3D12 {

class D3D12ScopedComputePass {
public:
  D3D12ScopedComputePass(U32 idx, U32 internalId, Memory::Allocator& mainAllocator, Log logger);

  void Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device,
              const PipelinePassCreateInfo& createInfo,
              const Containers::Vector<RenderTargetCreateInfo>& pipelineBuffers,
              const Containers::Vector<D3D12ScopedImage>& pipelineBufferImages,
              const Containers::Vector<DescriptorSlot>& descriptorSlots,
              const Containers::Vector<DescriptorTableEntry>& descriptorSetTable,
              const Containers::Vector<D3D12ScopedShader>& allShaders);

  U32 GetId() const;
  U32 GetInternalId() const;

  ID3D12GraphicsCommandList* GetPrimaryComputeCommandList(U32 idx) const;

  const DescriptorCount& GetDescriptorCount() const;

  ID3D12RootSignature* GetRootSignature() const;

  const Containers::Vector<std::reference_wrapper<const D3D12ScopedShader>>& GetShaders() const;
  const Containers::Vector<std::reference_wrapper<D3D12ScopedImage>>& GetInputImages() const;
  const Containers::Vector<std::reference_wrapper<D3D12ScopedImage>>& GetOutputImages() const;

  const Containers::Vector<DescriptorTableEntry>& GetRootSignatureTable() const;

  void RecordResourceBarriersForOutputsStart(ID3D12GraphicsCommandList* commandList) const;
  void RecordResourceBarriersForOutputsEnd(ID3D12GraphicsCommandList* commandList) const;
  void RecordResourceBarriersForInputsStart(ID3D12GraphicsCommandList* commandList) const;
  void RecordResourceBarriersForInputsEnd(ID3D12GraphicsCommandList* commandList) const;

  void WaitForGPU(ID3D12CommandQueue* commandQueue);

  U32 GetCommandBufferCount() const;

  U32 GetInputRootDescriptorTableId() const;
  U32 GetOutputRootDescriptorTableId() const;

private:
  void CreateBase(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const PipelinePassCreateInfo& createInfo,
    const Containers::Vector<DescriptorSlot>& descriptorSlots,
    const Containers::Vector<DescriptorTableEntry>& descriptorSetTable,
    const Containers::Vector<RenderTargetCreateInfo>& pipelineBuffers,
    const Containers::Vector<D3D12ScopedImage>& pipelineBufferImages,
    const Containers::Vector<D3D12ScopedShader>& allShaders);

  const Log log_D3D12RenderSystem;
  U32 m_id;
  U32 m_internalId;

  U32 m_computeOutputTableIdx{ 0 };
  U32 m_computeInputTableIdx{ 0 };

  Containers::Vector<DescriptorTableEntry> m_rootSignatureTable;

  Containers::Vector<std::reference_wrapper<const D3D12ScopedShader>> m_passShaders;

  SmallVector<D3D12ScopedCommandBuffer, DEFAULT_FRAMES_IN_FLIGHT> m_commandBuffers;

  Containers::Vector<std::reference_wrapper<D3D12ScopedImage>> m_computeOutputs;

  Containers::Vector<std::reference_wrapper<D3D12ScopedImage>> m_computeInputs;
  Containers::Vector<std::reference_wrapper<D3D12ScopedImage>> m_computeDepthInputs;
  Containers::Vector<std::reference_wrapper<D3D12ScopedImage>> m_allComputeInputs;

  Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

  DescriptorCount m_descriptorCount{};

  U32 m_fenceValue{1};
  HANDLE m_fenceEvent{};
  Microsoft::WRL::ComPtr<ID3D12Fence> m_signalFence;
  SmallVector<Microsoft::WRL::ComPtr<ID3D12Fence>, MAX_RENDER_PASS_INPUTS> m_waitFences;
};

} // namespace D3D12
} // namespace Azura
