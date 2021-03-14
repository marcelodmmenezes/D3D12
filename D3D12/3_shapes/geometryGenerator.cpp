#include "geometryGenerator.hpp"

#include <algorithm>

using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::createBox(
	float width,
	float height,
	float depth,
	std::uint32_t n_subdivisions)
{
	MeshData mesh_data;

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	v[8] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	mesh_data.vertices.assign(&v[0], &v[24]);

	std::uint32_t i[36]
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	mesh_data.indices_32.assign(&i[0], &i[36]);

	n_subdivisions = std::min<std::uint32_t>(n_subdivisions, 6u);

	for (std::uint32_t i = 0; i < n_subdivisions; ++i)
	{
		subdivide(mesh_data);
	}

	return mesh_data;
}

GeometryGenerator::MeshData GeometryGenerator::createSphere(
	float radius,
	std::uint32_t slice_count,
	std::uint32_t stack_count)
{
	MeshData mesh_data;

	Vertex top_vertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottom_vertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	mesh_data.vertices.emplace_back(top_vertex);

	float phi_step = XM_PI / stack_count;
	float theta_step = 2.0f * XM_PI / slice_count;

	for (std::uint32_t i = 1; i <= stack_count - 1; ++i)
	{
		float phi = i * phi_step;

		for (std::uint32_t j = 0; j <= slice_count; ++j)
		{
			float theta = j * theta_step;

			Vertex v;

			// spherical to cartesian
			v.position.x = radius * sinf(phi) * cosf(theta);
			v.position.y = radius * cosf(phi);
			v.position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.tangent_u.x = -radius * sinf(phi) * sinf(theta);
			v.tangent_u.y = 0.0f;
			v.tangent_u.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR t = XMLoadFloat3(&v.tangent_u);
			XMStoreFloat3(&v.tangent_u, XMVector3Normalize(t));

			XMVECTOR p = XMLoadFloat3(&v.position);
			XMStoreFloat3(&v.normal, XMVector3Normalize(p));

			v.tex_c.x = theta / XM_2PI;
			v.tex_c.y = phi / XM_PI;

			mesh_data.vertices.push_back(v);
		}
	}

	mesh_data.vertices.push_back(bottom_vertex);

	for (std::uint32_t i = 1; i <= slice_count; ++i)
	{
		mesh_data.indices_32.push_back(0);
		mesh_data.indices_32.push_back(i + 1);
		mesh_data.indices_32.push_back(i);
	}

	std::uint32_t base_index = 1;
	std::uint32_t ring_vertex_count = slice_count + 1;
	for (std::uint32_t i = 0; i < stack_count - 2; ++i)
	{
		for (std::uint32_t j = 0; j < slice_count; ++j)
		{
			mesh_data.indices_32.push_back(base_index + i * ring_vertex_count + j);
			mesh_data.indices_32.push_back(base_index + i * ring_vertex_count + j + 1);
			mesh_data.indices_32.push_back(base_index + (i + 1) * ring_vertex_count + j);

			mesh_data.indices_32.push_back(base_index + (i + 1) * ring_vertex_count + j);
			mesh_data.indices_32.push_back(base_index + i * ring_vertex_count + j + 1);
			mesh_data.indices_32.push_back(base_index + (i + 1) * ring_vertex_count + j + 1);
		}
	}

	std::uint32_t south_pole_index = (std::uint32_t)mesh_data.vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	base_index = south_pole_index - ring_vertex_count;

	for (std::uint32_t i = 0; i < slice_count; ++i)
	{
		mesh_data.indices_32.push_back(south_pole_index);
		mesh_data.indices_32.push_back(base_index + i);
		mesh_data.indices_32.push_back(base_index + i + 1);
	}

	return mesh_data;
}

