#ifndef _DXRender_
#define _DXRender_

#include "Constants.h"

#include "Camara.h"
#include "Light.h"
#include "MeshPrimitives.h"

#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

class DXRender
{
private:
	//Global Declarations//
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;

	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	D3DXMATRIX View;
	D3DXMATRIX Projection;


	MPrimitives *skydomo;
	MPrimitives *terreno;
	MPrimitives *Bitmap;

	//MIS PUTOS MODELOS
	MPrimitives *arbol;
	MPrimitives* barril;
	MPrimitives* gato;
	MPrimitives* tumba;
	MPrimitives* tree2;
	MPrimitives* ataud;
	MPrimitives* calaca;
	MPrimitives* calaca2;
	MPrimitives* castillo;
	MPrimitives* arbolmuerto1;
	MPrimitives* arbolmuerto2;

	Camara *camara;
	Camara *skyCamara;

	float rotPerFrame;
	float sphereSpeed;

public:
	DXRender()
	{
		rotPerFrame = 0.0f;
	}

	bool InitDevice(HWND hwnd, int Width, int Height)
	{
		//Describe our Buffer
		DXGI_MODE_DESC bufferDesc;

		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

		bufferDesc.Width = Width;
		bufferDesc.Height = Height;
		bufferDesc.RefreshRate.Numerator = 60;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		//Describe our SwapChain
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

		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ID3D11Texture2D* depthStencilBuffer;

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

		//Mis Modelos
		arbol = new MPrimitives(d3d11Device, d3d11DevCon, "Models/tree.txt", L"Texturas/tronco.jpg", L"Texturas/tronco.jpg", ShaderDiffuse);
		barril = new MPrimitives(d3d11Device, d3d11DevCon, "Models/barril.txt", L"Texturas/barrelcrateColor.png", L"Texturas/barrelcrateNormal.png", ShaderNormalmap);
		gato = new MPrimitives(d3d11Device, d3d11DevCon, "Models/gato.txt", L"Texturas/gatoColor.png", L"", ShaderDiffuse);
		tumba = new MPrimitives(d3d11Device, d3d11DevCon, "Models/tumba.txt", L"Texturas/tumbaColor.png", L"", ShaderDiffuse);
		tree2 = new MPrimitives(d3d11Device, d3d11DevCon, "Models/tree2.txt", L"Texturas/tree2Color.png", L"", ShaderDiffuse);
		ataud = new MPrimitives(d3d11Device, d3d11DevCon, "Models/ataud.txt", L"Texturas/ataudColor.png", L"", ShaderDiffuse);
		calaca = new MPrimitives(d3d11Device, d3d11DevCon, "Models/calaca.txt", L"Texturas/calacaColor.png", L"", ShaderDiffuse);
		calaca2 = new MPrimitives(d3d11Device, d3d11DevCon, "Models/calaca2.txt", L"Texturas/calaca2Color.png", L"", ShaderDiffuse);
		castillo = new MPrimitives(d3d11Device, d3d11DevCon, "Models/castillo.txt", L"Texturas/castilloColor.png", L"", ShaderDiffuse);
		arbolmuerto1 = new MPrimitives(d3d11Device, d3d11DevCon, "Models/arbolmuerto1.txt", L"Texturas/arbolmuerto1Color.jpg", L"", ShaderDiffuse);
		arbolmuerto2 = new MPrimitives(d3d11Device, d3d11DevCon, "Models/arbolmuerto2.txt", L"Texturas/arbolmuerto2Color.jpg", L"", ShaderDiffuse);


		//terreno
		skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/sky.jpg", L"Texturas/escudoNormal.jpg", ShaderDiffuse);
		terreno = new MPrimitives(d3d11Device, d3d11DevCon, 100.0f, L"Texturas/rock.jpg", L"Texturas/grass.jpg", "Texturas/terreno.bmp", ShaderMultitextura);
		
		return true;
	}

	bool InitDirectInput(HINSTANCE hInstance, HWND hwnd)
	{
		DirectInput8Create(hInstance,
			DIRECTINPUT_VERSION,
			IID_IDirectInput8,
			(void**)&DirectInput,
			NULL); 

		DirectInput->CreateDevice(GUID_SysKeyboard,
			&DIKeyboard,
			NULL);

		DirectInput->CreateDevice(GUID_SysMouse,
			&DIMouse,
			NULL);

		DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
		DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

		DIMouse->SetDataFormat(&c_dfDIMouse);
		DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

		return true;
	}

	void DetectInput()
	{
		DIMOUSESTATE mouseCurrState;

		BYTE keyboardState[256];

		DIKeyboard->Acquire();
		DIMouse->Acquire();

		DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

		DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);

		float speed = 0.01f;

