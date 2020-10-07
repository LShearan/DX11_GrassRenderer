#include "Framework.h"

#include "ShaderSet.h"
#include "Mesh.h"
#include "Texture.h"
#include "Grass.h"

class Application : public FrameworkApp 
{
public:
	struct PerFrameCBData
	{
		m4x4	m_matProjection;
		m4x4	m_matView;
		f32		m_time;
		f32     m_padding[3];
	};

	struct PerWindCBData
	{
		v3		m_windDirection;
		f32		m_windStrength;
		f32		m_timeStamp;
		f32		m_padding[3];
	};

	struct PerDrawCBData
	{
		m4x4 m_matMVP;
	};

	void on_init(SystemsInterface& systems) override
	{
		// initialise some values
		circleRadius = 5.f;
		grassCount = 50000;
		m_position = v3(0.5f, 0.5f, 0.5f);
		m_size = 1.0f;
		m_windDirection = 1;
		m_windIcrement = 0.001f;
		systems.pCamera->eye = v3(15.f, 11.f, 7.f);
		systems.pCamera->look_at(v3(0.f, 0.f, 0.f));
		systems.pCamera->farClip = 500.f;


		// Setup per-frame data
		m_perFrameCBData.m_time = 0.0f;

		// direction of the wind gust
		m_perWindCBData.m_windDirection = v3(1.f, 0.f, 0.f);
		// strength of wind what direction the wind will go 1 forward -1 backward
		m_perWindCBData.m_windStrength = 1.f;
		// time stamp to get smooth motion of wind ranging from 1 to 0
		m_perWindCBData.m_timeStamp = 1.f;

		//// Create Per Frame Constant Buffer.
		m_pPerFrameCB = create_constant_buffer<PerFrameCBData>(systems.pD3DDevice);

		// Create Per Frame Constant Buffer.
		m_pPerDrawCB = create_constant_buffer<PerDrawCBData>(systems.pD3DDevice);

		// Create Constant Buffer for wind
		m_pPerWindCB = create_constant_buffer<PerWindCBData>(systems.pD3DDevice);

		// Initialise grass class
		m_grass.Init(systems, grassCount);


		// Initialise floor
		m_meshShader.init(systems.pD3DDevice, ShaderSetDesc::Create_VS_PS("Assets/Shaders/MinimalShaders.fx", "VS_Mesh", "PS_Mesh"),
			{ VertexFormatTraits<MeshVertex>::desc, VertexFormatTraits<MeshVertex>::size });
		create_mesh_quad_xy(systems.pD3DDevice, m_floorMesh, 1500.f);
		m_floorTexture.init_from_dds(systems.pD3DDevice, "Assets/Textures/grassFloor1.dds");


		m_pLinearMipSamplerState = create_basic_sampler(systems.pD3DDevice, D3D11_TEXTURE_ADDRESS_WRAP);

	}

	void on_update(SystemsInterface& systems) override
	{
		// This function displays some useful debugging values, camera positions etc.
		DemoFeatures::editorHud(systems.pDebugDrawContext);


		// Setup up some ui functionality to change how the grass is rendered
		ImGui::SliderFloat3("Wind Direction", (float*)&m_perWindCBData.m_windDirection, -1.f, 1.f);
		ImGui::SliderFloat("Wind Strength", &m_perWindCBData.m_windStrength, 0.f, 50.f);
		ImGui::SliderFloat("Wind Speed", &m_windIcrement, 0.0001f, 1.f);
		ImGui::SliderInt("Grass Count", &grassCount, 1, 50000);
		ImGui::SliderFloat("Circle Radius", &circleRadius, 1.f, 500.f);


		// update the grass button with new count and circle radius
		if (ImGui::Button("Update Grass"))
		{
			m_grass.m_canUpdate = false;
			m_grass.UpdateGrassLocation(systems, grassCount, circleRadius);
		}

		// update wind data

		if (m_windDirection == 1)
		{
			m_perWindCBData.m_timeStamp -= m_windIcrement;
			if (m_perWindCBData.m_timeStamp < -0.9f)
			{
				m_windDirection = 2;
			}
		}
		else
		{
			m_perWindCBData.m_timeStamp += m_windIcrement;
			if (m_perWindCBData.m_timeStamp > 0.9f)
			{
				m_windDirection = 1;
			}
		}

		// Update Per Frame Data.
		m_perFrameCBData.m_matProjection = systems.pCamera->projMatrix.Transpose();
		m_perFrameCBData.m_matView = systems.pCamera->viewMatrix.Transpose();
		m_perFrameCBData.m_time += 0.001f;

		// call update function for the grass
		if (m_grass.m_canUpdate && m_grass.m_billboard) m_grass.Update(systems);
	}

	void on_render(SystemsInterface& systems) override
	{
		// Push Per Frame Data to GPU
		push_constant_buffer(systems.pD3DContext, m_pPerFrameCB, m_perFrameCBData);
		push_constant_buffer(systems.pD3DContext, m_pPerWindCB, m_perWindCBData);

		/* Draw the floor
		*/
		m_meshShader.bind(systems.pD3DContext);

		ID3D11Buffer* buffers[] = { m_pPerFrameCB,m_pPerDrawCB,m_pPerWindCB };
		systems.pD3DContext->VSSetConstantBuffers(0, 3, buffers);
		systems.pD3DContext->PSSetConstantBuffers(0, 3, buffers);

		ID3D11SamplerState* samplers[] = { m_pLinearMipSamplerState };
		systems.pD3DContext->PSSetSamplers(0, 1, samplers);

		m_floorMesh.bind(systems.pD3DContext);
		m_floorTexture.bind(systems.pD3DContext, ShaderStage::kPixel, 0);

		m4x4 matModel = m4x4::CreateRotationX(90.f * 0.0174532925f) * m4x4::CreateTranslation(v3(0.f, -0.9f, 0.f));
		matModel = matModel * systems.pCamera->vpMatrix;
		m_perDrawCBData.m_matMVP = matModel.Transpose();

		push_constant_buffer(systems.pD3DContext, m_pPerDrawCB, m_perDrawCBData);

		m_floorMesh.draw(systems.pD3DContext);


		/* Draw the grass
		*/
		m_grass.Render(systems.pD3DContext, m_pPerFrameCB, systems);

	}

	void on_resize(SystemsInterface&) override
	{

	}

private:

	PerWindCBData m_perWindCBData;
	ID3D11Buffer* m_pPerWindCB = nullptr;

	PerFrameCBData m_perFrameCBData;
	ID3D11Buffer* m_pPerFrameCB = nullptr;

	PerDrawCBData m_perDrawCBData;
	ID3D11Buffer* m_pPerDrawCB = nullptr;

	ShaderSet m_meshShader;

	Mesh m_floorMesh;
	Texture m_floorTexture;
	ID3D11SamplerState* m_pLinearMipSamplerState = nullptr;

	Grass m_grass;

	v3 m_position;
	f32 m_size;

	f32 circleRadius;
	float colour[4];
	int grassCount;

	int m_windDirection;
	float m_windIcrement;
};

Application g_app;

FRAMEWORK_IMPLEMENT_MAIN(g_app, "Liam Shearan - Grass Renderer - STGA 2020")