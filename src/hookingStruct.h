#pragma once
#include <DirectXMath.h>

struct RTVertex
{
	DirectX::XMFLOAT3 position;  // 位置
	DirectX::XMFLOAT2 texcoord;  // 纹理坐标
};

struct Vertex
{
	float x, y, z;  // 位置
	float r, g, b;  // 颜色
};

struct BufferInfo
{
	UINT stride;
	UINT offset;
	D3D11_BUFFER_DESC desc;
};


struct HookInfo
{
	UINT index;
	void* hook;
	void** original;
	const char* name;  // 用于日志
};
	

// 顶点数据
Vertex gdc_Vertices[] = {
	{ -1.0f, -1.0f, 0.0f, 0.0f, 0.0f},  // 左下角
	{ -1.0f, 3.0f, 0.0f, 0.0f, 2.0f},   // 左上角
	{ 3.0f, -1.0f, 0.0f, 2.0f, 0.0f}    // 右下角
};

unsigned int gdc_Indices[] = {
	0, 1, 2  // 第一个三角形
};

unsigned int plane_Indices[] = {
	0, 1, 2,
	2, 3, 0,
	1,4,5,
	5,2,1,
	3,2,6,
	6,7,3,
	2,5,8,
	8,6,2
};

D3D11_INPUT_ELEMENT_DESC gdc_layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  // 位置元素描述
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // 颜色元素描述
};

