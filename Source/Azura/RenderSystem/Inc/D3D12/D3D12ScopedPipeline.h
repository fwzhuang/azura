#pragma once

#include <map>

#include "Log/Log.h"
#include "Generic/GenericTypes.h"
#include "Memory/Allocator.h"
#include "D3D12/D3D12ScopedShader.h"
#include "D3D12/D3D12ScopedRenderPass.h"
#include "D3D12/D3D12ScopedComputePass.h"
#include <optional>

namespace Azura {
namespace D3D12 {

  class D3D12ScopedPipeline {
  public:
    D3D12ScopedPipeline(const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, const Log& log);
    D3D12ScopedPipeline(const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc, const Log& log);
    ID3D12PipelineState* GetState() const;

    PipelineType GetType() const;

  private:
    PipelineType m_type;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline;
  };

  class D3D12PipelineFactory {
  public:
    D3D12PipelineFactory(Memory::Allocator& allocator, Log logger);

    D3D12PipelineFactory & SetPipelineType(PipelineType type);

    D3D12PipelineFactory& BulkAddAttributeDescription(const VertexSlot& vertexSlot, U32 binding);

    D3D12PipelineFactory & SetRasterizerStage(CullMode cullMode, FrontFace faceOrder);

    D3D12PipelineFactory& AddShaderStage(const D3D12ScopedShader& shader);

    void Submit(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Containers::Vector<std::reference_wrapper<D3D12ScopedRenderPass>>& renderPasses, Containers::Vector<D3D12ScopedPipeline>& resultPipelines) const;

    void Submit(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Containers::Vector<std::reference_wrapper<D3D12ScopedComputePass>>& computePasses, Containers::Vector<D3D12ScopedPipeline>& resultPipelines) const;

  private:
    const Log log_D3D12RenderSystem;

    PipelineType m_type;

    struct BindingInfo {
      U32 m_offset{0};
    };

    // TODO(vasumahesh1): Make our own map
    std::map<U32, BindingInfo> m_bindingMap;

    D3D12_RASTERIZER_DESC m_rasterizerDesc{CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)};

    Containers::Vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs;
    std::optional<D3D12_SHADER_BYTECODE> m_vertexShaderModule;
    std::optional<D3D12_SHADER_BYTECODE> m_pixelShaderModule;
    std::optional<D3D12_SHADER_BYTECODE> m_computeShaderModule;
  };


} // namespace D3D12
} // namespace Azura