void GeometryGenerator::subdivide(MeshData& mesh_data)
{
	MeshData input_copy = mesh_data;


	mesh_data.vertices.resize(0);
	mesh_data.indices_32.resize(0);

	std::uint32_t n_tris = (std::uint32_t)input_copy.indices_32.size() / 3;

	for (std::uint32_t i = 0; i < n_tris; ++i)
	{
		Vertex v0 = input_copy.vertices[input_copy.indices_32[i * 3 + 0]];
		Vertex v1 = input_copy.vertices[input_copy.indices_32[i * 3 + 1]];
		Vertex v2 = input_copy.vertices[input_copy.indices_32[i * 3 + 2]];

		Vertex m0 = midPoint(v0, v1);
		Vertex m1 = midPoint(v1, v2);
		Vertex m2 = midPoint(v0, v2);

		mesh_data.vertices.push_back(v0);
		mesh_data.vertices.push_back(v1);
		mesh_data.vertices.push_back(v2);
		mesh_data.vertices.push_back(m0);
		mesh_data.vertices.push_back(m1);
		mesh_data.vertices.push_back(m2);

		mesh_data.indices_32.push_back(i * 6 + 0);
		mesh_data.indices_32.push_back(i * 6 + 3);
		mesh_data.indices_32.push_back(i * 6 + 5);

		mesh_data.indices_32.push_back(i * 6 + 3);
		mesh_data.indices_32.push_back(i * 6 + 4);
		mesh_data.indices_32.push_back(i * 6 + 5);

		mesh_data.indices_32.push_back(i * 6 + 5);
		mesh_data.indices_32.push_back(i * 6 + 4);
		mesh_data.indices_32.push_back(i * 6 + 2);

		mesh_data.indices_32.push_back(i * 6 + 3);
		mesh_data.indices_32.push_back(i * 6 + 1);
		mesh_data.indices_32.push_back(i * 6 + 4);
	}
}

GeometryGenerator::Vertex GeometryGenerator::midPoint(
	Vertex const& v0,
	Vertex const& v1)
{
	XMVECTOR p0 = XMLoadFloat3(&v0.position);
	XMVECTOR p1 = XMLoadFloat3(&v1.position);

	XMVECTOR n0 = XMLoadFloat3(&v0.normal);
	XMVECTOR n1 = XMLoadFloat3(&v1.normal);

	XMVECTOR tan0 = XMLoadFloat3(&v0.tangent_u);
	XMVECTOR tan1 = XMLoadFloat3(&v1.tangent_u);

	XMVECTOR tex0 = XMLoadFloat2(&v0.tex_c);
	XMVECTOR tex1 = XMLoadFloat2(&v1.tex_c);

	XMVECTOR pos = 0.5f * (p0 + p1);
	XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
	XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
	XMVECTOR tex = 0.5f * (tex0 + tex1);

	Vertex v;
	XMStoreFloat3(&v.position, pos);
	XMStoreFloat3(&v.normal, normal);
	XMStoreFloat3(&v.tangent_u, tangent);
	XMStoreFloat2(&v.tex_c, tex);

	return v;
}

