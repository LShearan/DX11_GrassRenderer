#include "Grass.h"

Grass::Grass() 
{

}

Grass::~Grass() {}

void Grass::Init(SystemsInterface& systems, int count)
{
	// inital values
	m_canUpdate = true;
	m_billboard = true;
	m_windDirection = 1;
	m_windRotation = 0.0f;
	m_circleRadius = 5.f;
	m_instanceCount = count;
	float red(randf() / RAND_MAX), green(randf() / RAND_MAX), blue(randf() / RAND_MAX);
	m_grassColour.x = red;
	m_grassColour.y = green;
	m_grassColour.z = blue;
	m_grassColour.w = 1.f;

	m_Instances = new PerInstanceData[m_instanceCount];
	m_grassArr = new GrassType[m_instanceCount];

	// Initialise postion randomly withing a circle
	for (u32 i = 0; i < m_instanceCount; ++i)
	{
		float r = m_circleRadius * sqrtf(randf_norm());
		float theta = randf_norm() * 2 * kfPI;
		float height = randf_norm() + 0.4f;

		m_grassArr[i].x = 0.f + (r * cosf(theta));
		m_grassArr[i].y = 0.f;
		m_grassArr[i].z = 0.f + (r * sinf(theta));

		m_grassArr[i].size = height;

		m_grassArr[i].r = m_grassColour.x + 1.f;
		m_grassArr[i].g = m_grassColour.y + 1.f;
		m_grassArr[i].b = 0.f;
	}

	m4x4 matrix = m4x4::Identity;

	// initialise the instance buffer with some default values;
	for (u32 i = 0; i < m_instanceCount; ++i)
	{
		m_Instances[i].matModel = matrix;
		m_Instances[i].colour = v4(m_grassArr[i].r, m_grassArr[i].g, m_grassArr[i].b, 1.f);
	}

	// initialise shaders
	m_instanceBufferShader.init(systems.pD3DDevice, ShaderSetDesc::Create_VS_PS("Assets/Shaders/GrassInstanceShaderBuffer.fx", "VS_Mesh", "PS_Mesh")
		, { VertexFormatTraits<MeshVertex>::desc, VertexFormatTraits<MeshVertex>::size });

	// create instance buffers
	m_instanceBuffer = create_structured_buffer<PerInstanceData>(systems.pD3DDevice, m_instanceCount);
	m_instanceBufferView = create_structured_buffer_view(systems.pD3DDevice, m_instanceBuffer);

	// Create Buffers
	m_perDrawCB = create_constant_buffer<PerDrawCBData>(systems.pD3DDevice);

	// Intialize mesh
	create_mesh_quad_xy(systems.pD3DDevice, m_mesh, 1.f);

	// Intialize textures
	m_texture.init_from_dds(systems.pD3DDevice, "Assets/Textures/grass.dds");

	m_samplerState = create_basic_sampler(systems.pD3DDevice, D3D11_TEXTURE_ADDRESS_WRAP);



}

void Grass::Update(SystemsInterface& systems)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	v3 camPos = systems.pCamera->getTarget();
	v3 camUp = systems.pCamera->up;
	v3 camForward = systems.pCamera->forward;

	// update the rotation of the grass objects so that the face the camera
	// then attach the appropriate mvp to the instance buffer
	for (u32 i = 0; i < m_instanceCount; ++i)
	{
		v3 position = v3(m_grassArr[i].x, -0.1f, m_grassArr[i].z);

		m4x4 matrix = m4x4::CreateScale(1.f,m_grassArr[i].size,1.f) *  m4x4::CreateBillboard(position, camPos, camUp, &camForward);
		m_Instances[i].matModel = matrix.Transpose();
	}

	HRESULT result = systems.pD3DContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// update the instance buffer with new mvp
	PerInstanceData* instancePtr = (PerInstanceData*)mappedResource.pData;
	memcpy(instancePtr, (void*)m_Instances, (sizeof(PerInstanceData) * m_instanceCount));
	systems.pD3DContext->Unmap(m_instanceBuffer, 0);
}

void Grass::Render(ID3D11DeviceContext* deviceContext, ID3D11Buffer* perFrameCB, SystemsInterface& systems)
{
	// Bind Constant Buffers to both PS and VS Stages
	ID3D11Buffer* buffers[] = { perFrameCB,m_perDrawCB };
	systems.pD3DContext->VSSetConstantBuffers(0, 2, buffers);
	systems.pD3DContext->PSSetConstantBuffers(0, 2, buffers);

	// Bind a sampler state
	ID3D11SamplerState* samplers[] = { m_samplerState };
	systems.pD3DContext->PSSetSamplers(0, 1, samplers);

	// Bind Shaders
	m_instanceBufferShader.bind(systems.pD3DContext);
	//m_instanceShaderNoData.bind(systems.pD3DContext);

	// Choose the mesh and text and bins them
	const Mesh& rMesh(m_mesh);
	const Texture& rTex(m_texture);
	rMesh.bind(systems.pD3DContext);
	rTex.bind(systems.pD3DContext, ShaderStage::kPixel, 0);

	// Bind the instance data
	// This is a Shader Resource View and will occupy register(t0) slot
	systems.pD3DContext->VSSetShaderResources(0, 1, &m_instanceBufferView);

	// Draw
	systems.pD3DContext->DrawIndexedInstanced(m_mesh.indices(), m_instanceCount, 0, 0, 0);
}

void Grass::UpdateGrassLocation(SystemsInterface& systems, int count, float radius)
{
	// re-intialise the grass locations based on updated count and circle radius
	m_circleRadius = radius;
	m_instanceCount = count;
	m_Instances = new PerInstanceData[m_instanceCount];
	m_grassArr = new GrassType[m_instanceCount];

	for (u32 i = 0; i < m_instanceCount; ++i)
	{
		float r = m_circleRadius * sqrtf(randf_norm());
		float theta = randf_norm() * 2 * kfPI;
		float height = randf_norm() + 0.4f;

		m_grassArr[i].x = 0.f + (r * cosf(theta));
		m_grassArr[i].y = 0.f;
		m_grassArr[i].z = 0.f + (r * sinf(theta));

		m_grassArr[i].size = height;

		m_grassArr[i].r = m_grassColour.x + 1.f;
		m_grassArr[i].g = m_grassColour.y + 1.f;
		m_grassArr[i].b = 0.f;
	}

	m4x4 matrix = m4x4::Identity;

	for (u32 i = 0; i < m_instanceCount; ++i)
	{
		m_Instances[i].matModel = matrix;
		m_Instances[i].colour = v4(m_grassArr[i].r, m_grassArr[i].g, m_grassArr[i].b, 1.f);
	}

	m_canUpdate = true;
}