		float moveLeftRight = 0;
		float moveBackForward = 0;

		float camYaw = 0;
		float camPitch = 0;

		

		if(keyboardState[DIK_A])
		{
			moveLeftRight -= speed;
		}
		if(keyboardState[DIK_D])
		{
			moveLeftRight += speed;
		}
		if(keyboardState[DIK_W])
		{
			moveBackForward += speed;
		}
		if(keyboardState[DIK_S])
		{
			moveBackForward -= speed;
		}

		if((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
		{
			float smoothness = 0.001f;
			camYaw = mouseCurrState.lX * smoothness;

			camPitch = mouseCurrState.lY * smoothness;

			mouseLastState = mouseCurrState;
		}

		//Control de Xbox (XInput)
		XINPUT_STATE padState;
		ZeroMemory(&padState, sizeof(XINPUT_STATE));
		if (XInputGetState(0, &padState) == ERROR_SUCCESS)
		{
			XINPUT_GAMEPAD pad = padState.Gamepad;

			
			float normLX = 0.0f;
			float normLY = 0.0f;
			if (pad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || pad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				normLX = (float)pad.sThumbLX / 32767.0f;
			if (pad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || pad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				normLY = (float)pad.sThumbLY / 32767.0f;

			float padSpeed = (pad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? speed * 3.0f : speed;
			moveLeftRight += normLX * padSpeed;
			moveBackForward += normLY * padSpeed;

			
			if (pad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || pad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				camYaw += (float)pad.sThumbRX / 32767.0f * 0.008f;
			if (pad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || pad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				camPitch -= (float)pad.sThumbRY / 32767.0f * 0.008f;

			
			if (pad.wButtons & XINPUT_GAMEPAD_START)
				PostQuitMessage(0);
		}

		camara->UpdateCamera(moveBackForward, moveLeftRight, camYaw, camPitch); 
		skyCamara->setSkyCamara(camara);
	}

	void ReleaseObjects()
	{
		//Release the COM Objects we created
		SwapChain->Release();
		d3d11Device->Release();
		d3d11DevCon->Release();
		renderTargetView->Release();
		depthStencilView->Release();
	}

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

		//set light values
		light.direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		light.ambient = D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
		light.diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

		//Set the Viewport
		d3d11DevCon->RSSetViewports(1, &viewport);

		//Create the camera
		D3DXVECTOR3 Position = D3DXVECTOR3(50, 2.0f, 40.0f);
		D3DXVECTOR3 Target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		camara = new Camara(Position,Target,Up, Width, Height);
		skyCamara = new Camara(Position,Target,Up, Width, Height);
		//esta es la gui
		Bitmap = new MPrimitives(d3d11Device, d3d11DevCon, 1, 1, L"Texturas/Health.png", camara);

		return true;
	}

	void UpdateScene()
	{
		rotPerFrame += 0.1f;

		//Mis Modelos
		D3DXMATRIX escalaArbol, lugarArbol;
		D3DXMatrixScaling(&escalaArbol, 2.0f, 2.0f, 2.0f);
		D3DXMatrixTranslation(&lugarArbol, 45.0f, 0.0f, 45.0f);

		D3DXMATRIX escalaBarril, lugarBarril;
		D3DXMatrixScaling(&escalaBarril, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzBarril(25.0f, 25.0f);
		float yBarril = terreno->getTerrainCollision(xzBarril) - 0.023f * 2.0f + 0.05f;
		D3DXMatrixTranslation(&lugarBarril, xzBarril.x, yBarril, xzBarril.y);

		D3DXMATRIX escalaGato, lugarGato;
		D3DXMatrixScaling(&escalaGato, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzGato(35.0f, 35.0f);
		float yGato = terreno->getTerrainCollision(xzGato) + 0.05f;
		D3DXMatrixTranslation(&lugarGato, xzGato.x, yGato, xzGato.y);

		D3DXMATRIX escalaTumba, lugarTumba;
		D3DXMatrixScaling(&escalaTumba, 2.5f, 2.5f, 2.5f);
		D3DXVECTOR2 xzTumba(30.0f, 35.0f);
		float yTumba = terreno->getTerrainCollision(xzTumba) + 0.05f;
		D3DXMatrixTranslation(&lugarTumba, xzTumba.x, yTumba, xzTumba.y);

		D3DXMATRIX escalaTree2, lugarTree2;
		D3DXMatrixScaling(&escalaTree2, 1.5f, 1.5f, 1.5f);
		D3DXVECTOR2 xzTree2(40.0f, 30.0f);
		float yTree2 = terreno->getTerrainCollision(xzTree2) + 0.05f;
		D3DXMatrixTranslation(&lugarTree2, xzTree2.x, yTree2, xzTree2.y);

		D3DXMATRIX escalaAtaud, lugarAtaud;
		D3DXMatrixScaling(&escalaAtaud, 1.5f, 1.5f, 1.5f);
		D3DXVECTOR2 xzAtaud(35.0f, 40.0f);
		float yAtaud = terreno->getTerrainCollision(xzAtaud) + 0.05f;
		D3DXMatrixTranslation(&lugarAtaud, xzAtaud.x, yAtaud, xzAtaud.y);

		D3DXMATRIX escalaCalaca, lugarCalaca;
		D3DXMatrixScaling(&escalaCalaca, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzCalaca(45.0f, 35.0f);
		float yCalaca = terreno->getTerrainCollision(xzCalaca) + 0.05f;
		D3DXMatrixTranslation(&lugarCalaca, xzCalaca.x, yCalaca, xzCalaca.y);

		D3DXMATRIX escalaCalaca2, lugarCalaca2;
		D3DXMatrixScaling(&escalaCalaca2, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzCalaca2(50.0f, 40.0f);
		float yCalaca2 = terreno->getTerrainCollision(xzCalaca2) + 0.05f;
		D3DXMatrixTranslation(&lugarCalaca2, xzCalaca2.x, yCalaca2, xzCalaca2.y);

		D3DXMATRIX escalaCastillo, lugarCastillo;
		D3DXMatrixScaling(&escalaCastillo, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzCastillo(80.0f, 130.0f);
		float yCastillo = terreno->getTerrainCollision(xzCastillo) + 0.05f;
		D3DXMatrixTranslation(&lugarCastillo, xzCastillo.x, yCastillo, xzCastillo.y);

		D3DXMATRIX escalaAm1, lugarAm1;
		D3DXMatrixScaling(&escalaAm1, 0.6f, 0.6f, 0.6f);
		D3DXVECTOR2 xzAm1(20.0f, 20.0f);
		float yAm1 = terreno->getTerrainCollision(xzAm1) + 0.889f * 0.6f + 0.05f;
		D3DXMatrixTranslation(&lugarAm1, xzAm1.x, yAm1, xzAm1.y);

		D3DXMATRIX escalaAm2, lugarAm2;
		D3DXMatrixScaling(&escalaAm2, 0.6f, 0.6f, 0.6f);
		D3DXVECTOR2 xzAm2(15.0f, 30.0f);
		float yAm2 = terreno->getTerrainCollision(xzAm2) + 0.889f * 0.6f + 0.05f;
		D3DXMatrixTranslation(&lugarAm2, xzAm2.x, yAm2, xzAm2.y);

		//Mis Modelos
		arbol->UpdateScene(escalaArbol * lugarArbol);
		barril->UpdateScene(escalaBarril * lugarBarril);
		gato->UpdateScene(escalaGato * lugarGato);
		tumba->UpdateScene(escalaTumba * lugarTumba);
		tree2->UpdateScene(escalaTree2 * lugarTree2);
		ataud->UpdateScene(escalaAtaud * lugarAtaud);
		calaca->UpdateScene(escalaCalaca * lugarCalaca);
		calaca2->UpdateScene(escalaCalaca2 * lugarCalaca2);
		castillo->UpdateScene(escalaCastillo * lugarCastillo);
		arbolmuerto1->UpdateScene(escalaAm1 * lugarAm1);
		arbolmuerto2->UpdateScene(escalaAm2 * lugarAm2);
		

		D3DXMATRIX skyRotMat, terrMat;
		D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
		skydomo->UpdateScene(skyRotMat);


		//D3DXMatrixTranslation(&terrMat, -50, 0, -50);
		D3DXMatrixIdentity(&terrMat);
		terreno->UpdateScene(terrMat);
		
		

		camara->Position.y = terreno->getTerrainCollision(camara->getXZ()) +3;
	}

	void DrawScene()
	{
		//Clear our backbuffer to the updated color
		D3DXCOLOR bgColor(0.0f, 0.0f, 0.25f, 1.0f);
		d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
		//Refresh the Depth/Stencil view
		d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		//Mis modelos
		barril->DrawScene(camara);
		arbol->DrawScene(camara);
		gato->DrawScene(camara);
		tumba->DrawScene(camara);
		tree2->DrawScene(camara);
		ataud->DrawScene(camara);
		calaca->DrawScene(camara);
		calaca2->DrawScene(camara);
		castillo->DrawScene(camara);
		arbolmuerto1->DrawScene(camara);
		arbolmuerto2->DrawScene(camara);

		terreno->DrawScene(camara);
		skydomo->DrawScene(skyCamara);

		Bitmap->DrawBitmap(camara);


		//Present the backbuffer to the screen
		SwapChain->Present(0, 0);
	}

};

#endif