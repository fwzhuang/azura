#include "D3D12/D3D12ScopedComputePass.h"
#include "D3D12/D3D12Macros.h"
#include "Memory/MonotonicAllocator.h"
#include "Memory/MemoryFactory.h"
#include "D3D12/D3D12TypeMapping.h"

using namespace Microsoft::WRL;    // NOLINT
using namespace Azura::Containers; // NOLINT

namespace Azura {
namespace D3D12 {

D3D12ScopedComputePass::D3D12ScopedComputePass(U32 idx, U32 internalId,
                                               Memory::Allocator& mainAllocator,
                                               Log logger)
  : log_D3D12RenderSystem(std::move(logger)),
    m_id(idx),
    m_internalId(internalId),
    m_rootSignatureTable(mainAllocator),
    m_passShaders(mainAllocator),
    m_computeOutputs(mainAllocator),
    m_computeInputs(mainAllocator),
    m_computeDepthInputs(mainAllocator),
    m_allComputeInputs(mainAllocator) {
}

void D3D12ScopedComputePass::Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device,
                                    const PipelinePassCreateInfo& createInfo,
                                    const Containers::Vector<RenderTargetCreateInfo>& pipelineBuffers,
                                    const Containers::Vector<D3D12ScopedImage>& pipelineBufferImages,
                                    const Containers::Vector<DescriptorSlot>& descriptorSlots,
                                    const Containers::Vector<DescriptorTableEntry>& descriptorSetTable,
                                    const Containers::Vector<D3D12ScopedShader>& allShaders) {

  CreateBase(device, createInfo, descriptorSlots, descriptorSetTable, pipelineBuffers, pipelineBufferImages,
             allShaders);

  D3D12ScopedCommandBuffer primaryCmdBuffer = D3D12ScopedCommandBuffer(device, D3D12_COMMAND_LIST_TYPE_COMPUTE,
                                                                       log_D3D12RenderSystem);
  primaryCmdBuffer.CreateGraphicsCommandList(device, nullptr, log_D3D12RenderSystem);
  m_commandBuffers.push_back(primaryCmdBuffer);
}

U32 D3D12ScopedComputePass::GetId() const {
  return m_id;
}

U32 D3D12ScopedComputePass::GetInternalId() const {
  return m_internalId;
}

ID3D12GraphicsCommandList* D3D12ScopedComputePass::GetPrimaryComputeCommandList(U32 idx) const {
  return m_commandBuffers[idx].GetGraphicsCommandList();
}

const DescriptorCount& D3D12ScopedComputePass::GetDescriptorCount() const {
  return m_descriptorCount;
}

ID3D12RootSignature* D3D12ScopedComputePass::GetRootSignature() const {
  return m_rootSignature.Get();
}

const Vector<std::reference_wrapper<const D3D12ScopedShader>>& D3D12ScopedComputePass::GetShaders() const {
  return m_passShaders;
}

const Vector<std::reference_wrapper<D3D12ScopedImage>>& D3D12ScopedComputePass::GetInputImages() const {
  return m_allComputeInputs;
}

const Vector<std::reference_wrapper<D3D12ScopedImage>>& D3D12ScopedComputePass::GetOutputImages() const {
  return m_computeOutputs;
}

const Vector<DescriptorTableEntry>& D3D12ScopedComputePass::GetRootSignatureTable() const {
  return m_rootSignatureTable;
}

void D3D12ScopedComputePass::RecordResourceBarriersForOutputsStart(ID3D12GraphicsCommandList* commandList) const {
  for (auto& rtv : m_computeOutputs) {
    rtv.get().Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  }
}
  
void D3D12ScopedComputePass::RecordResourceBarriersForOutputsEnd(ID3D12GraphicsCommandList* commandList) const {
  for (auto& rtv : m_computeOutputs) {
    rtv.get().Transition(commandList, D3D12_RESOURCE_STATE_COMMON);
  }
}

void D3D12ScopedComputePass::RecordResourceBarriersForInputsStart(ID3D12GraphicsCommandList* commandList) const {
  for (auto& rtv : m_computeInputs) {
    rtv.get().Transition(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
  }

  for (auto& dsv : m_computeDepthInputs) {
    dsv.get().Transition(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
  }
}
  
void D3D12ScopedComputePass::RecordResourceBarriersForInputsEnd(ID3D12GraphicsCommandList* commandList) const {
  for (auto& rtv : m_computeInputs) {
    rtv.get().Transition(commandList, D3D12_RESOURCE_STATE_COMMON);
  }

  for (auto& dsv : m_computeDepthInputs) {
    dsv.get().Transition(commandList, D3D12_RESOURCE_STATE_COMMON);
  }
}

void D3D12ScopedComputePass::WaitForGPU(ID3D12CommandQueue* commandQueue) {
  const U32 fence = m_fenceValue;
  VERIFY_D3D_OP(log_D3D12RenderSystem, commandQueue->Signal(m_signalFence.Get(), m_fenceValue), "Fence wait failed");
  m_fenceValue++;

  // Wait until the previous frame is finished.
  if (m_signalFence->GetCompletedValue() < fence) {
    VERIFY_D3D_OP(log_D3D12RenderSystem, m_signalFence->SetEventOnCompletion(fence, m_fenceEvent),
      "Failed to set event completion on Fence");
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }

}

U32 D3D12ScopedComputePass::GetCommandBufferCount() const {
  return U32(m_commandBuffers.size());
}

U32 D3D12ScopedComputePass::GetInputRootDescriptorTableId() const {
  return m_computeInputTableIdx;
}

U32 D3D12ScopedComputePass::GetOutputRootDescriptorTableId() const {
  return m_computeOutputTableIdx;
}

void D3D12ScopedComputePass::CreateBase(
  const Microsoft::WRL::ComPtr<ID3D12Device>& device,
  const PipelinePassCreateInfo& createInfo,
  const Containers::Vector<DescriptorSlot>& descriptorSlots,
  const Containers::Vector<DescriptorTableEntry>& descriptorSetTable,
  const Containers::Vector<RenderTargetCreateInfo>& pipelineBuffers,
  const Containers::Vector<D3D12ScopedImage>& pipelineBufferImages,
  const Containers::Vector<D3D12ScopedShader>& allShaders) {

  STACK_ALLOCATOR(Temporary, Memory::MonotonicAllocator, 2048);

  VERIFY_D3D_OP(log_D3D12RenderSystem, device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_signalFence)),
    "Failed to create fence");

  // Create an event handle to use for frame synchronization.
  m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_fenceEvent == nullptr) {
    VERIFY_D3D_OP(log_D3D12RenderSystem, HRESULT_FROM_WIN32(GetLastError()), "Fence Event Null");
  }

