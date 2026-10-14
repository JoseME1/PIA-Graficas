#ifndef _DXRender_
#define _DXRender_

#include "Light.h"
#include "Camara.h"
#include "MCube.h"
#include "MSphere.h"
#include "MTorus.h"
#include "MCone.h"
#include "MCilinder.h"
#include "MTerrain.h"
#include "MObject.h"

class DXRender
{
public:
	//Global Declarations//
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	ID3D11RenderTargetView* renderTargetView;

	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;

	float rotPerFrame;

	Camara *camara;

	MCube *cubo;
	MSphere *esfera;
	MTorus *toroide;
	MCone *cono;
	MCilinder *cilindro;
	MTerrain *Terreno;
	MObject *Barril;

	DXRender()
	{
		rotPerFrame = 0;
	}

	//en esta funcion se inicia esas cosas del buffer desc, swapchain, render target, y dephstencil
	//estas cosas son necesarias para que se rendere la escena
	bool InitDevice(HINSTANCE hInstance, 
		HWND hwnd, int Width, int Height)
	{
		//Descripcion de nuestro buffer
		DXGI_MODE_DESC bufferDesc;

		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

		bufferDesc.Width = Width;
		bufferDesc.Height = Height;
		bufferDesc.RefreshRate.Numerator = 60;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		//Descripcion del swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc;

		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


		//Create our SwapChain
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
			D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

		//Create our BackBuffer
		ID3D11Texture2D* BackBuffer;
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

		//Create our Render Target
		d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
		BackBuffer->Release();

		//Crea la descripcion del raster, el cual determina como se dibujaran los poligonos
		D3D11_RASTERIZER_DESC rasterDesc;
		ID3D11RasterizerState* rasterState;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;//se puede manipular para transparencias, cambiar a D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0;

		d3d11Device->CreateRasterizerState(&rasterDesc, &rasterState);
		d3d11DevCon->RSSetState(rasterState);

		//descripcion del depthstencil, gracia a este hay profundidad en la escena 
		D3D11_TEXTURE2D_DESC depthStencilDesc;

		depthStencilDesc.Width     = Width;
		depthStencilDesc.Height    = Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0; 
		depthStencilDesc.MiscFlags      = 0;

		//Create the Depth/Stencil View
		d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
		d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

		//Set our Render Target
		d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

		cubo = new MCube(d3d11Device, d3d11DevCon, 2.0f, L"Texturas/crate.gif", hwnd);
		esfera = new MSphere(d3d11Device, d3d11DevCon, 16, 16, 1.0f, L"Texturas/crate.gif", hwnd);
		toroide = new MTorus(d3d11Device, d3d11DevCon, 16, 16, 1.0f, 0.5f, L"Texturas/crate.gif", hwnd);
		cono = new MCone(d3d11Device, d3d11DevCon, 16, 3, 1.0f, L"Texturas/crate.gif", hwnd);
		cilindro = new MCilinder(d3d11Device, d3d11DevCon, 16, 3, 1.0f, L"Texturas/crate.gif", hwnd);
		Terreno = new MTerrain(d3d11Device, d3d11DevCon, L"Texturas/dirt01.dds", "../DXclase1/Texturas/heightmap01.bmp", hwnd);
		Barril = new MObject(d3d11Device, d3d11DevCon, "../DXclase1/BarrilTriangle.txt", L"../DXclase1/Texturas/Barril_Diffuse.png", hwnd);
		return true;
	}

	//en el realeaseobjects, se libera el espacio en memoria, aun faltan muchas variables por poner
	void ReleaseObjects()
	{
		//Release the COM Objects we created
		SwapChain->Release();
		d3d11Device->Release();
		d3d11DevCon->Release();
	}

	//esta funcion lo unico que hace es crear el viewport y la camara
	bool InitScene(int Width, int Height)
	{
		//Create the Viewport
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = Width;
		viewport.Height = Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//Set the Viewport
		d3d11DevCon->RSSetViewports(1, &viewport);

		//Create the camera
		D3DXVECTOR3 Position = D3DXVECTOR3(0.0f, 5.0f, -30.0f);
		D3DXVECTOR3 Target = D3DXVECTOR3(0.0f, 1.0f, 1.0f);
		D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		
		camara = new Camara(Position, Target, Up, Width, Height);

		light.dir = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		light.ambient = D3DXVECTOR4(0.3f, 0.3f, 0.3f, 0.0f);
		light.diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 0.0f);
		light.specularPower = 5.0f;

		return true;
	}

	//en esta parte se modifica la transformacion de los objetos
	void UpdateScene()
	{
		rotPerFrame += 0.0001f;
		
		D3DXMATRIX rotation, moveLeft, moveRight,moveUp,identity,ScaleBarrel;
		D3DXMatrixRotationY(&rotation, rotPerFrame);
		D3DXMatrixTranslation(&moveLeft, 5, 0, 0);
		D3DXMatrixTranslation(&moveRight, -5, 0, 0);
		D3DXMatrixTranslation(&moveUp, 0, 5, 0);
		D3DXMatrixScaling(&ScaleBarrel, .05, .05, .05);
		D3DXMatrixIdentity(&identity);

		//aqui se le pasa una nueva matriz a mis objetos
		cubo->UpdateScene(rotation);
		esfera->UpdateScene(moveLeft * rotation);
		toroide->UpdateScene(moveRight * rotation);
		cono->UpdateScene(moveRight*moveRight*rotation);
		cilindro->UpdateScene(moveLeft* moveLeft*rotation);
		
		
		Barril->UpdateScene(identity*ScaleBarrel*rotation*moveUp);

		D3DXMatrixTranslation(&moveUp, 0, -5, 0);
		D3DXMatrixTranslation(&moveRight, -60, 0, 0);
		D3DXMatrixTranslation(&moveLeft, 0, 0, -50);
		Terreno->UpdateScene(identity*moveRight*moveLeft*moveUp);
	}

	//aqui se dibuja todo
	void DrawScene()
	{
		//Clear our backbuffer to the updated color
		D3DXCOLOR bgColor(0.0f, 0.0f, 0.25f, 1.0f);
		d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
		d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

		cubo->DrawScene(camara);
		esfera->DrawScene(camara);
		toroide->DrawScene(camara);
		cono->DrawScene(camara);
		cilindro->DrawScene(camara);
		Terreno->DrawScene(camara);
		Barril->DrawScene(camara);
		//Present the backbuffer to the screen
		SwapChain->Present(0, 0);
	}
};

#endif