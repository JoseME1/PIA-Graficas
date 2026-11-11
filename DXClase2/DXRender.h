#ifndef _DXRender_
#define _DXRender_

#include "Constants.h"
#include <chrono>
#include "Camara.h"
#include "Light.h"
#include "MeshPrimitives.h"
#include "AudioManager.h"


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
	ID3D11DepthStencilState* depthDisabledState = nullptr;


	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	D3DXMATRIX View;
	D3DXMATRIX Projection;


	MPrimitives *skydomo;
	MPrimitives *terreno;
	MPrimitives* agua;
	MPrimitives *Bitmap;

	//MIS MODELOS
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
	MPrimitives* debugBox;
	bool showColliders = false;
	D3DXVECTOR3 playerPos;
	bool thirdPerson = false;
	bool vKeyLast = false;

	struct CircleCollider
	{
		float x, z, radius;
	};
	CircleCollider colliders[9];
	float playerRadius = 1.0f;

	struct BoxCollider
	{
		float minX, maxX, minZ, maxZ;
	};
	BoxCollider castleWalls[20];

	Camara *camara;
	Camara *skyCamara;
	AudioManager audio;
	float audioVolume = 0.2f;


	float rotPerFrame;
	float sphereSpeed;
	std::chrono::steady_clock::time_point lastTime;
	float deltaTime = 0.016f;


