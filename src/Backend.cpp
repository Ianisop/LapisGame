#include "Backend.h"

#include <iostream>
#include <chrono>
#include <DirectXMath.h>

namespace Lapis
{
    
    void LapisInstance::Init()
    {
        this->initDuration = std::chrono::high_resolution_clock::now().time_since_epoch();
    }

    void LapisInstance::InitD3D11(HWND hwnd)
    {

        DXGI_SWAP_CHAIN_DESC scd;

        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

        scd.BufferCount = 1;                                    // one back buffer
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
        scd.OutputWindow = hwnd;                                // the window to be used
        scd.SampleDesc.Count = 4;                               // how many multisamples
        scd.Windowed = TRUE;                                    // windowed/full-screen mode
        scd.BufferDesc.Width = SCREEN_WIDTH;
        scd.BufferDesc.Height = SCREEN_HEIGHT;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // create a device, device context and swap chain using the information in the scd struct
        D3D11CreateDeviceAndSwapChain(NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            NULL,
            NULL,
            NULL,
            D3D11_SDK_VERSION,
            &scd,
            &this->swapchain,
            &this->device,
            NULL,
            &this->deviceContext);

        // get the address of the back buffer
        ID3D11Texture2D* pBackBuffer = nullptr;
        this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

        // use the back buffer address to create the render target
        this->device->CreateRenderTargetView(pBackBuffer, NULL, &this->backbuffer);
        pBackBuffer->Release();

        // set the render target as the back buffer
        this->deviceContext->OMSetRenderTargets(1, &this->backbuffer, NULL);

        // Set the viewport
        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = SCREEN_WIDTH;
        viewport.Height = SCREEN_HEIGHT;

        this->deviceContext->RSSetViewports(1, &viewport);

        InitPipeline();
    }

    void LapisInstance::InitPipeline()
    {
        // load and compile the two shaders
        ID3DBlob* VS, * PS;
        D3DCompileFromFile(L"src/unlit.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &VS, 0);
        D3DCompileFromFile(L"src/unlit.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &PS, 0);

        // encapsulate both shaders into shader objects
        this->device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &this->pVS_unlit);
        this->device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &this->pPS_unlit);

        // set the shader objects
        this->deviceContext->VSSetShader(this->pVS_unlit, 0, 0);
        this->deviceContext->PSSetShader(this->pPS_unlit, 0, 0);

        D3D11_INPUT_ELEMENT_DESC ied[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        this->device->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &this->pLayout);
        this->deviceContext->IASetInputLayout(this->pLayout);

        
        // Supply the vertex shader constant data.
        VS_CONSTANT_BUFFER VsConstData;
        VsConstData.fTime = 1.7f;

        // Fill in a buffer description.
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Fill in the subresource data.
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = &VsConstData;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        // Create the buffer.
        auto hr = this->device->CreateBuffer(&cbDesc, &InitData, &this->pConstantBuffer);
        std::cout << "CreateBuffer - cbuffer: " << hr << "\n";
        
        if (FAILED(hr))
            return;


        this->deviceContext->VSSetConstantBuffers(0, 1, &this->pConstantBuffer);

    }

    void LapisInstance::BeginFrame() {
        static auto old = initDuration;

        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        this->deltaDuration = (now - old);
        this->elapsedDuration = (now - initDuration);
        old = now;

        this->deltaTime = (float)std::chrono::duration_cast<std::chrono::microseconds>(this->deltaDuration).count() / 1000 / 100;
        this->elapsedTime += this->deltaTime;
    }

    void LapisInstance::CleanFrame() {
        
        this->vertexBuffer.clear();
        this->VerticeCount = 0;

        this->commandList.clear();
    }

    // this is the function used to render a single frame
    void LapisInstance::RenderFrame()
    {
        static VS_CONSTANT_BUFFER cb0;
        cb0.fTime = this->elapsedTime;

        {
            D3D11_MAPPED_SUBRESOURCE ms;
            this->deviceContext->Map(this->pConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
            memcpy(ms.pData, &cb0, sizeof(cb0));
            this->deviceContext->Unmap(this->pConstantBuffer, NULL);
        }

        
        static int VBufferSize = 0;
        if (this->VBufferCapacity > VBufferSize) {

            if (this->pVBuffer)
                this->pVBuffer->Release();

            std::cout << "resizing back buffer\n";
            // create the vertex buffer
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));

            bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
            bd.ByteWidth = sizeof(VERTEX) * this->VBufferCapacity;             // size is the VERTEX struct * 3
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
        
            

            this->device->CreateBuffer(&bd, NULL, &this->pVBuffer);       // create the buffer
            if (!this->pVBuffer)
                return;

            VBufferSize = this->VBufferCapacity;
        }
        
        

        // copy the vertices into the buffer
        D3D11_MAPPED_SUBRESOURCE ms;
        this->deviceContext->Map(this->pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, this->vertexBuffer.data(), this->vertexBuffer.size() * sizeof(VERTEX));
        this->deviceContext->Unmap(this->pVBuffer, NULL);
        
        static float h = 0;
        h += this->deltaTime;
        if (h > 360) h -= 360;
        auto color = HSLToRGB((int)h, 1.0f, 0.95f, 1.0f);
        this->deviceContext->ClearRenderTargetView(this->backbuffer, (FLOAT*)&color);


        UINT stride = sizeof(VERTEX);
        UINT offset = 0;
        this->deviceContext->IASetVertexBuffers(0, 1, &this->pVBuffer, &stride, &offset);
        
        for (auto& command : this->commandList) {
            this->deviceContext->IASetPrimitiveTopology(command.TopologyType);
            this->deviceContext->Draw(command.VertexCount, command.StartVertexLocation);
        }

        this->swapchain->Present(0, 0);
    }

    void LapisInstance::PushCommand(int VerticeCount, D3D_PRIMITIVE_TOPOLOGY Topology) {
        this->commandList.push_back(LapisCommand(VerticeCount, this->VerticeCount, Topology));
    }

    void LapisInstance::PushVertex(float x, float y, DXGI_RGBA col) {
        if (this->VerticeCount + 1 > this->VBufferCapacity) {
            this->VBufferCapacity += 1000;
            this->vertexBuffer.reserve(this->VBufferCapacity);
        }
        this->vertexBuffer.push_back(VERTEX(x, y, 0, col));
//        this->vertexBuffer.at(this->VerticeCount) = (VERTEX(x, y, 0, col));
        this->VerticeCount += 1;
    }

    void LapisInstance::CleanD3D11()
    {
        this->swapchain->SetFullscreenState(0, NULL);

        this->pLayout->Release();
        this->pVS_unlit->Release();
        this->pPS_unlit->Release();
        this->pVBuffer->Release();
        this->swapchain->Release();
        this->backbuffer->Release();
        this->device->Release();
        this->deviceContext->Release();
    }
    bool LapisInstance::GetInput(int key)
    {
        return (GetAsyncKeyState(key) & 0x8000) != 0;

    }
}

