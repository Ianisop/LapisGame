#pragma once

#include <windows.h>
#include <windowsx.h>
#include <D3D11.h>
#include <DirectXMath.h>

#include "LapisTypes.h"
#include "Material.h"


namespace Lapis
{
	struct LapisCommand
	{
		UINT vertexCount;
		D3D_PRIMITIVE_TOPOLOGY topology;
		Transform transform;
		std::shared_ptr<Material> material;

		LapisCommand(UINT vertexCount, D3D_PRIMITIVE_TOPOLOGY topology, Transform transform, std::shared_ptr<Material> material) :
			vertexCount(vertexCount),
			topology(topology),
			transform(transform),
			material(material)
		{}

		LapisCommand(UINT vertexCount, D3D_PRIMITIVE_TOPOLOGY topology, std::shared_ptr<Material> material) :
			vertexCount(vertexCount),
			topology(topology),
			transform(Transform()),
			material(material)
		{}
	};

	struct InternalLapisCommand
	{
		UINT vertexCount;
		UINT startVertexLocation;
		D3D_PRIMITIVE_TOPOLOGY topology;
		Transform transform;
		std::shared_ptr<Material> material;

		InternalLapisCommand(LapisCommand drawCommand, UINT startVertexLocation)
			: vertexCount(drawCommand.vertexCount),
			startVertexLocation(startVertexLocation),
			topology(drawCommand.topology),
			transform(drawCommand.transform),
			material(drawCommand.material)
		{}
	};

	_declspec(align(16))
		struct GlobalConstantBuffer
	{
		float elapsedTime;
		float deltaTime;
		DirectX::XMMATRIX Screen;
		DirectX::XMMATRIX Model;
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};
}