#pragma once

#include <windows.h>
#include <windowsx.h>
#include <D3D11.h>

struct VERTEX
{
	FLOAT x, y, z;
	DXGI_RGBA color;
};

class Rect2D
{
	int x, y, z, w;

};

struct LapisCommand
{
	UINT IndexCount;
	UINT StartIndexLocation;
	INT BaseVertexLocation;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType;
};