#pragma once
#include "Shader.h"

class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

class CWaterShader : public CTerrainShader
{
public:
	CWaterShader() {};
	virtual ~CWaterShader() {};
	virtual D3D12_BLEND_DESC CreateBlendState();
};