GeometryGenerator::MeshData GeometryGenerator::createGeosphere(
	float radius,
	std::uint32_t n_subdivisions)
{
	MeshData mesh_data;

	n_subdivisions = std::min<std::uint32_t>(n_subdivisions, 6u);

	float const x = 0.525731f;
	float const z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-x, 0.0f, z),  XMFLOAT3(x, 0.0f, z),
		XMFLOAT3(-x, 0.0f, -z), XMFLOAT3(x, 0.0f, -z),
		XMFLOAT3(0.0f, z, x),   XMFLOAT3(0.0f, z, -x),
		XMFLOAT3(0.0f, -z, x),  XMFLOAT3(0.0f, -z, -x),
		XMFLOAT3(z, x, 0.0f),   XMFLOAT3(-z, x, 0.0f),
		XMFLOAT3(z, -x, 0.0f),  XMFLOAT3(-z, -x, 0.0f)
	};

	std::uint32_t k[60] =
	{
		1, 4, 0,
		4, 9, 0,
		4, 5, 9,
		8, 5, 4,
		1, 8, 4,

		1, 10, 8,
		0, 3, 8,
		8, 3, 5,
		3, 2, 5,
		3, 7, 2,

		3, 10, 7,
		10, 6, 7,
		6, 11, 7,
		6, 0, 11,
		6, 1, 0,

		10, 1, 6,
		11, 0, 9,
		2, 11, 9,
		5, 2, 9,
		11, 2, 7
	};

	mesh_data.vertices.resize(12);
	mesh_data.indices_32.assign(&k[0], &k[60]);

	for (std::uint32_t i = 0; i < 12; ++i)
		mesh_data.vertices[i].position = pos[i];

	for (std::uint32_t i = 0; i < n_subdivisions; ++i)
		subdivide(mesh_data);

	for (std::uint32_t i = 0; i < mesh_data.vertices.size(); ++i)
	{
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mesh_data.vertices[i].position));
		XMVECTOR p = radius * n;

		XMStoreFloat3(&mesh_data.vertices[i].position, p);
		XMStoreFloat3(&mesh_data.vertices[i].normal, n);

		float theta = atan2f(mesh_data.vertices[i].position.z, mesh_data.vertices[i].position.x);

		if (theta < 0.0f)
		{
			theta += XM_2PI;
		}

		float phi = acosf(mesh_data.vertices[i].position.y / radius);

		mesh_data.vertices[i].tex_c.x = theta / XM_2PI;
		mesh_data.vertices[i].tex_c.y = phi / XM_PI;

		mesh_data.vertices[i].tangent_u.x = -radius * sinf(phi) * sinf(theta);
		mesh_data.vertices[i].tangent_u.y = 0.0f;
		mesh_data.vertices[i].tangent_u.z = +radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&mesh_data.vertices[i].tangent_u);
		XMStoreFloat3(&mesh_data.vertices[i].tangent_u, XMVector3Normalize(T));
	}

	return mesh_data;
}

GeometryGenerator::MeshData GeometryGenerator::createCylinder(
	float bottom_radius,
	float top_radius,
	float height,
	std::uint32_t slice_count,
	std::uint32_t stack_count)
{
	MeshData mesh_data;

	float stack_height = height / stack_count;
	float radius_step = (top_radius - bottom_radius) / stack_count;

	std::uint32_t ring_count = stack_count + 1;

	for (std::uint32_t i = 0; i < ring_count; ++i)
	{
		float y = -0.5f * height + i * stack_height;
		float r = bottom_radius + i * radius_step;

		float d_theta = 2.0f * XM_PI / slice_count;

		for (std::uint32_t j = 0; j <= slice_count; ++j)
		{
			Vertex vertex;

			float c = cosf(j * d_theta);
			float s = sinf(j * d_theta);

			vertex.position = XMFLOAT3(r * c, y, r * s);

			vertex.tex_c.x = (float)j / slice_count;
			vertex.tex_c.y = 1.0f - (float)i / stack_count;

			vertex.tangent_u = XMFLOAT3(-s, 0.0f, c);

			float dr = bottom_radius - top_radius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.tangent_u);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.normal, N);

			mesh_data.vertices.push_back(vertex);
		}
	}

	std::uint32_t ring_vertex_count = slice_count + 1;

	for (std::uint32_t i = 0; i < stack_count; ++i)
	{
		for (std::uint32_t j = 0; j < slice_count; ++j)
		{
			mesh_data.indices_32.push_back(i * ring_vertex_count + j);
			mesh_data.indices_32.push_back((i + 1) * ring_vertex_count + j);
			mesh_data.indices_32.push_back((i + 1) * ring_vertex_count + j + 1);

			mesh_data.indices_32.push_back(i * ring_vertex_count + j);
			mesh_data.indices_32.push_back((i + 1) * ring_vertex_count + j + 1);
			mesh_data.indices_32.push_back(i * ring_vertex_count + j + 1);
		}
	}

	buildCylinderTopCap(bottom_radius, top_radius, height, slice_count, stack_count, mesh_data);
	buildCylinderBottomCap(bottom_radius, top_radius, height, slice_count, stack_count, mesh_data);

	return mesh_data;
}

