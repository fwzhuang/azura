#include "TestZone/AppRenderer.h"
#include "Core/RawStorageFormat.h"
#include "Generic/RenderSystem.h"
#include "Generic/Shader.h"
#include "Memory/MemoryFactory.h"
#include "Memory/MonotonicAllocator.h"
#include "Math/Transform.h"
#include "Math/Icosphere.h"
#include "Math/Plane.h"

namespace Azura {
using namespace Containers; // NOLINT
using namespace Math;       // NOLINT
namespace {
  struct Vertex {
    float m_pos[4];
    float m_col[4];
  };

  struct VertexWithUV {
    float m_pos[4];
    float m_uv[2];
  };

  struct LightData {
    Vector4f m_lightPos;
  };

  constexpr U32 CLOTH_DIV_X = 10;
  constexpr U32 CLOTH_DIV_Y = 10;

} // namespace

AppRenderer::AppRenderer()
  : m_mainBuffer(16384),
    m_mainAllocator(m_mainBuffer, 8192),
    m_drawableAllocator(m_mainBuffer, 8192),
    m_camera(1280, 720),
    m_clothPlane(Vector2f(-3.0f, -3.0f), Vector2f(3.0f, 3.0f), Vector2u(CLOTH_DIV_X, CLOTH_DIV_Y)),
    log_AppRenderer(Log("AppRenderer")) {
}

void AppRenderer::Initialize() {
  LOG_INF(log_AppRenderer, LOG_LEVEL, "Starting Init of AppRenderer");

  HEAP_ALLOCATOR(Temporary, Memory::MonotonicAllocator, 16384);
  m_window = RenderSystem::CreateApplicationWindow("TestZone", 1280, 720);

  m_window->SetUpdateCallback([this](float deltaTime)
  {
    WindowUpdate(deltaTime);
  });

  m_window->SetKeyEventCallback([this](KeyEvent evt)
  {
    m_camera.OnKeyEvent(evt);
  });

  m_window->SetMouseEventCallback([this](MouseEvent evt)
  {
    UNUSED(evt);
    // Disable as it has bugs
    // m_camera.OnMouseEvent(e);
  });

  m_camera.SetZoom(10);
  m_camera.RotateAboutRight(-30);

  VERIFY_TRUE(log_AppRenderer, m_window->Initialize(), "Cannot Initialize Window");

  ApplicationInfo appInfo;
  appInfo.m_name    = "TestZone";
  appInfo.m_version = Version(1, 0, 0);

  DeviceRequirements requirements;
  requirements.m_discreteGPU   = true;
  requirements.m_float64       = false;
  requirements.m_int64         = false;
  requirements.m_transferQueue = false;

  m_camera.Recompute();

  m_clothUBO                     = {};
  m_clothUBO.m_model             = Matrix4f::FromTranslationVector(Vector3f(0, 3, 0));
  m_clothUBO.m_view              = m_camera.GetViewMatrix();
  m_clothUBO.m_viewProj          = m_camera.GetViewProjMatrix();
  m_clothUBO.m_invViewProj       = m_camera.GetInvViewProjMatrix();
  m_clothUBO.m_invProj           = m_camera.GetProjMatrix().Inverse();
  m_clothUBO.m_modelInvTranspose = m_clothUBO.m_model.Inverse().Transpose();

  m_sphereUBO                     = {};
  m_sphereUBO.m_model             = Matrix4f::Identity();
  m_sphereUBO.m_viewProj          = m_camera.GetViewProjMatrix();
  m_sphereUBO.m_view              = m_camera.GetViewMatrix();
  m_sphereUBO.m_invViewProj       = m_camera.GetInvViewProjMatrix();
  m_sphereUBO.m_invProj           = m_camera.GetProjMatrix().Inverse();
  m_sphereUBO.m_modelInvTranspose = m_sphereUBO.m_model.Inverse().Transpose();

  LightData lightData  = {};
  lightData.m_lightPos = Vector4f(0.0f, 15.0f, 0.0f, 1.0f);

  // TODO(vasumahesh1):[Q]:Allocator?
  const ApplicationRequirements applicationRequirements = {};

  TextureRequirements textureRequirements = {};
  textureRequirements.m_maxCount          = 1;
  textureRequirements.m_poolSize          = 0x400000; // 4MB

  DescriptorRequirements descriptorRequirements = DescriptorRequirements(2, 2, allocatorTemporary);
  const U32 UBO_SLOT                            = descriptorRequirements.AddDescriptor({
    DescriptorType::UniformBuffer, ShaderStage::Vertex
  });
  const U32 LIGHT_SLOT = descriptorRequirements.AddDescriptor({
    DescriptorType::UniformBuffer, ShaderStage::Vertex
  });

  const U32 UBO_SET   = descriptorRequirements.AddSet({UBO_SLOT});
  const U32 LIGHT_SET = descriptorRequirements.AddSet({LIGHT_SLOT});

  ShaderRequirements shaderRequirements = ShaderRequirements(2, allocatorTemporary);
  const U32 VERTEX_SHADER_ID            = shaderRequirements.AddShader({
    ShaderStage::Vertex, "Cloth.vs", AssetLocation::Shaders
  });
  const U32 PIXEL_SHADER_ID = shaderRequirements.AddShader({
    ShaderStage::Pixel, "Cloth.ps", AssetLocation::Shaders
  });

  RenderPassRequirements renderPassRequirements = RenderPassRequirements(1, 2, 0, allocatorTemporary);
  renderPassRequirements.m_maxPools             = 1;

  const U32 SINGLE_PASS = renderPassRequirements.AddPass({
    PipelinePassCreateInfo::Shaders{VERTEX_SHADER_ID, PIXEL_SHADER_ID}, // SHADERS
    PipelinePassCreateInfo::InputTargets{},                             // INPUT TARGETS
    PipelinePassCreateInfo::InputBuffers{},                             // INPUT TARGETS
    PipelinePassCreateInfo::Outputs{},                                  // OUTPUT TARGETS
    PipelinePassCreateInfo::DescriptorSets{UBO_SET, LIGHT_SET},         // DESCRIPTORS
    ClearData{{0.2f, 0.2f, 0.2f, 1.0f}, 1.0f, 0}
  });

  m_renderer = RenderSystem::CreateRenderer(appInfo, requirements, applicationRequirements,
                                            m_window->GetSwapChainRequirements(), renderPassRequirements,
                                            descriptorRequirements, shaderRequirements, m_mainAllocator,
                                            m_drawableAllocator,
                                            *m_window);

  m_textureManager = RenderSystem::CreateTextureManager(textureRequirements);

  IcoSphere sphere(4);

  const auto& clothVertices = m_clothPlane.GetVertices();

  m_clothVertexVel.resize(m_clothPlane.GetVertices().size());
  m_clothProjectedPos.resize(m_clothPlane.GetVertices().size());

  for (U32 idx     = 0; idx < CLOTH_DIV_X; ++idx) {
    for (U32 idy   = 0; idy < CLOTH_DIV_Y; ++idy) {
      const U32 i1 = ((CLOTH_DIV_Y + 1) * idx) + idy;
      const U32 i2 = ((CLOTH_DIV_Y + 1) * (idx + 1)) + idy;
      const U32 i21 = i2 + 1;
      const U32 i11 = i1 + 1;

      EdgeConstraints e1 = { i1, i2, (clothVertices[i1] - clothVertices[i2]).Length() };
      EdgeConstraints e2 = { i1, i11, (clothVertices[i1] - clothVertices[i11]).Length() };
      EdgeConstraints e3 = { i1, i21, (clothVertices[i1] - clothVertices[i21]).Length() };

      EdgeConstraints e4 = { i2, i11, (clothVertices[i2] - clothVertices[i11]).Length() };
      EdgeConstraints e5 = { i2, i21, (clothVertices[i2] - clothVertices[i21]).Length() };
      EdgeConstraints e6 = { i11, i21, (clothVertices[i11] - clothVertices[i21]).Length() };

      m_clothEdgeConstraints.insert(e1);
      m_clothEdgeConstraints.insert(e2);
      m_clothEdgeConstraints.insert(e3);
      m_clothEdgeConstraints.insert(e4);
      m_clothEdgeConstraints.insert(e5);
      m_clothEdgeConstraints.insert(e6);
    }
  }

  for (auto& velocity : m_clothVertexVel) {
    velocity = Vector4f(0, 0, 0, 0);
  }

  DrawablePoolCreateInfo poolInfo = {allocatorTemporary};
  poolInfo.m_byteSize             = sphere.TotalDataSize() + m_clothPlane.TotalDataSize() + 0x400000;
  poolInfo.m_numDrawables         = 2;
  poolInfo.m_renderPasses         = {{SINGLE_PASS}, allocatorTemporary};
  poolInfo.m_drawType             = DrawType::InstancedIndexed;

  const auto VERTEX_SLOT = poolInfo.AddInputSlot({
    BufferUsageRate::PerVertex, {{"POSITION", RawStorageFormat::R32G32B32A32_FLOAT}}
  });
  const auto NORMAL_SLOT = poolInfo.AddInputSlot({
    BufferUsageRate::PerVertex, {{"NORMAL", RawStorageFormat::R32G32B32_FLOAT}}
  });

  DrawablePool& pool = m_renderer->CreateDrawablePool(poolInfo);

  const auto uboDataBuffer   = reinterpret_cast<U8*>(&m_clothUBO);  // NOLINT
  const auto sphereUBO       = reinterpret_cast<U8*>(&m_sphereUBO); // NOLINT
  const auto lightDataBuffer = reinterpret_cast<U8*>(&lightData);   // NOLINT
  // Create Drawable from Pool
  DrawableCreateInfo createInfo = {};
  createInfo.m_vertexCount      = m_clothPlane.GetVertexCount();
  createInfo.m_indexCount       = m_clothPlane.GetIndexCount();
  createInfo.m_instanceCount    = 1;
  createInfo.m_indexType        = RawStorageFormat::R32_UINT;

  const auto clothId = pool.CreateDrawable(createInfo);
  pool.BindVertexData(clothId, VERTEX_SLOT, m_clothPlane.VertexData(), m_clothPlane.VertexDataSize());
  pool.BindVertexData(clothId, NORMAL_SLOT, m_clothPlane.NormalData(), m_clothPlane.NormalDataSize());
  pool.SetIndexData(clothId, m_clothPlane.IndexData(), m_clothPlane.IndexDataSize());
  pool.BindUniformData(clothId, UBO_SLOT, uboDataBuffer, sizeof(SceneUBO));
  pool.BindUniformData(clothId, LIGHT_SLOT, lightDataBuffer, sizeof(LightData));

  DrawableCreateInfo sphereDrawableInfo = {};
  sphereDrawableInfo.m_vertexCount      = sphere.GetVertexCount();
  sphereDrawableInfo.m_indexCount       = sphere.GetIndexCount();
  sphereDrawableInfo.m_instanceCount    = 1;
  sphereDrawableInfo.m_indexType        = sphere.GetIndexFormat();

  const auto sphereId = pool.CreateDrawable(sphereDrawableInfo);
  pool.BindVertexData(sphereId, VERTEX_SLOT, sphere.VertexData(), sphere.VertexDataSize());
  pool.BindVertexData(sphereId, NORMAL_SLOT, sphere.NormalData(), sphere.NormalDataSize());
  pool.SetIndexData(sphereId, sphere.IndexData(), sphere.IndexDataSize());
  pool.BindUniformData(sphereId, UBO_SLOT, sphereUBO, sizeof(SceneUBO));
  pool.BindUniformData(sphereId, LIGHT_SLOT, lightDataBuffer, sizeof(LightData));

  m_renderPass.m_sceneUBOSlot = UBO_SLOT;
  m_renderPass.m_clothId      = clothId;
  m_renderPass.m_sphereId     = sphereId;

  m_mainPool = &pool;

  // All Drawables Done
  m_renderer->Submit();

  LOG_INF(log_AppRenderer, LOG_LEVEL, "Initialized AppRenderer");
}

void AppRenderer::WindowUpdate(float timeDelta) {
  m_camera.Update(timeDelta);

  auto& clothVertices = m_clothPlane.GetVertices();

  // for (auto& vertex : clothVertices) {
  //   vertex[1] -= (timeDelta * 1.5f);
  // }

  // Projected Positions
  std::memcpy(m_clothProjectedPos.data(), clothVertices.data(), sizeof(Vector4f) * clothVertices.size());

  const Vector4f gravity = {0.0f, -9.8f, 0.0f, 0.0f};

  const U32 numEntries = U32(clothVertices.size());

  for (U32 mainItr = 0; mainItr < 10; ++mainItr) {
    
    // External Force Update
    for (U32 idx = 0; idx < numEntries; ++idx) {
      m_clothVertexVel[idx] = m_clothVertexVel[idx] + timeDelta * 1.0f * gravity;
    }

    // Damp

    // Projected Positions
    for (U32 idx = 0; idx < numEntries; ++idx) {
      m_clothProjectedPos[idx] = m_clothProjectedPos[idx] +m_clothVertexVel[idx] * timeDelta;
    }


  }

  m_clothUBO.m_view              = m_camera.GetViewMatrix();
  m_clothUBO.m_viewProj          = m_camera.GetViewProjMatrix();
  m_clothUBO.m_invViewProj       = m_camera.GetInvViewProjMatrix();
  m_clothUBO.m_invProj           = m_camera.GetProjMatrix().Inverse();
  m_clothUBO.m_modelInvTranspose = m_clothUBO.m_model.Inverse().Transpose();

  m_sphereUBO.m_view              = m_camera.GetViewMatrix();
  m_sphereUBO.m_viewProj          = m_camera.GetViewProjMatrix();
  m_sphereUBO.m_invViewProj       = m_camera.GetInvViewProjMatrix();
  m_sphereUBO.m_invProj           = m_camera.GetProjMatrix().Inverse();
  m_sphereUBO.m_modelInvTranspose = m_sphereUBO.m_model.Inverse().Transpose();

  const auto vbStart          = reinterpret_cast<U8*>(clothVertices.data()); // NOLINT
  const auto uboDataBuffer    = reinterpret_cast<U8*>(&m_clothUBO);          // NOLINT
  const auto sphereDataBuffer = reinterpret_cast<U8*>(&m_sphereUBO);         // NOLINT

  m_mainPool->BeginUpdates();
  // Update Cloth
  m_mainPool->UpdateUniformData(m_renderPass.m_clothId, m_renderPass.m_sceneUBOSlot, uboDataBuffer, sizeof(SceneUBO));
  m_mainPool->UpdateVertexData(m_renderPass.m_clothId, m_renderPass.m_sceneUBOSlot, vbStart,
                               U32(clothVertices.size()) * sizeof(Vector4f));

  // Update Sphere
  m_mainPool->UpdateUniformData(m_renderPass.m_sphereId, m_renderPass.m_sceneUBOSlot, sphereDataBuffer,
                                sizeof(SceneUBO));
  m_mainPool->SubmitUpdates();

  m_renderer->RenderFrame();
}

void AppRenderer::Run() const {
  LOG_INF(log_AppRenderer, LOG_LEVEL, "Running AppRenderer");
  m_window->StartListening();
}

void AppRenderer::Destroy() const {
  m_window->Destroy();
}
} // namespace Azura