public:
	DXRender()
	{
		rotPerFrame = 0.0f;
		lastTime = std::chrono::steady_clock::now();
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

		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(dsDesc));
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.StencilEnable = FALSE;
		d3d11Device->CreateDepthStencilState(&dsDesc, &depthDisabledState);


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
		skydomo = new MPrimitives(d3d11Device, d3d11DevCon, 32, 32, 1000.0f, L"Texturas/sky.jpg", L"Texturas/escudoNormal.jpg", ShaderCielo);
		terreno = new MPrimitives(d3d11Device, d3d11DevCon, 100.0f, L"Texturas/rock.jpg", L"Texturas/grass.jpg", "Texturas/terreno.bmp", ShaderMultitextura);
		agua = new MPrimitives(d3d11Device, d3d11DevCon, 32, L"Texturas/agua.jpg", L"", "Texturas/agua_plana.bmp", ShaderAgua);

		debugBox = new MPrimitives(d3d11Device, d3d11DevCon, 1.0f, L"Texturas/rojo.jpg", L"", ShaderDiffuse);

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

	void ReacquireInput()
	{
		if (DIKeyboard)
		{
			DIKeyboard->Unacquire();
			DIKeyboard->Acquire();
		}
		if (DIMouse)
		{
			DIMouse->Unacquire();
			DIMouse->Acquire();
		}
		ZeroMemory(&mouseLastState, sizeof(mouseLastState));
	}

	void DetectInput()
	{
		DIMOUSESTATE mouseCurrState;
		ZeroMemory(&mouseCurrState, sizeof(mouseCurrState));

		BYTE keyboardState[256];
		ZeroMemory(keyboardState, sizeof(keyboardState));

		DIKeyboard->Acquire();

		if (FAILED(DIMouse->Acquire()))
		{
			DIMouse->Unacquire();
			DIMouse->Acquire();
		}

		if (FAILED(DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState)))
		{
			ZeroMemory(&mouseCurrState, sizeof(mouseCurrState));
			mouseLastState = mouseCurrState;
		}

		if (FAILED(DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState)))
		{
			ZeroMemory(keyboardState, sizeof(keyboardState));
		}

		float speed = 20.0f * deltaTime;

		float moveLeftRight = 0;
		float moveBackForward = 0;

		float camYaw = 0;
		float camPitch = 0;

		
		if (keyboardState[DIK_LSHIFT] || keyboardState[DIK_RSHIFT]){
			speed *= 2.0f;
		}

		showColliders = (keyboardState[DIK_C] != 0);

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

			float padSpeed = (pad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? speed * 1.5f : speed;
			moveLeftRight += normLX * padSpeed;
			moveBackForward += normLY * padSpeed;

			
			if (pad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || pad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				camYaw += (float)pad.sThumbRX / 32767.0f * 0.008f;
			if (pad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || pad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				camPitch -= (float)pad.sThumbRY / 32767.0f * 0.008f;


			if (pad.wButtons & XINPUT_GAMEPAD_START)
				PostQuitMessage(0);
		}

		bool vDown = (keyboardState[DIK_V] != 0);
		if (vDown && !vKeyLast) thirdPerson = !thirdPerson;
		vKeyLast = vDown;

		if (thirdPerson)
		{
			camara->UpdateCamera(0, 0, camYaw, camPitch);
			playerPos += moveBackForward * camara->Forward;
			playerPos += moveLeftRight * camara->Right;
		}
		else
		{
			camara->UpdateCamera(moveBackForward, moveLeftRight, camYaw, camPitch);
			playerPos.x = camara->Position.x;
			playerPos.z = camara->Position.z;
		}
		skyCamara->setSkyCamara(camara);
	}

	void ReleaseObjects()
	{
		//Release the COM Objects we created
		audio.Shutdown();
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
		D3DXVECTOR3 dir(0.5f, 1.0f, 0.3f);
		D3DXVec3Normalize(&dir, &dir);

		light.direction = dir;
		light.ambient = D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
		light.diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		light.specularPower = 32.0f;
		light.specular = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

		//Set the Viewport
		d3d11DevCon->RSSetViewports(1, &viewport);

		//Create the camera
		D3DXVECTOR3 Position = D3DXVECTOR3(20.0f, 0.0f, 20.0f);
		D3DXVECTOR3 Target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		camara = new Camara(Position,Target,Up, Width, Height);
		skyCamara = new Camara(Position,Target,Up, Width, Height);
		playerPos = D3DXVECTOR3(150.0f, 0.0f, 200.0f);
		thirdPerson = false;
		vKeyLast = false;
		//esta es la gui
		Bitmap = new MPrimitives(d3d11Device, d3d11DevCon, 1, 1, L"Texturas/Health.png", camara);
		// Colisiones: circulos para los objetos chicos, paredes para el castillo
		colliders[0] = { 110.0f, 130.0f, 2.0f };  // arbol
		colliders[1] = { 160.0f, 155.0f, 1.0f };  // barril
		colliders[2] = { 150.0f, 140.0f, 2.0f };  // gato
		colliders[3] = { 145.0f, 150.0f, 2.0f };  // tumba
		colliders[4] = { 190.0f, 130.0f, 2.0f };  // tree2
		colliders[5] = { 150.0f, 205.0f, 1.5f };  // ataud
		colliders[6] = { 165.0f, 170.0f, 1.5f };  // calaca2
		colliders[7] = { 45.0f, 45.0f, 1.0f };    // arbolmuerto1
		colliders[8] = { 70.0f, 30.0f, 1.0f };    // arbolmuerto2
		// Paredes del castillo (modelo 63 x 40 en el mundo, X 118-182, Z 230-270)
		// Puerta en la pared sur, hueco de X 147 a 153
		// Paredes exteriores
		castleWalls[0] = { 118.0f, 182.0f, 268.0f, 270.0f }; // norte
		castleWalls[1] = { 118.0f, 147.0f, 230.0f, 232.0f }; // sur izquierda
		castleWalls[2] = { 153.0f, 182.0f, 230.0f, 232.0f }; // sur derecha
		castleWalls[3] = { 118.0f, 120.0f, 230.0f, 270.0f }; // oeste
		castleWalls[4] = { 180.0f, 182.0f, 230.0f, 270.0f }; // este
		// Paredes internas, medidas del modelo (local * 2 + centro 150, 250)
		// Verticales X = -10.5 y +10.5, de lado a lado
		castleWalls[5] = { 128.0f, 130.0f, 230.8f, 269.8f }; // X -10.5
		castleWalls[6] = { 170.0f, 172.0f, 231.0f, 269.6f }; // X +10.5
		// Verticales X = -3.5 y +3.5, con puerta al centro (Z 245.8 - 250.4)
		castleWalls[7] = { 142.0f, 144.0f, 231.2f, 245.8f }; // X -3.5 sur
		castleWalls[8] = { 142.0f, 144.0f, 250.4f, 269.8f }; // X -3.5 norte
		castleWalls[9] = { 156.0f, 158.0f, 231.2f, 245.8f }; // X +3.5 sur
		castleWalls[10] = { 156.0f, 158.0f, 250.4f, 269.6f }; // X +3.5 norte
		// Horizontales Z = -4.5 y +5, con puerta al centro (X 147.6 - 152.2)
		castleWalls[11] = { 119.0f, 147.6f, 240.0f, 242.0f }; // Z -4.5 oeste
		castleWalls[12] = { 152.2f, 181.0f, 240.0f, 242.0f }; // Z -4.5 este
		castleWalls[13] = { 119.0f, 147.6f, 259.0f, 261.0f }; // Z +5 oeste
		castleWalls[14] = { 152.2f, 181.0f, 259.0f, 261.0f }; // Z +5 este
		// Horizontales Z = -7 y +7, con paso ancho al centro (X 143.2 - 156.6)
		castleWalls[15] = { 128.4f, 143.2f, 235.0f, 237.0f }; // Z -7 oeste
		castleWalls[16] = { 156.6f, 171.4f, 235.0f, 237.0f }; // Z -7 este
		castleWalls[17] = { 128.4f, 143.2f, 263.0f, 265.0f }; // Z +7 oeste
		castleWalls[18] = { 156.6f, 171.4f, 263.0f, 265.0f }; // Z +7 este
		// Escalera central (local X -1 a +1, Z -1.5 a 2.5, sube hacia el norte)
		castleWalls[19] = { 148.0f, 152.0f, 247.0f, 255.0f }; // escalera
		audio.Init(L"Sonidos/ambient.wav", audioVolume);
		return true;
	}

	void UpdateScene()
	{
		auto now = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;
		if (deltaTime > 0.05f) deltaTime = 0.05f;
		if (deltaTime < 0.0f) deltaTime = 0.0f;


		rotPerFrame += deltaTime * 6.0f;
		g_Time += deltaTime * 0.3f;

		float dayAngle = g_Time * 0.1f;
		float sunHeight = sin(dayAngle);

		// 0 en el horizonte, 1 cuando el sol ya subio un poco,
		// y se queda en 1 todo el mediodia, no solo en el cenit
		float dayFactor = sunHeight / 0.3f;
		if (dayFactor < 0.0f) dayFactor = 0.0f;
		if (dayFactor > 1.0f) dayFactor = 1.0f;

		D3DXVECTOR3 sunDir(cos(dayAngle), sin(dayAngle), cos(dayAngle) * 0.6f);
		D3DXVec3Normalize(&sunDir, &sunDir);
		light.direction = sunDir;

		if (sunHeight <= 0.0f)
		{
			// Noche: sin sol, sin difusa y sin especular.
			// Solo queda un poco de ambiente, como luz de luna.
			light.ambient = D3DXVECTOR4(0.05f, 0.05f, 0.08f, 1.0f);
			light.diffuse = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
			light.specular = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			light.ambient = D3DXVECTOR4(0.10f + 0.30f * dayFactor,
				0.10f + 0.30f * dayFactor,
				0.10f + 0.30f * dayFactor, 1.0f);
			light.diffuse = D3DXVECTOR4(0.15f + 0.85f * dayFactor,
				0.15f + 0.85f * dayFactor,
				0.15f + 0.85f * dayFactor, 1.0f);
			light.specular = D3DXVECTOR4(dayFactor, dayFactor, dayFactor, 1.0f);
		}




		//Mis Modelos
		D3DXMATRIX escalaArbol, lugarArbol;
		D3DXMatrixScaling(&escalaArbol, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzArbol(110.0f, 130.0f);
		float yArbol = terreno->getTerrainCollision(xzArbol) + 0.05f;
		D3DXMatrixTranslation(&lugarArbol, xzArbol.x, yArbol, xzArbol.y);

		D3DXMATRIX escalaBarril, lugarBarril;
		D3DXMatrixScaling(&escalaBarril, 1.0f, 1.0f, 1.0f);
		D3DXVECTOR2 xzBarril(160.0f, 155.0f);
		float yBarril = terreno->getTerrainCollision(xzBarril) - 0.023f * 1.0f + 0.05f;
		D3DXMatrixTranslation(&lugarBarril, xzBarril.x, yBarril, xzBarril.y);

		D3DXMATRIX escalaGato, lugarGato;
		D3DXMatrixScaling(&escalaGato, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzGato(150.0f, 140.0f);
		float yGato = terreno->getTerrainCollision(xzGato) + 0.05f;
		D3DXMatrixTranslation(&lugarGato, xzGato.x, yGato, xzGato.y);

		D3DXMATRIX escalaTumba, lugarTumba;
		D3DXMatrixScaling(&escalaTumba, 2.5f, 2.5f, 2.5f);
		D3DXVECTOR2 xzTumba(145.0f, 150.0f);
		float yTumba = terreno->getTerrainCollision(xzTumba) + 0.05f;
		D3DXMatrixTranslation(&lugarTumba, xzTumba.x, yTumba, xzTumba.y);

		D3DXMATRIX escalaTree2, lugarTree2;
		D3DXMatrixScaling(&escalaTree2, 1.5f, 1.5f, 1.5f);
		D3DXVECTOR2 xzTree2(190.0f, 130.0f);
		float yTree2 = terreno->getTerrainCollision(xzTree2) + 0.05f;
		D3DXMatrixTranslation(&lugarTree2, xzTree2.x, yTree2, xzTree2.y);

		D3DXMATRIX escalaAtaud, lugarAtaud;
		D3DXMatrixScaling(&escalaAtaud, 0.75f, 0.75f, 0.75f);
		D3DXVECTOR2 xzAtaud(150.0f, 205.0f);
		float yAtaud = terreno->getTerrainCollision(xzAtaud) + 1.0f * 0.75f + 0.05f;
		D3DXMatrixTranslation(&lugarAtaud, xzAtaud.x, yAtaud, xzAtaud.y);

		D3DXMATRIX escalaCalaca, lugarCalaca, rotCalaca;
		D3DXMatrixScaling(&escalaCalaca, 2.0f, 2.0f, 2.0f);
		D3DXMatrixRotationY(&rotCalaca, camara->Yaw);
		D3DXMatrixTranslation(&lugarCalaca, playerPos.x, playerPos.y + 0.05f, playerPos.z);

		D3DXMATRIX escalaCalaca2, lugarCalaca2;
		D3DXMatrixScaling(&escalaCalaca2, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzCalaca2(165.0f, 170.0f);
		float yCalaca2 = terreno->getTerrainCollision(xzCalaca2) + 0.05f;
		D3DXMatrixTranslation(&lugarCalaca2, xzCalaca2.x, yCalaca2, xzCalaca2.y);

		D3DXMATRIX escalaCastillo, lugarCastillo;
		D3DXMatrixScaling(&escalaCastillo, 2.0f, 2.0f, 2.0f);
		D3DXVECTOR2 xzCastillo(150.0f, 250.0f);
		float yCastillo = terreno->getTerrainCollision(xzCastillo) + 0.05f;
		D3DXMatrixTranslation(&lugarCastillo, xzCastillo.x, yCastillo, xzCastillo.y);

		D3DXMATRIX escalaAm1, lugarAm1;
		D3DXMatrixScaling(&escalaAm1, 0.6f, 0.6f, 0.6f);
		D3DXVECTOR2 xzAm1(45.0f, 45.0f);
		float yAm1 = terreno->getTerrainCollision(xzAm1) + 0.889f * 0.6f + 0.05f;
		D3DXMatrixTranslation(&lugarAm1, xzAm1.x, yAm1, xzAm1.y);

		D3DXMATRIX escalaAm2, lugarAm2;
		D3DXMatrixScaling(&escalaAm2, 0.6f, 0.6f, 0.6f);
		D3DXVECTOR2 xzAm2(70.0f, 30.0f);
		float yAm2 = terreno->getTerrainCollision(xzAm2) + 0.889f * 0.6f + 0.05f;
		D3DXMatrixTranslation(&lugarAm2, xzAm2.x, yAm2, xzAm2.y);

		D3DXMATRIX escalaAgua, lugarAgua;
		D3DXMatrixScaling(&escalaAgua, 2.0f, 1.0f, 2.0f);
		D3DXVECTOR2 xzAgua(20.0f, 20.0f);
		float yAgua = terreno->getTerrainCollision(xzAgua)-10.5f;
		D3DXMatrixTranslation(&lugarAgua, xzAgua.x, yAgua, xzAgua.y);
		


		//Mis Modelos
		arbol->UpdateScene(escalaArbol * lugarArbol);
		barril->UpdateScene(escalaBarril * lugarBarril);
		gato->UpdateScene(escalaGato * lugarGato);
		tumba->UpdateScene(escalaTumba * lugarTumba);
		tree2->UpdateScene(escalaTree2 * lugarTree2);
		ataud->UpdateScene(escalaAtaud * lugarAtaud);
		calaca->UpdateScene(escalaCalaca * rotCalaca * lugarCalaca);
		calaca2->UpdateScene(escalaCalaca2 * lugarCalaca2);
		castillo->UpdateScene(escalaCastillo * lugarCastillo);
		arbolmuerto1->UpdateScene(escalaAm1 * lugarAm1);
		arbolmuerto2->UpdateScene(escalaAm2 * lugarAm2);
		agua->UpdateScene(escalaAgua * lugarAgua);
		

		D3DXMATRIX skyRotMat, terrMat;
		D3DXMatrixRotationX(&skyRotMat, D3DXToRadian(-90));
		skydomo->UpdateScene(skyRotMat);


		//D3DXMatrixTranslation(&terrMat, -50, 0, -50);
		D3DXMatrixIdentity(&terrMat);
		terreno->UpdateScene(terrMat);
		
		
		if (playerPos.x < 25.0f) playerPos.x = 25.0f;
		if (playerPos.x > 280.0f) playerPos.x = 280.0f;
		if (playerPos.z < 25.0f) playerPos.z = 25.0f;
		if (playerPos.z > 280.0f) playerPos.z = 280.0f;

		// Colisiones con objetos (circulos en XZ)
		for (int i = 0; i < 9; i++)
		{
			float dx = playerPos.x - colliders[i].x;
			float dz = playerPos.z - colliders[i].z;
			float dist = sqrtf(dx * dx + dz * dz);
			float minDist = colliders[i].radius + playerRadius;
			if (dist < minDist && dist > 0.001f)
			{
				playerPos.x = colliders[i].x + dx / dist * minDist;
				playerPos.z = colliders[i].z + dz / dist * minDist;
			}
		}

		// Colisiones con las paredes del castillo (cajas en XZ, con puerta al sur)
		for (int i = 0; i < 20; i++)
		{
			float minX = castleWalls[i].minX - playerRadius;
			float maxX = castleWalls[i].maxX + playerRadius;
			float minZ = castleWalls[i].minZ - playerRadius;
			float maxZ = castleWalls[i].maxZ + playerRadius;
			float px = playerPos.x;
			float pz = playerPos.z;
			if (px > minX && px < maxX && pz > minZ && pz < maxZ)
			{
				float pushLeft = px - minX;
				float pushRight = maxX - px;
				float pushDown = pz - minZ;
				float pushUp = maxZ - pz;
				float minPush = pushLeft;
				int side = 0;
				if (pushRight < minPush) { minPush = pushRight; side = 1; }
				if (pushDown < minPush) { minPush = pushDown; side = 2; }
				if (pushUp < minPush) { minPush = pushUp; side = 3; }
				if (side == 0) playerPos.x = minX;
				if (side == 1) playerPos.x = maxX;
				if (side == 2) playerPos.z = minZ;
				if (side == 3) playerPos.z = maxZ;
			}
		}

		playerPos.y = terreno->getTerrainCollision(D3DXVECTOR2(playerPos.x, playerPos.z));
		if (thirdPerson)
		{
			camara->Position = playerPos - camara->Forward * 8.0f + D3DXVECTOR3(0.0f, 4.0f, 0.0f);
		}
		else
		{
			camara->Position.x = playerPos.x;
			camara->Position.z = playerPos.z;
			camara->Position.y = playerPos.y + 3.0f;
		}
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
		if (thirdPerson) calaca->DrawScene(camara);
		calaca2->DrawScene(camara);
		castillo->DrawScene(camara);
		arbolmuerto1->DrawScene(camara);
		arbolmuerto2->DrawScene(camara);
		agua->DrawScene(camara);


		terreno->DrawScene(camara);
		skydomo->DrawScene(skyCamara);

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		Bitmap->DrawBitmap(camara);
		d3d11DevCon->OMSetDepthStencilState(nullptr, 0);

		// Debug de colisiones: mantener C presionada para verlas en rojo
		if (showColliders)
		{
			for (int i = 0; i < 20; i++)
			{
				float cx = (castleWalls[i].minX + castleWalls[i].maxX) * 0.5f;
				float cz = (castleWalls[i].minZ + castleWalls[i].maxZ) * 0.5f;
				float w = castleWalls[i].maxX - castleWalls[i].minX;
				float d = castleWalls[i].maxZ - castleWalls[i].minZ;
				float y = terreno->getTerrainCollision(D3DXVECTOR2(cx, cz)) + 1.0f;
				D3DXMATRIX s, t;
				D3DXMatrixScaling(&s, w, 2.0f, d);
				D3DXMatrixTranslation(&t, cx, y, cz);
				debugBox->UpdateScene(s * t);
				debugBox->DrawScene(camara);
			}
			for (int i = 0; i < 9; i++)
			{
				float cx = colliders[i].x;
				float cz = colliders[i].z;
				float r = colliders[i].radius + playerRadius;
				float y = terreno->getTerrainCollision(D3DXVECTOR2(cx, cz)) + 1.0f;
				D3DXMATRIX s, t;
				D3DXMatrixScaling(&s, r * 2.0f, 2.0f, r * 2.0f);
				D3DXMatrixTranslation(&t, cx, y, cz);
				debugBox->UpdateScene(s * t);
				debugBox->DrawScene(camara);
			}
		}

		//Present the backbuffer to the screen
		SwapChain->Present(1, 0);
	}

};

#endif