void GeometryGenerator::buildCylinderTopCap(
	float bottom_radius,
	float top_radius,
	float height,
	std::uint32_t slice_count,
	std::uint32_t stackCount,
	MeshData& meshData)
{
	std::uint32_t base_index = (std::uint32_t)meshData.vertices.size();

	float y = 0.5f * height;
	float d_theta = 2.0f * XM_PI / slice_count;

	for (std::uint32_t i = 0; i <= slice_count; ++i)
	{
		float x = top_radius * cosf(i * d_theta);
		float z = top_radius * sinf(i * d_theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	meshData.vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	std::uint32_t center_index = (std::uint32_t)meshData.vertices.size() - 1;

	for (std::uint32_t i = 0; i < slice_count; ++i)
	{
		meshData.indices_32.push_back(center_index);
		meshData.indices_32.push_back(base_index + i + 1);
		meshData.indices_32.push_back(base_index + i);
	}
}

void GeometryGenerator::buildCylinderBottomCap(
	float bottom_radius,
	float top_radius,
	float height,
	std::uint32_t slice_count,
	std::uint32_t stackCount,
	MeshData& mesh_data)
{
	std::uint32_t base_index = (std::uint32_t)mesh_data.vertices.size();

	float y = -0.5f * height;
	float d_theta = 2.0f * XM_PI / slice_count;

	for (std::uint32_t i = 0; i <= slice_count; ++i)
	{
		float x = bottom_radius * cosf(i * d_theta);
		float z = bottom_radius * sinf(i * d_theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		mesh_data.vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	mesh_data.vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	std::uint32_t centerIndex = (std::uint32_t)mesh_data.vertices.size() - 1;

	for (std::uint32_t i = 0; i < slice_count; ++i)
	{
		mesh_data.indices_32.push_back(centerIndex);
		mesh_data.indices_32.push_back(base_index + i);
		mesh_data.indices_32.push_back(base_index + i + 1);
	}
}

GeometryGenerator::MeshData GeometryGenerator::createGrid(
	float width,
	float depth,
	std::uint32_t m,
	std::uint32_t n)
{
	MeshData mesh_data;

	std::uint32_t vertex_count = m * n;
	std::uint32_t face_count = (m - 1) * (n - 1) * 2;

	float half_width = 0.5f * width;
	float half_depth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	mesh_data.vertices.resize(vertex_count);

	for (std::uint32_t i = 0; i < m; ++i)
	{
		float z = half_depth - i * dz;

		for (std::uint32_t j = 0; j < n; ++j)
		{
			float x = -half_width + j * dx;

			mesh_data.vertices[i * n + j].position = XMFLOAT3(x, 0.0f, z);
			mesh_data.vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			mesh_data.vertices[i * n + j].tangent_u= XMFLOAT3(1.0f, 0.0f, 0.0f);

			mesh_data.vertices[i * n + j].tex_c.x = j * du;
			mesh_data.vertices[i * n + j].tex_c.y = i * dv;
		}
	}

	mesh_data.indices_32.resize(face_count * 3);

	std::uint32_t k = 0;

	for (std::uint32_t i = 0; i < m - 1; ++i)
	{
		for (std::uint32_t j = 0; j < n - 1; ++j)
		{
			mesh_data.indices_32[k] = i * n + j;
			mesh_data.indices_32[k + 1] = i * n + j + 1;
			mesh_data.indices_32[k + 2] = (i + 1) * n + j;

			mesh_data.indices_32[k + 3] = (i + 1) * n + j;
			mesh_data.indices_32[k + 4] = i * n + j + 1;
			mesh_data.indices_32[k + 5] = (i + 1) * n + j + 1;

			k += 6;
		}
	}

	return mesh_data;
}

GeometryGenerator::MeshData GeometryGenerator::createQuad(
	float x,
	float y,
	float w,
	float h,
	float depth)
{
	MeshData mesh_data;

	mesh_data.vertices.resize(4);
	mesh_data.indices_32.resize(6);

	mesh_data.vertices[0] = Vertex(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	mesh_data.vertices[1] = Vertex(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	mesh_data.vertices[2] = Vertex(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	mesh_data.vertices[3] = Vertex(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	mesh_data.indices_32[0] = 0;
	mesh_data.indices_32[1] = 1;
	mesh_data.indices_32[2] = 2;

	mesh_data.indices_32[3] = 0;
	mesh_data.indices_32[4] = 2;
	mesh_data.indices_32[5] = 3;

	return mesh_data;
}
