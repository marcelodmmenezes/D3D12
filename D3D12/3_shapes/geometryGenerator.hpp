#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <vector>

class GeometryGenerator
{
public:
	struct Vertex
	{
		Vertex()
		{

		}

		Vertex(
			DirectX::XMFLOAT3 const& p,
			DirectX::XMFLOAT3 const& n,
			DirectX::XMFLOAT3 const& t,
			DirectX::XMFLOAT2 const& uv)
			:
			position(p),
			normal(n),
			tangent_u(t),
			tex_c(uv)
		{

		}

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			:
			position(px, py, pz),
			normal(nx, ny, nz),
			tangent_u(tx, ty, tz),
			tex_c(u, v)
		{

		}

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent_u;
		DirectX::XMFLOAT2 tex_c;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<std::uint32_t> indices_32;

		std::vector<std::uint16_t>& getIndices16()
		{
			if (indices_16.empty())
			{
				indices_16.resize(indices_32.size());

				for (size_t i = 0; i < indices_32.size(); ++i)
				{
					indices_16[i] = static_cast<std::uint16_t>(indices_32[i]);
				}
			}

			return indices_16;
		}

	private:
		std::vector<std::uint16_t> indices_16;
	};

	MeshData createBox(
		float width,
		float height,
		float depth,
		std::uint32_t n_subdivisions);

	MeshData createSphere(
		float radius,
		std::uint32_t slice_count,
		std::uint32_t stack_count);

	MeshData createGeosphere(
		float radius,
		std::uint32_t n_subdivisions);

	MeshData createCylinder(
		float bottom_radius,
		float top_radius,
		float height,
		std::uint32_t slice_count,
		std::uint32_t stack_count);

	MeshData createGrid(
		float width,
		float depth,
		std::uint32_t m,
		std::uint32_t n);

	MeshData createQuad(
		float x,
		float y,
		float w,
		float h,
		float depth);

private:
	void subdivide(MeshData& mesh_data);
	Vertex midPoint(Vertex const& v0, Vertex const& v1);

	void buildCylinderTopCap(
		float bottom_radius,
		float top_radius,
		float height,
		std::uint32_t slice_count,
		std::uint32_t stack_count,
		MeshData& mesh_data);

	void buildCylinderBottomCap(
		float bottom_radius,
		float top_radius,
		float height,
		std::uint32_t slice_count,
		std::uint32_t stack_count,
		MeshData& mesh_data);
};
