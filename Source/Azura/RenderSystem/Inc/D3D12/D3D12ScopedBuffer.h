#pragma once
#include <wrl/client.h>
#include "Log/Log.h"

#include "D3D12/D3D12Core.h"


namespace Azura {
namespace D3D12 {

class D3D12ScopedBuffer {
public:

  void Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device,
              const D3D12_HEAP_PROPERTIES& heapProperties,
              U32 size,
              D3D12_RESOURCE_STATES resourceStates,
              const Log& log_D3D12RenderSystem);
  void Map();

  HRESULT SubAllocateFromBuffer(U32 size, U32 alignment);

  U32 AppendData(const void* pData, U32 byteSize, U32 alignment, const Log& log_D3D12RenderSystem);

  U32 AppendTextureData(const void * pData, U32 byteSize, U32 alignment, U32 textureWidth, U32 textureRowPitch, const Log & log_D3D12RenderSystem);

  ID3D12Resource* Real() const;

  U32 GetSize() const;
  U32 GetCurrentOffset() const;

  void Reset();

  void Transition(ID3D12GraphicsCommandList* commandList,
    D3D12_RESOURCE_STATES fromState,
    D3D12_RESOURCE_STATES toState) const;

private:
  Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
  U32 m_size{0};

  UINT8* p_dataBegin{nullptr};
  UINT8* p_dataCur{nullptr};
  UINT8* p_dataEnd{nullptr};
};

} // namespace D3D12
} // namespace Azura