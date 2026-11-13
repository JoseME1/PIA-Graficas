#ifndef _DXRender_
#define _DXRender_

#include "Constants.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include "Camara.h"
#include "Light.h"
#include "MeshPrimitives.h"
#include "AudioManager.h"
#include "Texto.h"


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
	MPrimitives* pasto[150];
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

	// HUD de texto en pantalla
	MPrimitives* hudQuad = nullptr;
	ID3D11BlendState* alphaBlendState = nullptr;
	std::wstring hudTexto;
	std::wstring hudTextoDibujado;
	std::vector<unsigned char> hudPixeles;
	int hudAncho = 1000;
	int hudAlto = 120;
	int hudFuente = 30;
	int hudAnchoDibujado = 0;
	int hudAltoDibujado = 0;
	int hudScreenW = 1920;
	int hudScreenH = 1080;

	// Circuito de la carrera (punto 0 = meta, el resto = checkpoints en orden)
	enum EstadoCarrera { CarreraSinRuta, CarreraEsperando, CarreraCuenta, CarreraCorriendo, CarreraTerminada };
	std::vector<D3DXVECTOR2> puntosRuta;
	EstadoCarrera estadoCarrera = CarreraSinRuta;
	int siguientePunto = 1;
	float tiempoMensaje = 0.0f;
	float radioMeta = 9.0f;
	float radioPunto = 9.0f;

	// Carrera automatica (la calaca corre sola de checkpoint a checkpoint)
	bool autoCorriendo = false;
	float cuentaRegresiva = 0.0f;
	float velocidadCarrera = 12.0f;

	// Estadisticas de la calaca:
	//   velocidad   = velocidad maxima a la que puede llegar
	//   aceleracion = segundos que tarda en llegar a esa velocidad maxima
	//   resistencia = segundos que aguanta corriendo a la velocidad maxima
	float aceleracionCarrera = 2.0f;
	float resistenciaCarrera = 15.0f;
	float velocidadActual = 0.0f;		// velocidad en este instante (unidades/s)
	float resistenciaActual = 1.0f;		// 1 = llena, 0 = agotada

	// Valores base leidos de ruta.txt. Las mejoras de entrenamiento se aplican
	// sobre estos y no se escriben de vuelta al archivo.
	float velocidadBase = 12.0f;
	float aceleracionBase = 2.0f;
	float resistenciaBase = 15.0f;
	int mejorasVelocidad = 0;
	int mejorasAceleracion = 0;
	int mejorasResistencia = 0;

	// Modo de captura de ruta
	bool modoRuta = false;
	bool f2Last = false, pLast = false, backLast = false, eLast = false;

	// Vistas de camara para la carrera: la vista 1 se usa mientras la calaca va
	// hacia el checkpoint 1, la 2 hacia el checkpoint 2, y asi. Si hay mas
	// checkpoints que vistas, el resto se ve en tercera persona.
	struct VistaCamara
	{
		D3DXVECTOR3 pos;
		float yaw;
		float pitch;
	};
	std::vector<VistaCamara> vistasCamara;
	bool modoCamara = false;
	bool f3Last = false;
	bool vistasSeguir = true;			// la camara mira siempre a la calaca
	float velocidadVuelo = 20.0f;		// velocidad de la camara libre (unidades/s)
	float yawCalaca = 0.0f;				// hacia donde corre la calaca en la carrera automatica
	float suavizadoCamara = 5.0f;		// que tan rapido gira la camara hacia la calaca
	int vistaActual = -1;				// vista que se esta usando (para no interpolar al cambiar de camara)
	bool vistaCarreraActiva = false;	// true si la camara la manda una vista de la carrera

	// Marcadores del circuito dibujados en el mundo
	MPrimitives* marcadorMeta = nullptr;	// anillo dorado en la meta
	MPrimitives* marcadorSig = nullptr;		// anillo verde en el siguiente punto
	MPrimitives* marcadorPunto = nullptr;	// anillo azul en el resto
	float alturaMarcador = 0.35f;			// altura de los anillos sobre el terreno

	// Flecha del HUD que apunta al siguiente punto
	MPrimitives* hudFlecha = nullptr;
	std::vector<unsigned char> flechaPixeles;
	float gradosFlecha = 999.0f;

	// Panel de estadisticas (esquina inferior derecha)
	MPrimitives* hudPanel = nullptr;
	std::vector<unsigned char> panelPixeles;
	std::wstring panelClave;
	int panelAncho = 480;
	int panelAlto = 160;

	// Aros de resistencia (quick time event): se colocan con F2 + R y se
	// guardan en ruta.txt como "aro = x, z" despues de los checkpoints.
	std::vector<D3DXVECTOR2> arosResistencia;
	std::vector<bool> arosUsados;			// un aro solo se puede usar una vez por carrera
	float radioAro = 6.0f;					// distancia horizontal para activar el aro
	MPrimitives* marcadorAro = nullptr;		// aro disponible (magenta)
	MPrimitives* marcadorAroUsado = nullptr;	// aro ya usado (gris)

	// Calacas rivales de la carrera. De base tienen las mismas estadisticas que
	// el jugador (las de ruta.txt) y cada hora de entrenamiento mejoran un stat
	// al azar. Cada una mejora un porcentaje distinto y tiene su propia
	// probabilidad de acertar el quick time event de los aros.
	struct CalacaIA
	{
		const wchar_t* nombre;
		float porcentajeMejora;
		float probabilidadAro;
		int mejoras[3];				// 0 = velocidad, 1 = aceleracion, 2 = resistencia

		D3DXVECTOR3 pos;
		float velocidad;
		float aceleracion;
		float resistencia;
		float velocidadActual;
		float resistenciaActual;
		int siguientePunto;
		bool terminado;
		float tiempoLlegada;
		float yaw;
		std::vector<bool> arosUsados;
		MPrimitives* modelo;
	};
	CalacaIA calacasIA[4];
	float relojCarrera = 0.0f;
	float tiempoLlegadaJugador = 0.0f;

	// Diamante verde sobre la calaca del jugador (como el de los Sims)
	MPrimitives* diamanteJugador = nullptr;

	// Marcador de posicion de la carrera (esquina inferior izquierda)
	MPrimitives* hudPosicion = nullptr;
	std::vector<unsigned char> posicionPixeles;
	int posicionAncho = 420;
	int posicionAlto = 150;
	int posicionFuente = 96;
	int rangoDibujado = -1;

	// Tabla de stats de las calacas rivales (esquina superior derecha)
	MPrimitives* hudTabla = nullptr;
	std::vector<unsigned char> tablaPixeles;
	std::wstring tablaClave;
	int tablaAncho = 548;
	int tablaAlto = 198;

	// Ulti: circulo de fuego con una calavera (esquina superior izquierda)
	MPrimitives* hudUlti = nullptr;
	std::vector<unsigned char> ultiPixeles;
	int ultiAncho = 200;
	int ultiAlto = 200;
	int ultiCargaDibujada = -1;
	int ultiSemilla = 0;
	float ultiFlicker = 0.0f;

	// Tecla de espacio que avisa cuando la ulti esta lista
	MPrimitives* hudTeclaEspacio = nullptr;
	std::vector<unsigned char> teclaEspacioPixeles;
	int teclaAncho = 280;
	int teclaAlto = 84;

	// Estado de la ulti
	float cargaUlti = 0.0f;		// 0 = vacia, 1 = lista para usar
	float ultiTiempo = 0.0f;	// segundos que le quedan a la ulti activa
	bool spaceUltiLast = false;

	// Quick time event del aro: 4 flechas aleatorias en 2 segundos.
	// 0 = arriba, 1 = derecha, 2 = abajo, 3 = izquierda
	bool qteActivo = false;
	int qteSecuencia[4] = { 0, 0, 0, 0 };
	int qteProgreso = 0;					// flechas acertadas en orden
	float qteTiempo = 0.0f;					// segundos restantes
	bool qteLast[4] = { false, false, false, false };
	MPrimitives* hudQte[4] = { nullptr, nullptr, nullptr, nullptr };
	std::vector<unsigned char> qtePixeles[4];
	int qteSlotDir[4] = { -1, -1, -1, -1 };	// direccion dibujada en cada slot
	bool qteSlotVerde[4] = { false, false, false, false };
	MPrimitives* hudQteBarra = nullptr;		// relleno del contrarreloj
	MPrimitives* hudQteBarraFondo = nullptr;	// pista oscura del contrarreloj
	bool rLast = false, delLast = false;
	float hudY = 0.62f;						// posicion vertical del mensaje (0 = arriba, 1 = abajo)
	float tamFlecha = 110.0f;				// lado de la flecha en pixeles

	// Puntos de entrenamiento (F2 + 1/2/3). Cada uno mejora un 10% su stat.
	// tipo: 0 = velocidad, 1 = aceleracion, 2 = resistencia
	struct PuntoEntrenamiento
	{
		D3DXVECTOR2 pos;
		int tipo;
		bool usadoSlot;		// ya usado en esta hora del dia
	};
	std::vector<PuntoEntrenamiento> puntosEntrenamiento;
	float radioEntrenamiento = 6.0f;
	MPrimitives* marcadorEntVel = nullptr;		// cian, como la barra de velocidad
	MPrimitives* marcadorEntAcel = nullptr;		// naranja, como la de aceleracion
	MPrimitives* marcadorEntRes = nullptr;		// verde, como la de resistencia
	MPrimitives* marcadorEntUsado = nullptr;	// gris, ya usado en esta hora

	// Calendario: 2 dias de entrenamiento y el tercero es la carrera.
	// slot: 0 = mañana, 1 = dia, 2 = tarde, 3 = noche
	int diaActual = 1;
	int entrenamientosHoy = 0;
	int slotActual = 0;
	bool k1Last = false, k2Last = false, k3Last = false, k4Last = false;
	float tiempoMensajeEntrenamiento = 0.0f;
	std::wstring mensajeEntrenamiento;

	// Aviso grande al empezar cada dia (titulo + subtitulo)
	MPrimitives* hudBannerFondo = nullptr;
	MPrimitives* hudBannerTitulo = nullptr;
	MPrimitives* hudBannerSub = nullptr;
	std::vector<unsigned char> bannerPixelesTitulo;
	std::vector<unsigned char> bannerPixelesSub;
	std::wstring bannerTitulo;
	std::wstring bannerSub;
	std::wstring bannerTituloDibujado;
	std::wstring bannerSubDibujado;
	float bannerTiempo = 0.0f;


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
		HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
			D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

		if (FAILED(hr))
		{
			return false;
		}

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
		for (int i = 0; i < 150; i++)
			pasto[i] = new MPrimitives(d3d11Device, d3d11DevCon,1.5f, 3.0f, L"Texturas/pasto.png", ShaderPasto);

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

		float speed = (modoCamara ? velocidadVuelo : 20.0f) * deltaTime;

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

		// Modo de captura de ruta: F2 lo activa, P agrega el punto actual, Retroceso borra el ultimo
		bool f2Down = (keyboardState[DIK_F2] != 0);
		if (f2Down && !f2Last && !modoCamara)
		{
			modoRuta = !modoRuta;
			if (!modoRuta)
				CargarRuta();	// al salir, recarga el circuito para poder correrlo sin reiniciar
		}
		f2Last = f2Down;

		// Modo editor de vistas de camara: F3 lo activa (vuelo libre), P guarda la
		// vista actual como la del siguiente tramo y Retroceso borra la ultima.
		bool f3Down = (keyboardState[DIK_F3] != 0);
		if (f3Down && !f3Last && !modoRuta)
		{
			if (modoCamara)
			{
				modoCamara = false;
				GuardarVistas();
			}
			else if (estadoCarrera != CarreraCuenta && !autoCorriendo)
				modoCamara = true;
		}
		f3Last = f3Down;

		if (modoCamara)
		{
			bool pDown = (keyboardState[DIK_P] != 0);
			if (pDown && !pLast)
			{
				VistaCamara vista;
				vista.pos = camara->Position;
				vista.yaw = camara->Yaw;
				vista.pitch = camara->Pitch;
				vistasCamara.push_back(vista);
				GuardarVistas();
			}
			pLast = pDown;

			bool backDown = (keyboardState[DIK_BACK] != 0);
			if (backDown && !backLast && !vistasCamara.empty())
			{
				vistasCamara.pop_back();
				GuardarVistas();
			}
			backLast = backDown;

			wchar_t buffer[320];
			swprintf_s(buffer,
				L"Modo camara: %d vistas    [P] agregar    [Retroceso] borrar    [Espacio/Ctrl] subir/bajar    [F3] salir\n"
				L"Camara:  X %.1f   Y %.1f   Z %.1f",
				(int)vistasCamara.size(), camara->Position.x, camara->Position.y, camara->Position.z);
			hudTexto = buffer;
		}

		if (modoRuta)
		{
			bool pDown = (keyboardState[DIK_P] != 0);
			if (pDown && !pLast)
			{
				puntosRuta.push_back(D3DXVECTOR2(playerPos.x, playerPos.z));
				GuardarRuta();
			}
			pLast = pDown;

			bool backDown = (keyboardState[DIK_BACK] != 0);
			if (backDown && !backLast && !puntosRuta.empty())
			{
				puntosRuta.pop_back();
				GuardarRuta();
			}
			backLast = backDown;

			// R agrega un aro de resistencia en la posicion actual
			bool rDown = (keyboardState[DIK_R] != 0);
			if (rDown && !rLast)
			{
				arosResistencia.push_back(D3DXVECTOR2(playerPos.x, playerPos.z));
				arosUsados.push_back(false);
				GuardarRuta();
			}
			rLast = rDown;

			// Supr borra el ultimo aro
			bool delDown = (keyboardState[DIK_DELETE] != 0);
			if (delDown && !delLast && !arosResistencia.empty())
			{
				arosResistencia.pop_back();
				if (!arosUsados.empty())
					arosUsados.pop_back();
				GuardarRuta();
			}
			delLast = delDown;

			// 1/2/3 agregan un punto de entrenamiento del tipo correspondiente
			bool k1Down = (keyboardState[DIK_1] != 0);
			if (k1Down && !k1Last)
			{
				PuntoEntrenamiento punto;
				punto.pos = D3DXVECTOR2(playerPos.x, playerPos.z);
				punto.tipo = 0;
				punto.usadoSlot = false;
				puntosEntrenamiento.push_back(punto);
				GuardarRuta();
			}
			k1Last = k1Down;

			bool k2Down = (keyboardState[DIK_2] != 0);
			if (k2Down && !k2Last)
			{
				PuntoEntrenamiento punto;
				punto.pos = D3DXVECTOR2(playerPos.x, playerPos.z);
				punto.tipo = 1;
				punto.usadoSlot = false;
				puntosEntrenamiento.push_back(punto);
				GuardarRuta();
			}
			k2Last = k2Down;

			bool k3Down = (keyboardState[DIK_3] != 0);
			if (k3Down && !k3Last)
			{
				PuntoEntrenamiento punto;
				punto.pos = D3DXVECTOR2(playerPos.x, playerPos.z);
				punto.tipo = 2;
				punto.usadoSlot = false;
				puntosEntrenamiento.push_back(punto);
				GuardarRuta();
			}
			k3Last = k3Down;

			// 4 borra el ultimo punto de entrenamiento
			bool k4Down = (keyboardState[DIK_4] != 0);
			if (k4Down && !k4Last && !puntosEntrenamiento.empty())
			{
				puntosEntrenamiento.pop_back();
				GuardarRuta();
			}
			k4Last = k4Down;

			wchar_t buffer[384];
			swprintf_s(buffer,
				L"Modo ruta: %d puntos    [P] agregar    [Retroceso] borrar    [F2] salir\n"
				L"Aros: %d    [R] agregar    [Supr] borrar    Posicion:  X %.1f   Z %.1f\n"
				L"Entrenamientos: %d    [1] velocidad    [2] aceleraci\u00F3n    [3] resistencia    [4] borrar",
				(int)puntosRuta.size(), (int)arosResistencia.size(), playerPos.x, playerPos.z,
				(int)puntosEntrenamiento.size());
			hudTexto = buffer;
		}

		// Quick time event del aro: hay que pulsar las 4 flechas en orden.
		// Una tecla equivocada cancela el QTE; si se acaba el tiempo, tambien.
		{
			const int teclasQte[4] = { DIK_UP, DIK_RIGHT, DIK_DOWN, DIK_LEFT };

			for (int i = 0; i < 4; i++)
			{
				bool down = (keyboardState[teclasQte[i]] != 0);

				if (qteActivo && down && !qteLast[i])
				{
					if (i == qteSecuencia[qteProgreso])
					{
						qteProgreso++;

						if (qteProgreso >= 4)
						{
							// Acierto completo: +25% de resistencia
							resistenciaActual += 0.25f;
							if (resistenciaActual > 1.0f)
								resistenciaActual = 1.0f;

							// y +20% de carga de la ulti (salvo que ya este activa)
							if (ultiTiempo <= 0.0f)
							{
								cargaUlti += 0.20f;
								if (cargaUlti > 1.0f)
									cargaUlti = 1.0f;
							}

							qteActivo = false;
						}
					}
					else
					{
						qteActivo = false;	// tecla equivocada: no se gana nada
					}
				}

				qteLast[i] = down;
			}
		}

		// Espacio: activa la ulti cuando la carga esta completa
		bool spaceDown = (keyboardState[DIK_SPACE] != 0);
		if (spaceDown && !spaceUltiLast && !modoRuta && !modoCamara &&
			estadoCarrera == CarreraCorriendo && cargaUlti >= 1.0f && ultiTiempo <= 0.0f)
		{
			ultiTiempo = 2.0f;
			velocidadActual *= 1.3f;	// empujon instantaneo del 30%
		}
		spaceUltiLast = spaceDown;

		// E: durante los dias de entrenamiento entrena en el punto cercano;
		// el dia de la carrera la usa para arrancar junto a la meta.
		bool eDown = (keyboardState[DIK_E] != 0);
		if (eDown && !eLast && !modoRuta && !modoCamara)
		{
			if (diaActual < 3)
			{
				IntentarEntrenar();
			}
			else if (estadoCarrera == CarreraEsperando)
			{
				float dx = playerPos.x - puntosRuta[0].x;
				float dz = playerPos.z - puntosRuta[0].y;
				if (dx * dx + dz * dz <= radioMeta * radioMeta)
				{
					estadoCarrera = CarreraCuenta;
					cuentaRegresiva = 4.0f;
					siguientePunto = 1;
					thirdPerson = true;		// la carrera se ve desde atras de la calaca
					yawCalaca = camara->Yaw;
					vistaActual = -1;
					velocidadActual = 0.0f;		// arranca parada y acelera
					resistenciaActual = 1.0f;	// resistencia llena
					qteActivo = false;
					cargaUlti = 0.0f;			// la ulti arranca vacia
					ultiTiempo = 0.0f;
					arosUsados.assign(arosResistencia.size(), false);	// aros disponibles otra vez
					IniciarCalacasIA();		// rivales en la linea de salida
				}
			}
		}
		eLast = eDown;

		bool vDown = (keyboardState[DIK_V] != 0);
		if (vDown && !vKeyLast && !modoCamara) thirdPerson = !thirdPerson;
		vKeyLast = vDown;

		// Durante la cuenta y la carrera automatica el jugador no controla a la calaca
		if (estadoCarrera == CarreraCuenta || autoCorriendo)
		{
			moveLeftRight = 0.0f;
			moveBackForward = 0.0f;
			camYaw = 0.0f;
			camPitch = 0.0f;
		}

		// La camara gira suave hacia el siguiente punto (y la calaca mira hacia donde corre)
		if (autoCorriendo && siguientePunto < (int)puntosRuta.size())
		{
			float dx = puntosRuta[siguientePunto].x - playerPos.x;
			float dz = puntosRuta[siguientePunto].y - playerPos.z;

			if (dx * dx + dz * dz > 0.01f)
			{
				float objetivo = atan2f(dx, dz);
				float diferencia = objetivo - camara->Yaw;

				while (diferencia > D3DX_PI) diferencia -= 2.0f * D3DX_PI;
				while (diferencia < -D3DX_PI) diferencia += 2.0f * D3DX_PI;

				// Giro suave (interpolacion exponencial), no a velocidad constante
				camYaw = diferencia * (1.0f - expf(-3.0f * deltaTime));
			}
		}

		if (modoCamara)
		{
			// Vuelo libre: la camara se mueve sola y la calaca se queda quieta
			camara->UpdateCamera(moveBackForward, moveLeftRight, camYaw, camPitch);

			float vertical = 0.0f;
			if (keyboardState[DIK_SPACE]) vertical += speed;
			if (keyboardState[DIK_LCONTROL] || keyboardState[DIK_RCONTROL]) vertical -= speed;

			if (vertical != 0.0f)
			{
				camara->Position.y += vertical;
				camara->UpdateCamera(0, 0, 0, 0);	// recalcula View con la nueva altura
			}
		}
		else if (vistaCarreraActiva)
		{
			// La camara y la calaca las controla la carrera: aqui no se tocan,
			// si no la rotacion hacia el checkpoint pelea con la vista guardada.
		}
		else if (thirdPerson)
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

		ActualizarCarrera();
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

	// Lee el circuito desde ruta.txt: una linea por punto, "x, z".
	// La primera linea es la meta, las demas son los checkpoints en orden.
	// Tambien acepta lineas de configuracion: "radio_meta = 9", "radio_punto = 9",
	// "hud_y = 0.62", "altura_marcador = 0.2".
	void CargarRuta()
	{
		// Conserva que puntos ya se usaron en esta hora aunque se recargue el circuito
		std::vector<PuntoEntrenamiento> anteriores = puntosEntrenamiento;

		puntosRuta.clear();
		arosResistencia.clear();
		arosUsados.clear();
		puntosEntrenamiento.clear();
		qteActivo = false;

		radioMeta = 9.0f;
		radioPunto = 9.0f;
		radioAro = 6.0f;
		radioEntrenamiento = 6.0f;
		hudY = 0.62f;
		alturaMarcador = 0.35f;
		velocidadBase = 12.0f;
		aceleracionBase = 2.0f;
		resistenciaBase = 15.0f;

		FILE* archivo = NULL;
		if (fopen_s(&archivo, "ruta.txt", "r") != 0 || archivo == NULL)
		{
			estadoCarrera = CarreraSinRuta;
			return;
		}

		char linea[256];
		while (fgets(linea, sizeof(linea), archivo) != NULL)
		{
			float x = 0.0f;
			float z = 0.0f;

			if (strncmp(linea, "radio_meta", 10) == 0)
				sscanf_s(linea, "radio_meta = %f", &radioMeta);
			else if (strncmp(linea, "radio_punto", 11) == 0)
				sscanf_s(linea, "radio_punto = %f", &radioPunto);
			else if (strncmp(linea, "radio_aro", 9) == 0)
				sscanf_s(linea, "radio_aro = %f", &radioAro);
			else if (strncmp(linea, "radio_entrenamiento", 19) == 0)
				sscanf_s(linea, "radio_entrenamiento = %f", &radioEntrenamiento);
			else if (strncmp(linea, "hud_y", 5) == 0)
				sscanf_s(linea, "hud_y = %f", &hudY);
			else if (strncmp(linea, "altura_marcador", 15) == 0)
				sscanf_s(linea, "altura_marcador = %f", &alturaMarcador);
			else if (strncmp(linea, "velocidad", 9) == 0)
				sscanf_s(linea, "velocidad = %f", &velocidadBase);
			else if (strncmp(linea, "aceleracion", 11) == 0)
				sscanf_s(linea, "aceleracion = %f", &aceleracionBase);
			else if (strncmp(linea, "resistencia", 11) == 0)
				sscanf_s(linea, "resistencia = %f", &resistenciaBase);
			else if (strncmp(linea, "entrenamiento", 13) == 0)
			{
				char tipo[32] = "";
				if (sscanf_s(linea, "entrenamiento = %31[^,], %f , %f",
					tipo, (unsigned)sizeof(tipo), &x, &z) == 3)
				{
					PuntoEntrenamiento punto;
					punto.pos = D3DXVECTOR2(x, z);
					punto.usadoSlot = false;
					if (strncmp(tipo, "velocidad", 9) == 0)			punto.tipo = 0;
					else if (strncmp(tipo, "aceleracion", 11) == 0)	punto.tipo = 1;
					else											punto.tipo = 2;
					puntosEntrenamiento.push_back(punto);
				}
			}
			else if (strncmp(linea, "aro", 3) == 0)
			{
				if (sscanf_s(linea, "aro = %f , %f", &x, &z) == 2)
					arosResistencia.push_back(D3DXVECTOR2(x, z));
			}
			else if (sscanf_s(linea, "%f , %f", &x, &z) == 2)
				puntosRuta.push_back(D3DXVECTOR2(x, z));
		}
		fclose(archivo);

		// Recupera el estado "usado en esta hora" de los puntos que ya existian
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			for (size_t j = 0; j < anteriores.size(); j++)
			{
				if (puntosEntrenamiento[i].tipo == anteriores[j].tipo &&
					fabsf(puntosEntrenamiento[i].pos.x - anteriores[j].pos.x) < 0.02f &&
					fabsf(puntosEntrenamiento[i].pos.y - anteriores[j].pos.y) < 0.02f)
				{
					puntosEntrenamiento[i].usadoSlot = anteriores[j].usadoSlot;
					break;
				}
			}
		}

		RecalcularEstadisticas();

		arosUsados.assign(arosResistencia.size(), false);
		siguientePunto = 1;
		autoCorriendo = false;			// si se recarga a mitad de carrera, se corta la carrera automatica
		cuentaRegresiva = 0.0f;
		velocidadActual = 0.0f;
		resistenciaActual = 1.0f;
		estadoCarrera = (puntosRuta.size() >= 2) ? CarreraEsperando : CarreraSinRuta;
	}

	// Lee las vistas de camara desde camaras.txt. Una linea por vista, en orden:
	// "x, y, z, yaw, pitch" (yaw y pitch en grados).
	// Acepta la configuracion "seguir = 1" (la camara apunta a la calaca) o
	// "seguir = 0" (usa el yaw/pitch guardados tal cual).
	void CargarVistas()
	{
		vistasCamara.clear();
		vistaActual = -1;

		vistasSeguir = true;
		velocidadVuelo = 20.0f;
		suavizadoCamara = 5.0f;

		FILE* archivo = NULL;
		if (fopen_s(&archivo, "camaras.txt", "r") != 0 || archivo == NULL)
			return;

		char linea[256];
		while (fgets(linea, sizeof(linea), archivo) != NULL)
		{
			float x = 0.0f, y = 0.0f, z = 0.0f, yaw = 0.0f, pitch = 0.0f;

			if (strncmp(linea, "seguir", 6) == 0)
			{
				int seguir = 1;
				sscanf_s(linea, "seguir = %d", &seguir);
				vistasSeguir = (seguir != 0);
			}
			else if (strncmp(linea, "velocidad_vuelo", 15) == 0)
				sscanf_s(linea, "velocidad_vuelo = %f", &velocidadVuelo);
			else if (strncmp(linea, "suavizado", 9) == 0)
				sscanf_s(linea, "suavizado = %f", &suavizadoCamara);
			else if (sscanf_s(linea, "%f , %f , %f , %f , %f", &x, &y, &z, &yaw, &pitch) == 5)
			{
				VistaCamara vista;
				vista.pos = D3DXVECTOR3(x, y, z);
				vista.yaw = D3DXToRadian(yaw);
				vista.pitch = D3DXToRadian(pitch);
				vistasCamara.push_back(vista);
			}
		}
		fclose(archivo);
	}

	void GuardarVistas()
	{
		FILE* archivo = NULL;
		if (fopen_s(&archivo, "camaras.txt", "w") != 0 || archivo == NULL)
			return;

		fprintf(archivo, "# Vistas de camara de la carrera: la vista 1 se usa en el tramo hacia el\n");
		fprintf(archivo, "# checkpoint 1, la 2 hacia el checkpoint 2, etc. Si hay mas checkpoints que\n");
		fprintf(archivo, "# vistas, los tramos sobrantes se ven en tercera persona.\n");
		fprintf(archivo, "# Formato: x, y, z, yaw, pitch  (yaw y pitch en grados).\n");
		fprintf(archivo, "# Generado dentro del juego con F3 + P. Editable a mano.\n");
		fprintf(archivo, "seguir = %d\n", vistasSeguir ? 1 : 0);
		fprintf(archivo, "velocidad_vuelo = %.2f\n", velocidadVuelo);
		fprintf(archivo, "suavizado = %.2f\n", suavizadoCamara);
		for (size_t i = 0; i < vistasCamara.size(); i++)
		{
			fprintf(archivo, "%.2f, %.2f, %.2f, %.2f, %.2f\n",
				vistasCamara[i].pos.x, vistasCamara[i].pos.y, vistasCamara[i].pos.z,
				D3DXToDegree(vistasCamara[i].yaw), D3DXToDegree(vistasCamara[i].pitch));
		}
		fclose(archivo);
	}

	void GuardarRuta()
	{
		FILE* archivo = NULL;
		if (fopen_s(&archivo, "ruta.txt", "w") != 0 || archivo == NULL)
			return;

		fprintf(archivo, "# Circuito: la primera linea de puntos es la meta, luego los checkpoints en orden.\n");
		fprintf(archivo, "# Generado dentro del juego con F2 + P. Los valores de abajo se pueden editar a mano.\n");
		fprintf(archivo, "radio_meta = %.2f\n", radioMeta);
		fprintf(archivo, "radio_punto = %.2f\n", radioPunto);
		fprintf(archivo, "radio_aro = %.2f\n", radioAro);
		fprintf(archivo, "radio_entrenamiento = %.2f\n", radioEntrenamiento);
		fprintf(archivo, "hud_y = %.2f\n", hudY);
		fprintf(archivo, "altura_marcador = %.2f\n", alturaMarcador);
		fprintf(archivo, "velocidad = %.2f\n", velocidadBase);
		fprintf(archivo, "aceleracion = %.2f\n", aceleracionBase);
		fprintf(archivo, "resistencia = %.2f\n", resistenciaBase);
		for (size_t i = 0; i < puntosRuta.size(); i++)
			fprintf(archivo, "%.2f, %.2f\n", puntosRuta[i].x, puntosRuta[i].y);
		for (size_t i = 0; i < arosResistencia.size(); i++)
			fprintf(archivo, "aro = %.2f, %.2f\n", arosResistencia[i].x, arosResistencia[i].y);
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			const char* tipo = (puntosEntrenamiento[i].tipo == 0) ? "velocidad"
				: (puntosEntrenamiento[i].tipo == 1) ? "aceleracion" : "resistencia";
			fprintf(archivo, "entrenamiento = %s, %.2f, %.2f\n", tipo,
				puntosEntrenamiento[i].pos.x, puntosEntrenamiento[i].pos.y);
		}
		fclose(archivo);
	}

	// Penalizacion por resistencia agotada:
	//   50% -> velocidad y aceleracion -10%
	//   25% -> -20%     0% -> -30%
	static float PenalizacionPorResistencia(float resistencia)
	{
		if (resistencia <= 0.001f) return 0.30f;
		if (resistencia <= 0.25f) return 0.20f;
		if (resistencia <= 0.50f) return 0.10f;
		return 0.0f;
	}

	float PenalizacionResistencia()
	{
		return PenalizacionPorResistencia(resistenciaActual);
	}

	// Segundos que tarda la calaca en llegar a su velocidad maxima (con la
	// penalizacion de resistencia ya aplicada)
	float TiempoAceleracion()
	{
		return aceleracionCarrera * (1.0f + PenalizacionResistencia());
	}

	// Lanza el quick time event de un aro: 4 flechas al azar (se pueden repetir)
	// y 2 segundos para pulsarlas en orden.
	void IniciarQTE()
	{
		for (int i = 0; i < 4; i++)
		{
			qteSecuencia[i] = rand() % 4;
			qteSlotDir[i] = -1;			// fuerza a redibujar las flechas
			qteSlotVerde[i] = false;
		}

		qteProgreso = 0;
		qteTiempo = 2.0f;
		qteActivo = true;
	}

	// Aplica las mejoras de entrenamiento sobre los valores base de ruta.txt.
	// Cada entrenamiento suma +10% del valor base (aditivo). En aceleracion,
	// mejorar significa tardar menos, asi que el tiempo baja un 10%.
	void RecalcularEstadisticas()
	{
		velocidadCarrera = velocidadBase * (1.0f + 0.10f * mejorasVelocidad);

		float factorAceleracion = 1.0f - 0.10f * mejorasAceleracion;
		if (factorAceleracion < 0.2f)
			factorAceleracion = 0.2f;
		aceleracionCarrera = aceleracionBase * factorAceleracion;

		resistenciaCarrera = resistenciaBase * (1.0f + 0.10f * mejorasResistencia);
	}

	const wchar_t* NombreSlot(int slot)
	{
		switch (slot)
		{
		case 0: return L"Ma\u00F1ana";
		case 1: return L"D\u00EDa";
		case 2: return L"Tarde";
		default: return L"Noche";
		}
	}

	// Luz fija de cada momento del dia. El angulo tambien lo usa el shader del
	// cielo (g_DayAngle) para que el fondo combine con la iluminacion.
	void AplicarLuzDelDia()
	{
		int slot = (diaActual >= 3) ? 1 : slotActual;	// dia de carrera = pleno dia

		switch (slot)
		{
		case 0:		// mañana: sol bajo, tono calido
			g_DayAngle = 0.55f;
			light.ambient = D3DXVECTOR4(0.32f, 0.31f, 0.30f, 1.0f);
			light.diffuse = D3DXVECTOR4(0.95f, 0.88f, 0.78f, 1.0f);
			light.specular = D3DXVECTOR4(0.70f, 0.65f, 0.60f, 1.0f);
			break;
		case 1:		// dia: sol alto y blanco
			g_DayAngle = 1.40f;
			light.ambient = D3DXVECTOR4(0.40f, 0.40f, 0.40f, 1.0f);
			light.diffuse = D3DXVECTOR4(1.00f, 1.00f, 1.00f, 1.0f);
			light.specular = D3DXVECTOR4(1.00f, 1.00f, 1.00f, 1.0f);
			break;
		case 2:		// tarde: sol bajo, naranja
			g_DayAngle = 2.90f;
			light.ambient = D3DXVECTOR4(0.30f, 0.26f, 0.23f, 1.0f);
			light.diffuse = D3DXVECTOR4(1.00f, 0.72f, 0.45f, 1.0f);
			light.specular = D3DXVECTOR4(0.90f, 0.65f, 0.40f, 1.0f);
			break;
		default:	// noche: luz de luna azulada
			g_DayAngle = 3.80f;
			light.ambient = D3DXVECTOR4(0.16f, 0.17f, 0.24f, 1.0f);
			light.diffuse = D3DXVECTOR4(0.12f, 0.13f, 0.22f, 1.0f);
			light.specular = D3DXVECTOR4(0.05f, 0.05f, 0.08f, 1.0f);
			break;
		}

		D3DXVECTOR3 sol(cosf(g_DayAngle), sinf(g_DayAngle), cosf(g_DayAngle) * 0.6f);
		D3DXVec3Normalize(&sol, &sol);
		light.direction = sol;
	}

	void MostrarBannerDia()
	{
		if (diaActual == 1)
		{
			bannerTitulo = L"D\u00EDa 1, Entrenamiento";
			bannerSub = L"Faltan 2 d\u00EDas para la carrera";
		}
		else if (diaActual == 2)
		{
			bannerTitulo = L"D\u00EDa 2, Entrenamiento";
			bannerSub = L"Falta 1 d\u00EDa para la carrera";
		}
		else
		{
			bannerTitulo = L"D\u00EDa de la Carrera";
			bannerSub = L"Ve a la meta y pulsa E";
		}

		bannerTiempo = 5.0f;
	}

	// Termina el dia y arranca el siguiente (o el dia de la carrera)
	void AvanzarDia()
	{
		diaActual++;
		entrenamientosHoy = 0;
		slotActual = 0;
		tiempoMensajeEntrenamiento = 0.0f;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
			puntosEntrenamiento[i].usadoSlot = false;

		AplicarLuzDelDia();
		MostrarBannerDia();
	}

	// Completa una sesion de entrenamiento del tipo indicado
	void Entrenar(int tipo)
	{
		if (tipo == 0)			mejorasVelocidad++;
		else if (tipo == 1)		mejorasAceleracion++;
		else					mejorasResistencia++;

		RecalcularEstadisticas();

		entrenamientosHoy++;
		slotActual = entrenamientosHoy;

		MejorarCalacasIA();		// las rivales tambien entrenan una vez por hora

		wchar_t buffer[192];
		if (tipo == 0)
			swprintf_s(buffer, L"Entrenamiento de velocidad completado!    Velocidad: %.1f", velocidadCarrera);
		else if (tipo == 1)
			swprintf_s(buffer, L"Entrenamiento de aceleraci\u00F3n completado!    Aceleraci\u00F3n: %.2f s", aceleracionCarrera);
		else
			swprintf_s(buffer, L"Entrenamiento de resistencia completado!    Resistencia: %.1f", resistenciaCarrera);

		mensajeEntrenamiento = buffer;
		tiempoMensajeEntrenamiento = 3.0f;

		if (entrenamientosHoy >= 4)
			AvanzarDia();
		else
		{
			// Nueva hora del dia: cada punto vuelve a estar disponible, asi que
			// se puede usar el mismo punto una vez por franja (4 veces al dia).
			for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
				puntosEntrenamiento[i].usadoSlot = false;

			AplicarLuzDelDia();
		}
	}

	// E junto a un punto de entrenamiento disponible
	void IntentarEntrenar()
	{
		if (diaActual >= 3 || entrenamientosHoy >= 4)
			return;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			float dx = puntosEntrenamiento[i].pos.x - playerPos.x;
			float dz = puntosEntrenamiento[i].pos.y - playerPos.z;

			if (dx * dx + dz * dz > radioEntrenamiento * radioEntrenamiento)
				continue;

			if (puntosEntrenamiento[i].usadoSlot)
				continue;

			puntosEntrenamiento[i].usadoSlot = true;
			Entrenar(puntosEntrenamiento[i].tipo);
			return;
		}
	}

	// HUD de los dias de entrenamiento: aviso cerca del punto y estado del dia
	void ActualizarEntrenamiento()
	{
		if (tiempoMensajeEntrenamiento > 0.0f)
		{
			tiempoMensajeEntrenamiento -= deltaTime;
			hudTexto = mensajeEntrenamiento;
			return;
		}

		wchar_t buffer[256];

		// Punto de entrenamiento mas cercano dentro del radio
		int cerca = -1;
		float mejorDistancia = radioEntrenamiento * radioEntrenamiento;
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			float dx = puntosEntrenamiento[i].pos.x - playerPos.x;
			float dz = puntosEntrenamiento[i].pos.y - playerPos.z;
			float distancia = dx * dx + dz * dz;

			if (distancia <= mejorDistancia)
			{
				mejorDistancia = distancia;
				cerca = (int)i;
			}
		}

		if (cerca >= 0)
		{
			const wchar_t* nombre = (puntosEntrenamiento[cerca].tipo == 0) ? L"Velocidad"
				: (puntosEntrenamiento[cerca].tipo == 1) ? L"Aceleraci\u00F3n" : L"Resistencia";

			if (puntosEntrenamiento[cerca].usadoSlot)
				swprintf_s(buffer, L"Ya entrenaste %s en esta hora.    Entrenamientos: %d/4",
					nombre, entrenamientosHoy);
			else
				swprintf_s(buffer, L"Presiona E para entrenar: %s    (%d/4 hoy)",
					nombre, entrenamientosHoy);

			hudTexto = buffer;
			return;
		}

		swprintf_s(buffer, L"D\u00EDa %d - %s    Entrenamientos: %d/4",
			diaActual, NombreSlot(slotActual), entrenamientosHoy);
		hudTexto = buffer;
	}

	// Estado inicial: dia 1 por la mañana
	void IniciarDias()
	{
		diaActual = 1;
		entrenamientosHoy = 0;
		slotActual = 0;
		mejorasVelocidad = 0;
		mejorasAceleracion = 0;
		mejorasResistencia = 0;
		RecalcularEstadisticas();

		// Las rivales empiezan sin mejoras, igual que el jugador
		for (int i = 0; i < 4; i++)
			calacasIA[i].mejoras[0] = calacasIA[i].mejoras[1] = calacasIA[i].mejoras[2] = 0;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
			puntosEntrenamiento[i].usadoSlot = false;

		AplicarLuzDelDia();
		MostrarBannerDia();
	}

	// Cada hora de entrenamiento las rivales mejoran un stat al azar
	void MejorarCalacasIA()
	{
		for (int i = 0; i < 4; i++)
			calacasIA[i].mejoras[rand() % 3]++;
	}

	// Calcula velocidad, aceleracion y resistencia efectivas de una rival con
	// las mejoras que lleva acumuladas. Misma formula que usa la carrera.
	void CalcularStatsIA(CalacaIA& ia)
	{
		ia.velocidad = velocidadBase * (1.0f + ia.porcentajeMejora * ia.mejoras[0]);

		float factorAceleracion = 1.0f - ia.porcentajeMejora * ia.mejoras[1];
		if (factorAceleracion < 0.2f) factorAceleracion = 0.2f;
		ia.aceleracion = aceleracionBase * factorAceleracion;

		ia.resistencia = resistenciaBase * (1.0f + ia.porcentajeMejora * ia.mejoras[2]);
	}

	// Prepara a las rivales al empezar la carrera: aplica las mejoras de los
	// dos dias de entrenamiento y las coloca en la linea de salida.
	void IniciarCalacasIA()
	{
		static const float desvio[4][2] = {
			{ 2.5f, 2.5f }, { -2.5f, 2.5f }, { 2.5f, -2.5f }, { -2.5f, -2.5f }
		};

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];

			CalcularStatsIA(ia);

			ia.pos = D3DXVECTOR3(puntosRuta[0].x + desvio[i][0], 0.0f,
				puntosRuta[0].y + desvio[i][1]);
			ia.pos.y = terreno->getTerrainCollision(D3DXVECTOR2(ia.pos.x, ia.pos.z));

			ia.velocidadActual = 0.0f;
			ia.resistenciaActual = 1.0f;
			ia.siguientePunto = 1;
			ia.terminado = false;
			ia.tiempoLlegada = 0.0f;
			ia.yaw = 0.0f;
			ia.arosUsados.assign(arosResistencia.size(), false);
		}

		relojCarrera = 0.0f;
		tiempoLlegadaJugador = 0.0f;
	}

	// Mueve a las 4 rivales igual que la calaca del jugador: aceleran hacia su
	// velocidad maxima, gastan resistencia y pasan los aros con su probabilidad.
	void ActualizarCalacasIA()
	{
		relojCarrera += deltaTime;

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];
			if (ia.terminado)
				continue;

			float dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
			float dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
			float distancia = sqrtf(dx * dx + dz * dz);

			if (distancia > 0.001f)
			{
				float penalizacion = PenalizacionPorResistencia(ia.resistenciaActual);
				float velocidadObjetivo = ia.velocidad * (1.0f - penalizacion);

				float tiempoAceleracion = ia.aceleracion * (1.0f + penalizacion);
				if (tiempoAceleracion < 0.01f) tiempoAceleracion = 0.01f;
				float tasaAceleracion = velocidadObjetivo / tiempoAceleracion;

				if (ia.velocidadActual < velocidadObjetivo)
				{
					ia.velocidadActual += tasaAceleracion * deltaTime;
					if (ia.velocidadActual > velocidadObjetivo) ia.velocidadActual = velocidadObjetivo;
				}
				else
				{
					ia.velocidadActual -= tasaAceleracion * deltaTime;
					if (ia.velocidadActual < velocidadObjetivo) ia.velocidadActual = velocidadObjetivo;
				}

				float paso = ia.velocidadActual * deltaTime;
				if (paso > distancia) paso = distancia;

				float metrosDeResistencia = ia.resistencia * ia.velocidad;
				if (metrosDeResistencia > 0.001f)
				{
					ia.resistenciaActual -= paso / metrosDeResistencia;
					if (ia.resistenciaActual < 0.0f) ia.resistenciaActual = 0.0f;
				}

				ia.pos.x += dx / distancia * paso;
				ia.pos.z += dz / distancia * paso;
				ia.yaw = atan2f(dx, dz);

				dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
				dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
				distancia = sqrtf(dx * dx + dz * dz);
			}

			// Aros de resistencia: cada rival los pasa una sola vez y tira su
			// probabilidad de lograr el quick time event (+25% de resistencia).
			for (size_t a = 0; a < arosResistencia.size(); a++)
			{
				if (a < ia.arosUsados.size() && ia.arosUsados[a])
					continue;

				float ax = arosResistencia[a].x - ia.pos.x;
				float az = arosResistencia[a].y - ia.pos.z;

				if (ax * ax + az * az <= radioAro * radioAro)
				{
					ia.arosUsados[a] = true;

					float suerte = (float)rand() / (float)RAND_MAX;
					if (suerte <= ia.probabilidadAro)
					{
						ia.resistenciaActual += 0.25f;
						if (ia.resistenciaActual > 1.0f) ia.resistenciaActual = 1.0f;
					}
				}
			}

			if (distancia <= radioPunto)
			{
				ia.siguientePunto++;

				if (ia.siguientePunto >= (int)puntosRuta.size())
				{
					ia.terminado = true;
					ia.tiempoLlegada = relojCarrera;
				}
			}

			ia.pos.y = terreno->getTerrainCollision(D3DXVECTOR2(ia.pos.x, ia.pos.z));
		}
	}

	// Progreso en la carrera: mas alto = mas adelante.
	float ProgresoCalacaIA(int i)
	{
		CalacaIA &ia = calacasIA[i];
		float dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
		float dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
		return (ia.siguientePunto - 1) * 10000.0f - sqrtf(dx * dx + dz * dz);
	}

	float ProgresoJugador()
	{
		float dx = puntosRuta[siguientePunto].x - playerPos.x;
		float dz = puntosRuta[siguientePunto].y - playerPos.z;
		return (siguientePunto - 1) * 10000.0f - sqrtf(dx * dx + dz * dz);
	}

	int RangoJugador()
	{
		bool llegue = (estadoCarrera == CarreraTerminada || siguientePunto >= (int)puntosRuta.size());
		int rango = 1;

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];

			if (llegue)
			{
				// Ya termine: solo me superan las que llegaron antes que yo.
				// (Comparar tiempos directamente evita que el float pierda
				// precision y empate llegadas con pocos segundos de diferencia.)
				if (ia.terminado && ia.tiempoLlegada < tiempoLlegadaJugador)
					rango++;
			}
			else if (ia.terminado || ProgresoCalacaIA(i) > ProgresoJugador())
			{
				rango++;
			}
		}

		return rango;
	}

	void ActualizarCarrera()
	{
		if (modoRuta || modoCamara)
			return;

		// Los dos primeros dias son de entrenamiento; la carrera es el tercero
		if (diaActual < 3)
		{
			ActualizarEntrenamiento();
			return;
		}

		if (estadoCarrera == CarreraSinRuta)
		{
			hudTexto.clear();
			return;
		}

		if (estadoCarrera == CarreraEsperando)
		{
			float dx = playerPos.x - puntosRuta[0].x;
			float dz = playerPos.z - puntosRuta[0].y;
			if (dx * dx + dz * dz <= radioMeta * radioMeta)
				hudTexto = L"Presiona  E  para empezar la carrera";
			else
				hudTexto.clear();
			return;
		}

		if (estadoCarrera == CarreraCuenta)
		{
			cuentaRegresiva -= deltaTime;

			if (cuentaRegresiva > 3.0f)		hudTexto = L"3";
			else if (cuentaRegresiva > 2.0f)	hudTexto = L"2";
			else if (cuentaRegresiva > 1.0f)	hudTexto = L"1";
			else if (cuentaRegresiva > 0.0f)	hudTexto = L"\u00A1Ya!";
			else
			{
				estadoCarrera = CarreraCorriendo;
				autoCorriendo = true;
				hudTexto.clear();
			}
			return;
		}

		if (estadoCarrera == CarreraCorriendo)
		{
			ActualizarCalacasIA();

			// Ulti: mientras esta activa se apaga en 2 segundos; si no, se
			// recarga sola un 3% por segundo.
			if (ultiTiempo > 0.0f)
			{
				ultiTiempo -= deltaTime;
				if (ultiTiempo < 0.0f) ultiTiempo = 0.0f;
				cargaUlti = ultiTiempo / 2.0f;
			}
			else
			{
				cargaUlti += 0.03f * deltaTime;
				if (cargaUlti > 1.0f) cargaUlti = 1.0f;
			}

			float dx = puntosRuta[siguientePunto].x - playerPos.x;
			float dz = puntosRuta[siguientePunto].y - playerPos.z;
			float distancia = sqrtf(dx * dx + dz * dz);

			// La calaca avanza sola hacia el siguiente punto
			if (autoCorriendo && distancia > 0.001f)
			{
				// Estadisticas: con la resistencia baja, la velocidad maxima y la
				// aceleracion se reducen, asi que cuesta mas llegar a la meta.
				float penalizacion = PenalizacionResistencia();
				float velocidadObjetivo = velocidadCarrera * (1.0f - penalizacion);

				// La ulti da un 30% mas de velocidad mientras esta activa
				if (ultiTiempo > 0.0f)
					velocidadObjetivo *= 1.3f;

				float tiempoAceleracion = TiempoAceleracion();
				if (tiempoAceleracion < 0.01f) tiempoAceleracion = 0.01f;
				float tasaAceleracion = velocidadObjetivo / tiempoAceleracion;

				if (velocidadActual < velocidadObjetivo)
				{
					velocidadActual += tasaAceleracion * deltaTime;
					if (velocidadActual > velocidadObjetivo) velocidadActual = velocidadObjetivo;
				}
				else
				{
					velocidadActual -= tasaAceleracion * deltaTime;
					if (velocidadActual < velocidadObjetivo) velocidadActual = velocidadObjetivo;
				}

				float paso = velocidadActual * deltaTime;
				if (paso > distancia) paso = distancia;

				// La resistencia baja cada metro: a velocidad maxima dura
				// 'resistenciaCarrera' segundos.
				float metrosDeResistencia = resistenciaCarrera * velocidadCarrera;
				if (metrosDeResistencia > 0.001f)
				{
					resistenciaActual -= paso / metrosDeResistencia;
					if (resistenciaActual < 0.0f) resistenciaActual = 0.0f;
				}

				playerPos.x += dx / distancia * paso;
				playerPos.z += dz / distancia * paso;

				// La calaca mira hacia donde corre (la camara ya no define su rumbo)
				yawCalaca = atan2f(dx, dz);

				dx = puntosRuta[siguientePunto].x - playerPos.x;
				dz = puntosRuta[siguientePunto].y - playerPos.z;
				distancia = sqrtf(dx * dx + dz * dz);
			}

			// Aros de resistencia: al pasar sobre uno disponible se lanza el QTE.
			// Cada aro solo se usa una vez por carrera.
			if (!qteActivo)
			{
				for (size_t i = 0; i < arosResistencia.size(); i++)
				{
					if (i < arosUsados.size() && arosUsados[i])
						continue;

					float ax = arosResistencia[i].x - playerPos.x;
					float az = arosResistencia[i].y - playerPos.z;

					if (ax * ax + az * az <= radioAro * radioAro)
					{
						arosUsados[i] = true;
						IniciarQTE();
						break;
					}
				}
			}

			// Contrarreloj del QTE: si se agota, no se gana ni se pierde nada
			if (qteActivo)
			{
				qteTiempo -= deltaTime;
				if (qteTiempo <= 0.0f)
				{
					qteTiempo = 0.0f;
					qteActivo = false;
				}
			}

			if (distancia <= radioPunto)
			{
				siguientePunto++;

				if (siguientePunto >= (int)puntosRuta.size())
				{
					// Fin de la carrera: se devuelve el control al jugador
					tiempoLlegadaJugador = relojCarrera;
					estadoCarrera = CarreraTerminada;
					autoCorriendo = false;
					qteActivo = false;
					tiempoMensaje = 6.0f;

					wchar_t final[64];
					swprintf_s(final, L"Carrera terminada!   %d\u00B0 lugar", RangoJugador());
					hudTexto = final;
					return;
				}
			}

			wchar_t buffer[128];
			swprintf_s(buffer, L"Punto %d de %d    (%.0f m)",
				siguientePunto, (int)puntosRuta.size() - 1, distancia);
			hudTexto = buffer;
			return;
		}

		if (estadoCarrera == CarreraTerminada)
		{
			tiempoMensaje -= deltaTime;

			if (tiempoMensaje <= 0.0f)
			{
				estadoCarrera = CarreraEsperando;
				siguientePunto = 1;
				hudTexto.clear();
			}
			else
			{
				wchar_t final[64];
				swprintf_s(final, L"Carrera terminada!   %d\u00B0 lugar", RangoJugador());
				hudTexto = final;
			}
		}
	}

	void DibujarHUD()
	{
		if (!hudTexto.empty())
		{
			// La cuenta regresiva se dibuja con fuente grande y centrada en pantalla
			bool grande = (estadoCarrera == CarreraCuenta);
			int ancho = grande ? 640 : (modoRuta ? 1250 : hudAncho);
			int alto = grande ? 360 : (modoRuta ? 190 : hudAlto);	// el editor usa 3 lineas
			int fuente = grande ? 240 : (modoRuta ? 26 : hudFuente);

			if (hudTexto != hudTextoDibujado || ancho != hudAnchoDibujado || alto != hudAltoDibujado)
			{
				GenerarTextoBGRA(hudTexto.c_str(), ancho, alto, fuente, 255, 255, 255, hudPixeles);
				hudQuad->SetTextureFromPixels((UINT)ancho, (UINT)alto, hudPixeles.data());
				hudTextoDibujado = hudTexto;
				hudAnchoDibujado = ancho;
				hudAltoDibujado = alto;
			}

			d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
			d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

			float x = (hudScreenW - ancho) * 0.5f;
			float y = grande ? (hudScreenH - alto) * 0.35f : hudScreenH * hudY;
			hudQuad->DrawHUD(x, y, (float)ancho, (float)alto, hudScreenW, hudScreenH);

			d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
			d3d11DevCon->OMSetDepthStencilState(NULL, 0);
		}
		else
		{
			hudTextoDibujado.clear();
		}

		if (estadoCarrera == CarreraCorriendo && !modoRuta)
		{
			DibujarFlechaHUD();
			DibujarPanelEstadisticas();
			DibujarQTE();
			DibujarUlti();
		}

		if (!modoRuta)
		{
			DibujarPosicion();
			DibujarTablaStats();
		}

		DibujarBanner();
	}

	// Aviso grande al empezar cada dia: titulo y subtitulo sobre un fondo
	// semitransparente. Se queda 5 segundos y se va solo.
	void DibujarBanner()
	{
		if (bannerTiempo <= 0.0f)
			return;

		bannerTiempo -= deltaTime;

		const int anchoTitulo = 1100, altoTitulo = 160, fuenteTitulo = 96;
		const int anchoSub = 1100, altoSub = 70, fuenteSub = 40;
		const float anchoFondo = 1000.0f;
		const float altoFondo = 250.0f;
		float xFondo = (hudScreenW - anchoFondo) * 0.5f;
		float yFondo = hudScreenH * 0.10f;

		if (bannerTitulo != bannerTituloDibujado)
		{
			GenerarTextoBGRA(bannerTitulo.c_str(), anchoTitulo, altoTitulo, fuenteTitulo,
				255, 255, 255, bannerPixelesTitulo);
			hudBannerTitulo->SetTextureFromPixels((UINT)anchoTitulo, (UINT)altoTitulo,
				bannerPixelesTitulo.data());
			bannerTituloDibujado = bannerTitulo;
		}

		if (bannerSub != bannerSubDibujado)
		{
			GenerarTextoBGRA(bannerSub.c_str(), anchoSub, altoSub, fuenteSub,
				230, 220, 190, bannerPixelesSub);
			hudBannerSub->SetTextureFromPixels((UINT)anchoSub, (UINT)altoSub,
				bannerPixelesSub.data());
			bannerSubDibujado = bannerSub;
		}

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

		hudBannerFondo->DrawHUD(xFondo, yFondo, anchoFondo, altoFondo, hudScreenW, hudScreenH);
		hudBannerTitulo->DrawHUD(xFondo + (anchoFondo - anchoTitulo) * 0.5f, yFondo + 24.0f,
			(float)anchoTitulo, (float)altoTitulo, hudScreenW, hudScreenH);
		hudBannerSub->DrawHUD(xFondo + (anchoFondo - anchoSub) * 0.5f,
			yFondo + 24.0f + altoTitulo + 4.0f,
			(float)anchoSub, (float)altoSub, hudScreenW, hudScreenH);

		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Quick time event del aro: las 4 flechas de la secuencia arriba al centro y
	// una barra que se encoge con el contrarreloj de 2 segundos.
	void DibujarQTE()
	{
		if (!qteActivo)
			return;

		const float lado = 96.0f;
		const float separacion = 20.0f;
		float total = lado * 4.0f + separacion * 3.0f;
		float x0 = (hudScreenW - total) * 0.5f;
		float y = hudScreenH * 0.16f;

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

		for (int i = 0; i < 4; i++)
		{
			int dir = qteSecuencia[i];
			bool acertada = (i < qteProgreso);

			// La textura de cada flecha solo se regenera si cambia de direccion o de color
			if (qteSlotDir[i] != dir || qteSlotVerde[i] != acertada)
			{
				if (acertada)
					GenerarFlechaBGRA(128, dir * 90.0f, 80, 230, 110, qtePixeles[i]);
				else
					GenerarFlechaBGRA(128, dir * 90.0f, 255, 255, 255, qtePixeles[i]);

				hudQte[i]->SetTextureFromPixels(128, 128, qtePixeles[i].data());
				qteSlotDir[i] = dir;
				qteSlotVerde[i] = acertada;
			}

			hudQte[i]->DrawHUD(x0 + i * (lado + separacion), y, lado, lado, hudScreenW, hudScreenH);
		}

		// Contrarreloj: pista oscura y relleno que se encoge
		float barraAlto = 14.0f;
		float barraY = y + lado + 16.0f;
		float fraccion = qteTiempo / 2.0f;
		if (fraccion < 0.0f) fraccion = 0.0f;
		if (fraccion > 1.0f) fraccion = 1.0f;

		hudQteBarraFondo->DrawHUD(x0, barraY, total, barraAlto, hudScreenW, hudScreenH);
		if (fraccion > 0.001f)
			hudQteBarra->DrawHUD(x0, barraY, total * fraccion, barraAlto, hudScreenW, hudScreenH);

		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Ulti en la esquina superior izquierda: circulo de fuego con calavera.
	// Cuando esta completa aparece la tecla de espacio latiendo abajo al centro.
	void DibujarUlti()
	{
		if (estadoCarrera != CarreraCorriendo || modoRuta)
			return;

		int paso = (int)(cargaUlti * 100.0f);
		if (paso > 100) paso = 100;

		// Se regenera al cambiar el porcentaje y, ya llena, cada 0.12 s para
		// que las llamas titilen.
		ultiFlicker -= deltaTime;
		if (paso != ultiCargaDibujada || (paso >= 100 && ultiFlicker <= 0.0f))
		{
			GenerarUltiBGRA(ultiAncho, ultiAlto, cargaUlti, ultiSemilla, ultiPixeles);
			hudUlti->SetTextureFromPixels((UINT)ultiAncho, (UINT)ultiAlto,
				ultiPixeles.data());
			ultiCargaDibujada = paso;
			ultiSemilla++;
			ultiFlicker = 0.12f;
		}

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

		hudUlti->DrawHUD(30.0f, 30.0f, (float)ultiAncho, (float)ultiAlto,
			hudScreenW, hudScreenH);

		// Tecla de espacio: late mientras la ulti esta lista
		if (paso >= 100)
		{
			float escala = 1.02f + 0.08f * sinf(relojCarrera * 9.0f);
			float ancho = teclaAncho * escala;
			float alto = teclaAlto * escala;
			float x = (hudScreenW - ancho) * 0.5f;
			float y = hudScreenH - alto - 60.0f;

			hudTeclaEspacio->DrawHUD(x, y, ancho, alto, hudScreenW, hudScreenH);
		}

		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Marcador de posicion (1 a 5) en la esquina inferior izquierda del HUD.
	// Solo se regenera la textura cuando cambia el puesto.
	void DibujarPosicion()
	{
		if (estadoCarrera != CarreraCorriendo && estadoCarrera != CarreraTerminada)
			return;

		int rango = RangoJugador();

		if (rango != rangoDibujado)
		{
			wchar_t texto[32];
			swprintf_s(texto, L"%d\u00B0 / 5", rango);

			unsigned char r = 235, g = 235, b = 235;
			if (rango == 1)			{ r = 255; g = 210; b = 60; }	// dorado
			else if (rango == 2)	{ r = 220; g = 220; b = 230; }	// plata
			else if (rango == 3)	{ r = 205; g = 130; b = 70; }	// bronce

			GenerarTextoBGRA(texto, posicionAncho, posicionAlto, posicionFuente,
				r, g, b, posicionPixeles);
			hudPosicion->SetTextureFromPixels((UINT)posicionAncho, (UINT)posicionAlto,
				posicionPixeles.data());
			rangoDibujado = rango;
		}

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

		float x = 30.0f;
		float y = hudScreenH - posicionAlto - 70.0f;
		hudPosicion->DrawHUD(x, y, (float)posicionAncho, (float)posicionAlto, hudScreenW, hudScreenH);

		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Tabla de stats de las rivales en la esquina superior derecha. En los dias
	// de entrenamiento los valores se ocultan con "?" y el dia de la carrera se
	// muestran los reales. Solo se regenera la textura cuando cambian los datos.
	void DibujarTablaStats()
	{
		if (bannerTiempo > 0.0f)
			return;

		std::wstring clave = std::to_wstring(diaActual);
		for (int i = 0; i < 4; i++)
		{
			CalcularStatsIA(calacasIA[i]);
			clave += L"|";
			clave += std::to_wstring(calacasIA[i].mejoras[0]);
			clave += L",";
			clave += std::to_wstring(calacasIA[i].mejoras[1]);
			clave += L",";
			clave += std::to_wstring(calacasIA[i].mejoras[2]);
		}

		if (clave != tablaClave)
		{
			const wchar_t* celdas[5][4];
			wchar_t textos[4][3][32];

			celdas[0][0] = L"Calaca";
			celdas[0][1] = L"Velocidad";
			celdas[0][2] = L"Aceleraci\u00F3n";
			celdas[0][3] = L"Resistencia";

			for (int i = 0; i < 4; i++)
			{
				celdas[i + 1][0] = calacasIA[i].nombre;

				if (diaActual >= 3)
				{
					swprintf_s(textos[i][0], L"%.1f", calacasIA[i].velocidad);
					swprintf_s(textos[i][1], L"%.2f s", calacasIA[i].aceleracion);
					swprintf_s(textos[i][2], L"%.1f", calacasIA[i].resistencia);
					celdas[i + 1][1] = textos[i][0];
					celdas[i + 1][2] = textos[i][1];
					celdas[i + 1][3] = textos[i][2];
				}
				else
				{
					celdas[i + 1][1] = L"?";
					celdas[i + 1][2] = L"?";
					celdas[i + 1][3] = L"?";
				}
			}

			GenerarTablaBGRA(tablaAncho, tablaAlto, celdas, tablaPixeles);
			hudTabla->SetTextureFromPixels((UINT)tablaAncho, (UINT)tablaAlto,
				tablaPixeles.data());
			tablaClave = clave;
		}

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);

		float x = (float)(hudScreenW - tablaAncho - 20);
		float y = 20.0f;
		hudTabla->DrawHUD(x, y, (float)tablaAncho, (float)tablaAlto, hudScreenW, hudScreenH);

		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Pinta un objeto con un color plano (textura 2x2 generada en CPU).
	static void ColorPlano(MPrimitives* objeto, unsigned char r, unsigned char g, unsigned char b,
		unsigned char a = 255)
	{
		unsigned char pixeles[16];
		for (int i = 0; i < 4; i++)
		{
			pixeles[i * 4 + 0] = b;
			pixeles[i * 4 + 1] = g;
			pixeles[i * 4 + 2] = r;
			pixeles[i * 4 + 3] = a;
		}
		objeto->SetTextureFromPixels(2, 2, pixeles);
	}

	// Dibuja un anillo acostado en el suelo en cada punto del circuito.
	void DibujarMarcadores()
	{
		// Puntos de entrenamiento: color segun la barra del HUD, gris si ya se uso en esta hora.
		// El dia de la carrera ya no se muestran.
		for (size_t i = 0; diaActual < 3 && i < puntosEntrenamiento.size(); i++)
		{
			bool usado = puntosEntrenamiento[i].usadoSlot;
			MPrimitives* anillo = marcadorEntUsado;
			if (!usado)
			{
				if (puntosEntrenamiento[i].tipo == 0)		anillo = marcadorEntVel;
				else if (puntosEntrenamiento[i].tipo == 1)	anillo = marcadorEntAcel;
				else										anillo = marcadorEntRes;
			}

			float radio = radioEntrenamiento;
			if (!usado)
				radio *= (1.0f + 0.06f * sinf(g_Time * 6.0f));	// late para llamar la atencion

			float y = terreno->getTerrainCollision(puntosEntrenamiento[i].pos) + alturaMarcador;

			D3DXMATRIX escala, rotX, traslacion;
			D3DXMatrixScaling(&escala, radio, radio, radio);
			D3DXMatrixRotationX(&rotX, -D3DX_PI * 0.5f);
			D3DXMatrixTranslation(&traslacion, puntosEntrenamiento[i].pos.x, y, puntosEntrenamiento[i].pos.y);

			anillo->UpdateScene(escala * rotX * traslacion);
			anillo->DrawScene(camara);
		}

		if (puntosRuta.empty())
			return;

		for (int i = 0; i < (int)puntosRuta.size(); i++)
		{
			MPrimitives* anillo = marcadorPunto;
			float radio = radioPunto;

			if (i == 0)
			{
				anillo = marcadorMeta;
				radio = radioMeta;
			}
			else if (i == siguientePunto && (estadoCarrera == CarreraCorriendo || estadoCarrera == CarreraCuenta))
			{
				anillo = marcadorSig;
				radio = radioPunto * (1.0f + 0.06f * sinf(g_Time * 6.0f));	// late para llamar la atencion
			}

			float y = terreno->getTerrainCollision(puntosRuta[i]) + alturaMarcador;

			D3DXMATRIX escala, rotX, traslacion;
			D3DXMatrixScaling(&escala, radio, radio, radio);	// tambien el grosor del tubo
			D3DXMatrixRotationX(&rotX, -D3DX_PI * 0.5f);		// el toro nace en el plano XY: acostarlo
			D3DXMatrixTranslation(&traslacion, puntosRuta[i].x, y, puntosRuta[i].y);

			anillo->UpdateScene(escala * rotX * traslacion);
			anillo->DrawScene(camara);
		}

		// Aros de resistencia: magenta si estan disponibles, gris si ya se usaron
		for (size_t i = 0; i < arosResistencia.size(); i++)
		{
			bool usado = (i < arosUsados.size() && arosUsados[i]);
			MPrimitives* anillo = usado ? marcadorAroUsado : marcadorAro;
			float radio = radioAro;

			if (!usado)
				radio *= (1.0f + 0.06f * sinf(g_Time * 6.0f));	// late para llamar la atencion

			float y = terreno->getTerrainCollision(arosResistencia[i]) + alturaMarcador;

			D3DXMATRIX escala, rotX, traslacion;
			D3DXMatrixScaling(&escala, radio, radio, radio);
			D3DXMatrixRotationX(&rotX, -D3DX_PI * 0.5f);
			D3DXMatrixTranslation(&traslacion, arosResistencia[i].x, y, arosResistencia[i].y);

			anillo->UpdateScene(escala * rotX * traslacion);
			anillo->DrawScene(camara);
		}
	}

	// Flecha en pantalla que apunta al siguiente checkpoint.
	void DibujarFlechaHUD()
	{
		float dx = puntosRuta[siguientePunto].x - playerPos.x;
		float dz = puntosRuta[siguientePunto].y - playerPos.z;

		if (dx * dx + dz * dz < 1.0f)
			return;

		// Angulo de la direccion respecto a la camara: 0 = de frente, 90 = a la derecha
		D3DXVECTOR3 dir(dx, 0.0f, dz);
		D3DXVec3Normalize(&dir, &dir);
		float derecha = D3DXVec3Dot(&dir, &camara->Right);
		float adelante = D3DXVec3Dot(&dir, &camara->Forward);
		float grados = atan2f(derecha, adelante) * 180.0f / D3DX_PI;

		if (fabsf(grados - gradosFlecha) > 3.0f)
		{
			GenerarFlechaBGRA(128, grados, 255, 255, 255, flechaPixeles);
			hudFlecha->SetTextureFromPixels(128, 128, flechaPixeles.data());
			gradosFlecha = grados;
		}

		float x = (hudScreenW - tamFlecha) * 0.5f;
		float y = hudScreenH * hudY - tamFlecha - 10.0f;

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);
		hudFlecha->DrawHUD(x, y, tamFlecha, tamFlecha, hudScreenW, hudScreenH);
		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	}

	// Panel de estadisticas en la esquina inferior derecha: velocidad actual
	// contra la maxima, aceleracion (llena mientras acelera) y resistencia.
	void DibujarPanelEstadisticas()
	{
		float penalizacion = PenalizacionResistencia();
		int porcentaje = (int)(penalizacion * 100.0f + 0.5f);
		float velocidadObjetivo = velocidadCarrera * (1.0f - penalizacion);
		bool acelerando = (velocidadActual < velocidadObjetivo - 0.01f);

		// La textura solo se regenera cuando cambia algun valor visible
		wchar_t clave[128];
		swprintf_s(clave, L"%.1f|%.1f|%.0f|%d|%d",
			velocidadActual, TiempoAceleracion(), resistenciaActual * 100.0f,
			porcentaje, acelerando ? 1 : 0);

		if (panelClave != clave)
		{
			GenerarPanelBGRA(panelAncho, panelAlto,
				velocidadActual, velocidadCarrera,
				TiempoAceleracion(), acelerando ? 1.0f : 0.0f,
				resistenciaActual, porcentaje, panelPixeles);
			hudPanel->SetTextureFromPixels((UINT)panelAncho, (UINT)panelAlto, panelPixeles.data());
			panelClave = clave;
		}

		float x = hudScreenW - panelAncho - 24.0f;
		float y = hudScreenH - panelAlto - 90.0f;	// arriba de la barra de tareas

		d3d11DevCon->OMSetDepthStencilState(depthDisabledState, 0);
		d3d11DevCon->OMSetBlendState(alphaBlendState, NULL, 0xFFFFFFFF);
		hudPanel->DrawHUD(x, y, (float)panelAncho, (float)panelAlto, hudScreenW, hudScreenH);
		d3d11DevCon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		d3d11DevCon->OMSetDepthStencilState(NULL, 0);
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

		// HUD de texto y estado del circuito
		hudScreenW = Width;
		hudScreenH = Height;
		hudQuad = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		d3d11Device->CreateBlendState(&blendDesc, &alphaBlendState);

		// Marcadores del circuito: anillos de color plano en el suelo.
		// El toro se crea con radio 1 y se escala al radio real de cada punto.
		marcadorMeta = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorSig = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorPunto = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorAro = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorAroUsado = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorEntVel = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorEntAcel = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorEntRes = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		marcadorEntUsado = new MPrimitives(d3d11Device, d3d11DevCon, 8, 40, 1.0f, 0.10f, L"", L"", ShaderTexto);
		hudFlecha = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudPanel = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		for (int i = 0; i < 4; i++)
			hudQte[i] = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudQteBarra = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudQteBarraFondo = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudBannerFondo = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudBannerTitulo = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudBannerSub = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudPosicion = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudTabla = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudUlti = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		hudTeclaEspacio = new MPrimitives(d3d11Device, d3d11DevCon, 2.0f, 2.0f, L"", camara, ShaderTexto);
		GenerarTeclaEspacioBGRA(teclaAncho, teclaAlto, teclaEspacioPixeles);
		hudTeclaEspacio->SetTextureFromPixels((UINT)teclaAncho, (UINT)teclaAlto,
			teclaEspacioPixeles.data());

		// Diamante verde de la calaca del jugador y calacas rivales
		diamanteJugador = new MPrimitives(d3d11Device, d3d11DevCon, L"", ShaderDiffuse);
		for (int i = 0; i < 4; i++)
			calacasIA[i].modelo = new MPrimitives(d3d11Device, d3d11DevCon,
				"Models/calaca.txt", L"Texturas/calacaColor.png", L"", ShaderDiffuse);

		const wchar_t* nombresIA[4] = {
			L"Calaca Elite", L"Calaca Experimentada",
			L"Calaca Aficionado", L"Calaca Principiante"
		};
		float porcentajesIA[4] = { 0.150f, 0.120f, 0.100f, 0.100f };
		float probabilidadesIA[4] = { 0.95f, 0.80f, 0.65f, 0.50f };

		for (int i = 0; i < 4; i++)
		{
			calacasIA[i].nombre = nombresIA[i];
			calacasIA[i].porcentajeMejora = porcentajesIA[i];
			calacasIA[i].probabilidadAro = probabilidadesIA[i];
			calacasIA[i].mejoras[0] = calacasIA[i].mejoras[1] = calacasIA[i].mejoras[2] = 0;
			calacasIA[i].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			calacasIA[i].velocidad = 0.0f;
			calacasIA[i].aceleracion = 0.0f;
			calacasIA[i].resistencia = 0.0f;
			calacasIA[i].velocidadActual = 0.0f;
			calacasIA[i].resistenciaActual = 1.0f;
			calacasIA[i].siguientePunto = 1;
			calacasIA[i].terminado = false;
			calacasIA[i].tiempoLlegada = 0.0f;
			calacasIA[i].yaw = 0.0f;
		}

		ColorPlano(marcadorMeta, 255, 200, 60);		// dorado
		ColorPlano(marcadorSig, 80, 255, 120);		// verde
		ColorPlano(marcadorPunto, 90, 160, 255);	// azul
		ColorPlano(marcadorAro, 255, 80, 220);		// magenta: aro disponible
		ColorPlano(marcadorAroUsado, 90, 90, 90);	// gris: aro ya usado
		ColorPlano(marcadorEntVel, 80, 200, 255);	// cian: velocidad
		ColorPlano(marcadorEntAcel, 255, 170, 60);	// naranja: aceleracion
		ColorPlano(marcadorEntRes, 80, 230, 110);	// verde: resistencia
		ColorPlano(marcadorEntUsado, 90, 90, 90);	// gris: ya usado en esta hora
		ColorPlano(hudQteBarra, 255, 210, 70);		// amarillo: tiempo restante
		ColorPlano(hudQteBarraFondo, 40, 45, 60);	// pista oscura
		ColorPlano(hudBannerFondo, 15, 18, 28, 170);	// fondo semitransparente del aviso
		ColorPlano(diamanteJugador, 40, 230, 40);	// diamante verde de la calaca del jugador

		srand((unsigned int)time(NULL));			// secuencias aleatorias del QTE

		CargarRuta();
		IniciarDias();
		CargarVistas();
		vKeyLast = false;
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
		castleWalls[19] = { 147.0f, 151.0f, 247.0f, 255.0f }; // escalera
		audio.Init(L"Sonidos/ambient.wav", audioVolume);
		return true;
	}

	// Coloca la camara en la vista guardada para el tramo actual de la carrera.
	// Devuelve false si no hay vista para este tramo (se sigue en tercera persona).
	bool AplicarVistaCarrera()
	{
		if (!autoCorriendo || siguientePunto < 1)
			return false;

		int indice = siguientePunto - 1;
		if (indice >= (int)vistasCamara.size())
			return false;

		camara->Position = vistasCamara[indice].pos;

		if (vistasSeguir)
		{
			// Solo se sigue el movimiento horizontal: la altura del terreno y el
			// rebote de la calaca no mueven el encuadre, asi la camara no tiembla.
			float dx = playerPos.x - camara->Position.x;
			float dz = playerPos.z - camara->Position.z;

			if (dx * dx + dz * dz > 0.01f)
			{
				float yawObjetivo = atan2f(dx, dz);

				if (indice != vistaActual)
				{
					// Camara nueva: se encuadra de golpe (corte limpio, sin giro raro)
					camara->Yaw = yawObjetivo;
				}
				else
				{
					// Interpolacion exponencial: giro suave e independiente de los FPS
					float diferencia = yawObjetivo - camara->Yaw;

					while (diferencia > D3DX_PI) diferencia -= 2.0f * D3DX_PI;
					while (diferencia < -D3DX_PI) diferencia += 2.0f * D3DX_PI;

					float factor = 1.0f - expf(-suavizadoCamara * deltaTime);
					camara->Yaw += diferencia * factor;
				}
			}

			// El encuadre vertical es el que el jugador dejo con P
			camara->Pitch = vistasCamara[indice].pitch;
		}
		else
		{
			camara->Yaw = vistasCamara[indice].yaw;
			camara->Pitch = vistasCamara[indice].pitch;
		}

		vistaActual = indice;
		camara->UpdateCamera(0.0f, 0.0f, 0.0f, 0.0f);
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

		// La luz la fija el horario del entrenamiento (AplicarLuzDelDia),
		// asi que aqui ya no hay ciclo automatico de dia y noche.


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
		// En la carrera automatica la calaca mira hacia donde corre (la camara puede
		// estar en cualquier lado); el resto del tiempo mira hacia donde apunta la vista.
		D3DXMatrixRotationY(&rotCalaca, (autoCorriendo ? yawCalaca : camara->Yaw) + D3DX_PI);

		// Pequeño rebote para que se note que va corriendo
		float reboteCalaca = 0.0f;
		if (autoCorriendo)
			reboteCalaca = fabsf(sinf(g_Time * 60.0f)) * 0.4f;

		D3DXMatrixTranslation(&lugarCalaca, playerPos.x, playerPos.y + 0.05f + reboteCalaca, playerPos.z);

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

		// Pasto con movimiento (48 manojos)
		static float pastoXZ[150][2] = { {67.1f, 200.9f}, {186.9f, 147.2f}, {80.1f, 227.1f}, {231.0f, 155.7f}, {25.8f, 119.6f}, {174.3f, 42.7f}, {227.4f, 84.2f}, {248.1f, 182.1f}, {151.9f, 54.5f}, {267.8f, 119.0f}, {64.5f, 233.5f}, {140.5f, 165.8f}, {112.4f, 148.6f}, {224.0f, 78.2f}, {173.1f, 211.1f}, {260.4f, 251.3f}, {129.4f, 118.1f}, {140.1f, 123.4f}, {151.5f, 150.3f}, {165.4f, 215.8f}, {166.9f, 183.8f}, {113.8f, 261.4f}, {41.7f, 232.5f}, {223.1f, 120.6f}, {199.3f, 197.4f}, {254.2f, 269.4f}, {128.8f, 277.2f}, {270.2f, 175.6f}, {250.2f, 150.2f}, {120.8f, 88.0f}, {222.3f, 148.2f}, {90.8f, 98.9f}, {132.0f, 68.5f}, {254.3f, 275.8f}, {215.2f, 147.1f}, {89.3f, 269.5f}, {159.0f, 210.8f}, {32.2f, 79.1f}, {238.8f, 227.8f}, {133.7f, 173.2f}, {241.6f, 103.5f}, {199.8f, 268.3f}, {189.2f, 242.2f}, {49.2f, 49.0f}, {60.9f, 246.2f}, {192.1f, 57.3f}, {27.0f, 143.3f}, {41.2f, 219.8f}, {69.7f, 206.4f}, {89.5f, 202.7f}, {124.2f, 103.9f}, {153.3f, 165.1f}, {178.3f, 154.6f}, {104.1f, 203.4f}, {159.1f, 168.0f}, {106.1f, 278.3f}, {52.4f, 44.4f}, {218.8f, 139.3f}, {189.4f, 137.9f}, {156.9f, 124.3f}, {127.4f, 82.5f}, {141.3f, 85.3f}, {123.3f, 149.1f}, {186.1f, 270.6f}, {150.4f, 207.9f}, {252.1f, 223.8f}, {184.8f, 218.4f}, {195.3f, 244.0f}, {235.3f, 270.7f}, {104.7f, 104.5f}, {124.8f, 62.1f}, {166.1f, 45.3f}, {102.2f, 265.6f}, {95.7f, 121.6f}, {264.9f, 144.1f}, {136.4f, 182.4f}, {123.1f, 94.5f}, {33.8f, 231.7f}, {263.4f, 204.7f}, {119.4f, 59.4f}, {229.6f, 240.1f}, {45.5f, 254.4f}, {232.9f, 179.7f}, {229.0f, 111.2f}, {93.8f, 115.0f}, {198.6f, 191.6f}, {262.4f, 162.3f}, {210.6f, 158.8f}, {114.8f, 160.8f}, {206.0f, 150.9f}, {66.3f, 242.0f}, {230.7f, 256.1f}, {223.3f, 255.2f}, {272.4f, 124.5f}, {265.2f, 263.9f}, {179.8f, 217.7f}, {250.1f, 117.4f}, {232.8f, 117.4f}, {31.7f, 194.7f}, {253.2f, 156.1f}, {133.2f, 188.4f}, {134.7f, 121.4f}, {149.8f, 92.7f}, {162.1f, 142.0f}, {212.8f, 211.4f}, {194.4f, 173.3f}, {152.5f, 142.7f}, {177.6f, 201.7f}, {241.2f, 252.1f}, {236.9f, 92.5f}, {80.6f, 200.0f}, {118.9f, 74.3f}, {61.0f, 39.7f}, {202.6f, 130.5f}, {56.4f, 249.5f}, {209.8f, 176.6f}, {138.2f, 152.7f}, {187.2f, 187.1f}, {118.3f, 113.3f}, {217.5f, 187.1f}, {30.1f, 189.0f}, {174.6f, 173.7f}, {108.4f, 108.7f}, {128.3f, 219.0f}, {76.8f, 203.4f}, {157.7f, 194.2f}, {27.3f, 102.2f}, {116.2f, 225.1f}, {271.0f, 222.9f}, {133.4f, 201.2f}, {166.9f, 117.8f}, {102.6f, 109.2f}, {72.6f, 257.9f}, {133.5f, 76.1f}, {246.5f, 110.9f}, {201.4f, 147.4f}, {108.8f, 252.3f}, {198.2f, 221.9f}, {121.9f, 274.0f}, {93.1f, 227.5f}, {226.4f, 247.4f}, {252.9f, 235.0f}, {274.5f, 152.3f}, {203.9f, 156.0f}, {272.2f, 192.0f}, {134.8f, 194.3f}, {140.9f, 219.7f}, {124.9f, 210.8f}, {186.2f, 233.7f}, {25.9f, 111.8f} };
		for (int i = 0; i < 150; i++)
		{
			D3DXMATRIX lugarPasto;
			D3DXVECTOR2 xzPasto(pastoXZ[i][0], pastoXZ[i][1]);
			float yPasto = terreno->getTerrainCollision(xzPasto) +0.75f;
			D3DXMatrixTranslation(&lugarPasto, xzPasto.x, yPasto, xzPasto.y);
			pasto[i]->UpdateScene(lugarPasto);
		}
		
		
		if (playerPos.x < 25.0f) playerPos.x = 25.0f;
		if (playerPos.x > 280.0f) playerPos.x = 280.0f;
		if (playerPos.z < 25.0f) playerPos.z = 25.0f;
		if (playerPos.z > 280.0f) playerPos.z = 280.0f;

		// Durante la carrera automatica la calaca sigue su carril: si chocara con un
		// objeto quedaria empujada hacia atras y la carrera se trabaria.
		if (!autoCorriendo)
		{
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
		}

		playerPos.y = terreno->getTerrainCollision(D3DXVECTOR2(playerPos.x, playerPos.z));

		if (modoCamara)
		{
			// La camara la controla el jugador en el editor de vistas
			vistaCarreraActiva = false;
		}
		else
		{
			vistaCarreraActiva = AplicarVistaCarrera();

			if (!vistaCarreraActiva)
			{
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
		if (thirdPerson)
		{
			calaca->DrawScene(camara);

			// Diamante verde flotando sobre la cabeza de la calaca del jugador,
			// solo mientras dura la carrera.
			if (estadoCarrera == CarreraCuenta || estadoCarrera == CarreraCorriendo ||
				estadoCarrera == CarreraTerminada)
			{
				D3DXMATRIX escalaDiamante, rotDiamante, lugarDiamante;
				float reboteDiamante = autoCorriendo ? fabsf(sinf(g_Time * 60.0f)) * 0.4f : 0.0f;
				D3DXMatrixScaling(&escalaDiamante, 0.45f, 0.85f, 0.45f);
				D3DXMatrixRotationY(&rotDiamante, g_Time * 2.5f);
				D3DXMatrixTranslation(&lugarDiamante, playerPos.x,
					playerPos.y + 4.05f + reboteDiamante, playerPos.z);
				diamanteJugador->UpdateScene(escalaDiamante * rotDiamante * lugarDiamante);
				diamanteJugador->DrawScene(camara);
			}
		}
		calaca2->DrawScene(camara);

		// Calacas rivales: solo existen durante la carrera
		if (estadoCarrera == CarreraCuenta || estadoCarrera == CarreraCorriendo ||
			estadoCarrera == CarreraTerminada)
		{
			for (int i = 0; i < 4; i++)
			{
				D3DXMATRIX escalaIA, rotIA, lugarIA;
				D3DXMatrixScaling(&escalaIA, 2.0f, 2.0f, 2.0f);

				float reboteIA = 0.0f;
				if (estadoCarrera == CarreraCorriendo && !calacasIA[i].terminado)
					reboteIA = fabsf(sinf(g_Time * 60.0f + i * 1.7f)) * 0.4f;

				D3DXMatrixRotationY(&rotIA, calacasIA[i].yaw + D3DX_PI);
				D3DXMatrixTranslation(&lugarIA, calacasIA[i].pos.x,
					calacasIA[i].pos.y + 0.05f + reboteIA, calacasIA[i].pos.z);

				calacasIA[i].modelo->UpdateScene(escalaIA * rotIA * lugarIA);
				calacasIA[i].modelo->DrawScene(camara);
			}
		}
		castillo->DrawScene(camara);
		arbolmuerto1->DrawScene(camara);
		arbolmuerto2->DrawScene(camara);
		for (int i = 0; i < 150; i++)
			pasto[i]->DrawScene(camara);
		agua->DrawScene(camara);


		terreno->DrawScene(camara);
		skydomo->DrawScene(skyCamara);

		DibujarMarcadores();

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

		DibujarHUD();

		//Present the backbuffer to the screen
		HRESULT hr = SwapChain->Present(1, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HRESULT reason = d3d11Device->GetDeviceRemovedReason();
			MessageBox(NULL,
				reason == DXGI_ERROR_DEVICE_HUNG
					? L"La GPU dejó de responder (device hung).\nSe cerrará la aplicación."
					: L"Se perdió el dispositivo de video (driver reiniciado).\nSe cerrará la aplicación.",
				L"Error de Direct3D", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
		}
	}

};

#endif