  m_passShaders.Reserve(U32(createInfo.m_shaders.size()));
  m_computeInputs.Reserve(U32(createInfo.m_inputs.size()));
  m_computeDepthInputs.Reserve(U32(createInfo.m_inputs.size()));
  m_allComputeInputs.Reserve(U32(createInfo.m_inputs.size()));
  m_computeOutputs.Reserve(U32(createInfo.m_outputs.size()));

  for (const auto& outputId : createInfo.m_outputs) {
    m_computeOutputs.PushBack(pipelineBufferImages[outputId]);
  }

  for (const auto& shaderId : createInfo.m_shaders) {
    const auto& shaderRef = allShaders[shaderId];
    m_passShaders.PushBack(shaderRef);
  }

  for (const auto& inputId : createInfo.m_inputs) {
    // TODO(vasumahesh1):[?]: Respect shader stages here for correct binding
    const auto& targetBufferRef = pipelineBuffers[inputId.m_id];

    if (HasDepthOrStencilComponent(targetBufferRef.m_format)) {
      m_computeDepthInputs.PushBack(pipelineBufferImages[inputId.m_id]);
    } else {
      m_computeInputs.PushBack(pipelineBufferImages[inputId.m_id]);
    }

    m_allComputeInputs.PushBack(pipelineBufferImages[inputId.m_id]);
  }

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "======== D3D12 Render Pass: Root Signature ========");

  Vector<CD3DX12_ROOT_PARAMETER> descriptorTables(U32(createInfo.m_descriptorSets.size() + 1), allocatorTemporary);

  m_rootSignatureTable.Reserve(U32(createInfo.m_descriptorSets.size()));

  U32 cbvOffset     = 0;
  U32 srvOffset     = 0;
  U32 uavOffset     = 0;
  U32 samplerOffset = 0;

  for (const auto& setId : createInfo.m_descriptorSets) {
    const auto& tableEntry = descriptorSetTable[setId];

    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: Generating for Set Position: %d", m_rootSignatureTable
      .GetSize());
    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: Original Set Position: %d", setId);

    m_rootSignatureTable.PushBack(tableEntry);

    Vector<CD3DX12_DESCRIPTOR_RANGE> currentRanges(tableEntry.m_count, allocatorTemporary);

    for (const auto& slot : descriptorSlots) {
      if (slot.m_setIdx != setId) {
        continue;
      }

      CD3DX12_DESCRIPTOR_RANGE rangeData;

      switch (slot.m_type) {
        case DescriptorType::UniformBuffer:
          LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: [%d] Applying Uniform Buffer at Register b(%d)",
            setId, cbvOffset);

          rangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cbvOffset);
          currentRanges.PushBack(rangeData);
          ++cbvOffset;

          ++m_descriptorCount.m_numUniformSlots;
          break;

        case DescriptorType::Sampler:
          LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: [%d] Applying Sampler Buffer at Register s(%d)",
            setId, samplerOffset);

          rangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, samplerOffset);
          currentRanges.PushBack(rangeData);
          ++samplerOffset;

          ++m_descriptorCount.m_numSamplerSlots;
          break;

        case DescriptorType::SampledImage:
          LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: [%d] Applying Texture Image at Register t(%d)",
            setId, srvOffset);

          rangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvOffset);
          currentRanges.PushBack(rangeData);
          ++srvOffset;

          ++m_descriptorCount.m_numSampledImageSlots;
          break;

        case DescriptorType::UnorderedView:
          LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: [%d] Applying UAV at Register t(%d)",
            setId, uavOffset);

          rangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, uavOffset);
          currentRanges.PushBack(rangeData);
          ++uavOffset;

          ++m_descriptorCount.m_numUnorderedViewSlots;
          break;

        case DescriptorType::PushConstant:
        case DescriptorType::CombinedImageSampler:
        default:
          LOG_ERR(log_D3D12RenderSystem, LOG_LEVEL, "Unsupported Descriptor Type for D3D12");
          break;
      }
    }

    CD3DX12_ROOT_PARAMETER rootParameter;
    rootParameter.InitAsDescriptorTable(currentRanges.GetSize(), currentRanges.Data(), D3D12_SHADER_VISIBILITY_ALL);
    descriptorTables.PushBack(rootParameter);
  }

  CD3DX12_ROOT_PARAMETER inputsRootParameter;
  CD3DX12_ROOT_PARAMETER ouputsRootParameter;
  CD3DX12_DESCRIPTOR_RANGE inputRangeData;
  CD3DX12_DESCRIPTOR_RANGE outputRangeData;

  // Have some inputs
  if (!createInfo.m_inputs.empty()) {
    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: Generating SRV for Set Position: %d", descriptorTables.
      GetSize());
    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
      "D3D12 Render Pass: [Attachments] Applying %d Image Attachments as register t(%d) to t(%d)", createInfo.m_inputs.
      size(), srvOffset, srvOffset + createInfo.m_inputs.size() - 1);

    inputRangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT(createInfo.m_inputs.size()), srvOffset);
    inputsRootParameter.InitAsDescriptorTable(1, &inputRangeData, D3D12_SHADER_VISIBILITY_ALL);

    m_computeInputTableIdx = descriptorTables.GetSize();

    descriptorTables.PushBack(inputsRootParameter);
  }

  if (!createInfo.m_outputs.empty()) {
    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: Generating UAV for Set Position: %d", descriptorTables.
      GetSize());
    LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL,
      "D3D12 Render Pass: [Attachments] Applying %d Image Attachments as register t(%d) to t(%d)", createInfo.m_outputs.
      size(), uavOffset, uavOffset + createInfo.m_outputs.size() - 1);

    outputRangeData.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UINT(createInfo.m_outputs.size()), uavOffset);
    ouputsRootParameter.InitAsDescriptorTable(1, &outputRangeData, D3D12_SHADER_VISIBILITY_ALL);

    m_computeOutputTableIdx = descriptorTables.GetSize();

    descriptorTables.PushBack(ouputsRootParameter);
  }

  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init(descriptorTables.GetSize(), descriptorTables.Data(), 0, nullptr,
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  VERIFY_D3D_OP(log_D3D12RenderSystem, D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &
    signature, &error), "Failed to serialize D3D12 Root Signature");

  VERIFY_D3D_OP(log_D3D12RenderSystem, device->CreateRootSignature(0, signature->GetBufferPointer(), signature->
    GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)), "Failed to create Root Signature");

  LOG_DBG(log_D3D12RenderSystem, LOG_LEVEL, "D3D12 Render Pass: Completed Root Signature");

}
} // namespace D3D12
} // namespace Azura
