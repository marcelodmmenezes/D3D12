#pragma once

#include "config.hpp"
#include "math.hpp"

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 world_view_proj = identity4x4();
};

D3D12_INPUT_ELEMENT_DESC input_layout[]
{
	{
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	},
	{
		"COLOR",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		offsetof(Vertex, color),
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	}
};

Vertex vertices[]
{
	{
		DirectX::XMFLOAT3{ -1.0f, -1.0f, -1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::White }
	},
	{
		DirectX::XMFLOAT3{ -1.0f, +1.0f, -1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Black }
	},
	{
		DirectX::XMFLOAT3{ +1.0f, +1.0f, -1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Red }
	},
	{
		DirectX::XMFLOAT3{ +1.0f, -1.0f, -1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Green }
	},
	{
		DirectX::XMFLOAT3{ -1.0f, -1.0f, +1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Blue }
	},
	{
		DirectX::XMFLOAT3{ -1.0f, +1.0f, +1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Yellow }
	},
	{
		DirectX::XMFLOAT3{ +1.0f, +1.0f, +1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Cyan }
	},
	{
		DirectX::XMFLOAT3{ +1.0f, -1.0f, +1.0f },
		DirectX::XMFLOAT4{ DirectX::Colors::Magenta }
	}
};

uint16_t indices[]
{
	0, 1, 2,
	0, 2, 3,
	4, 6, 5,
	4, 7, 6,
	4, 5, 1,
	4, 1, 0,
	3, 2, 6,
	3, 6, 7,
	1, 5, 6,
	1, 6, 2,
	4, 0, 3,
	4, 3, 7
};
