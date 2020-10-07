#pragma once
#include "Framework.h"
#include "Mesh.h"
#include "Texture.h"
#include "ShaderSet.h"

class Grass
{
private:

	struct PerDrawCBData
	{
		m4x4 matMVP;
	};

	struct PerInstanceData
	{
		m4x4 matModel;
		v4 colour;
	};


	struct GrassType
	{
		float x, y, z;
		float r, g, b;
		float size;
	};
public:
	Grass();
	~Grass();

	void Init(SystemsInterface&, int);
	void Update(SystemsInterface&);
	void Render(ID3D11DeviceContext*,ID3D11Buffer*, SystemsInterface&);

	void UpdateGrassLocation(SystemsInterface& systems,int,float);

	PerInstanceData* m_Instances = nullptr;
	GrassType* m_grassArr = nullptr;

	ID3D11Buffer* m_perDrawCB = nullptr;
	PerDrawCBData m_perDrawCBData;

	ID3D11Buffer* m_instanceBuffer = nullptr;
	ID3D11ShaderResourceView* m_instanceBufferView = nullptr;

	ShaderSet m_instanceShaderNoData;
	ShaderSet m_instanceBufferShader;

	int m_instanceCount;

	float m_circleRadius;
	float m_windRotation;
	int m_windDirection;

	bool m_canUpdate;
	bool m_billboard;

	v4 m_grassColour;

	Mesh m_mesh;
	Texture m_texture;
	ID3D11SamplerState* m_samplerState = nullptr;

};
