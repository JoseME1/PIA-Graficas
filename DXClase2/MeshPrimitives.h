#ifndef _MeshPrimitives_
#define _MeshPrimitives_

class MPrimitives {
private:
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;
	ID3D11Buffer* cbBuffer;
	ID3D11Buffer* cbBufferPS;

	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;

	ID3D11ShaderResourceView* Texture;
	ID3D11ShaderResourceView* Texture2;
	ID3D11ShaderResourceView* NormalTexture;
	ID3D11SamplerState* TexSamplerState;

	//Bitmap
	ID3D11Texture2D* renderTargetTextureGUI;

	D3DXMATRIX mapView;
	D3DXMATRIX mapProjection;

	UINT numVertex;
	UINT numIndex;

	D3DXMATRIX World;

	float Size;

	int Stacks;
	int Slices;
	int tWidth, tHeight;
	float Radius;
	float radius;
	float Height;
	float Width;



	int CantOfSections;
	int TEXTURE_REPEAT;
	bool faceToCam;

	//The constant buffer Structure
	struct ConstantBuffer
	{
		D3DXMATRIX WorldViewProjection;
		D3DXMATRIX World;
		D3DXVECTOR3 CameraPos;
		float MaxH;
	} consBuff;

	struct ConstantBufferPS
	{
		Light  light;
	} consBuffPS;

	//The vertex Structure
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 tex;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 biTangent;
	} ;

	SimpleVertex* heightMapDummy;

	

public:

	struct SphereBoundingInfo{
		D3DXVECTOR3 PositionM;
		float Radius;
	};

	SphereBoundingInfo* PrimitiveBoundings;

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Size, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Size = Size;
		light.specularPower = 2;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createCube();
		
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Stacks, int Slices, float Radius, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		PrimitiveBoundings = new SphereBoundingInfo();
		PrimitiveBoundings->PositionM = D3DXVECTOR3(0, 0, 0);
		PrimitiveBoundings->Radius = Radius;
		this->Stacks = Stacks;
		this->Slices = Slices;
		this->Radius = Radius;

		int numVertexInTwoTriangles = 6;
		numVertex = Slices * Stacks;
		numIndex = Slices * Stacks * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		
		createSphere();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Stacks, int Slices, float Radius, float radius, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Stacks = Stacks;
		this->Slices = Slices;
		this->Radius = Radius;
		this->radius = radius;

		int numVertexInTwoTriangles = 6;
		numVertex = Slices * Stacks;
		numIndex = Slices * Stacks * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createTorus();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Slices, float Radius, float radius, float Height, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		this->Slices = Slices;
		this->Radius = Radius;
		this->radius = radius;
		this->Height = Height;

		int numVertexInTwoTriangles = 6;
		CantOfSections = 4;
		numVertex = Slices * CantOfSections;
		numIndex = Slices * CantOfSections * numVertexInTwoTriangles;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		createCilinder();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int tWidth, LPCWSTR texturePath, LPCWSTR textureNormalPath, char* heightMapFilename, int ShaderType)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		this->Size = Size;
		TEXTURE_REPEAT = 8;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		LoadHeightMap(heightMapFilename);
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, char* modelObjectPath, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		InitScene(d3d11Device, d3d11DevCon, texturePath, textureNormalPath, ShaderType);
		LoadModel(modelObjectPath);
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Height, float Width, LPCWSTR texturePath)
	{
		this->Height = Height;
		this->Width = Width;
		faceToCam = true;

		InitScene(d3d11Device, d3d11DevCon, texturePath, L"", ShaderDiffuse);
		createBillboard();
	}

	MPrimitives(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Height, float Width, LPCWSTR texturePath, Camara* camara){


		
		this->Height = Height;
		this->Width = Width;

		InitScene(d3d11Device, d3d11DevCon, texturePath, L"", ShaderGUI);
		createBitMap(camara);
	}

	bool InitScene(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, LPCWSTR texturePath, LPCWSTR textureNormalPath, int ShaderType)
	{
		//Inicializacion de variables
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		//The input-layout description
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);
		LPCWSTR pixelShaderFile = L"pixelShader.ps";

		switch (ShaderType)
		{
		case ShaderSpecular:
			pixelShaderFile = L"pixelShaderSpecular.ps";
			break;
		case ShaderNormalmap:
			pixelShaderFile = L"pixelShaderNormalMap.ps";
			break;
		case ShaderGUI:
			pixelShaderFile = L"pixelShaderGUI.ps";
			break;
		case ShaderMultitextura:
			pixelShaderFile = L"pixelShaderMultitextura.ps";
			break;
		default:
			break;
		}

		

		//Compile Shaders from shader file
		D3DX11CompileFromFile(L"vertexShader.vs", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
		D3DX11CompileFromFile(pixelShaderFile, 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

		//Create the Shader Objects
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),
			PS_Buffer->GetBufferSize(), NULL, &PS);

		//Create the Input Layout
		d3d11Device->CreateInputLayout(layout, numElements,
			VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Set the Input Layout
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Set Primitive Topology
		d3d11DevCon->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



		//Create the buffer to send to the cbuffer in effect file
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(ConstantBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbd, NULL,
			&cbBuffer);

		// Set the texture
		D3DX11CreateShaderResourceViewFromFile(d3d11Device, texturePath,
			NULL, NULL, &Texture, NULL);

		// Set the normal texture
		D3DX11CreateShaderResourceViewFromFile(d3d11Device, textureNormalPath,
			NULL, NULL, &NormalTexture, NULL);

		D3DX11CreateShaderResourceViewFromFile(d3d11Device, L"Texturas/rock.jpg",
			NULL, NULL, &Texture2, NULL);

		// Describe the Sample State
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		//Create the Sample State
		d3d11Device->CreateSamplerState(&sampDesc, &TexSamplerState);

		//Create the pixel constant buffer
		D3D11_BUFFER_DESC cbbdps;
		ZeroMemory(&cbbdps, sizeof(D3D11_BUFFER_DESC));

		cbbdps.Usage = D3D11_USAGE_DEFAULT;
		cbbdps.ByteWidth = sizeof(ConstantBufferPS);
		cbbdps.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbdps.CPUAccessFlags = 0;
		cbbdps.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbdps, NULL,
			&cbBufferPS);

		return true;
	}

	void createBillboard()
	{
		Height = Height / 2.0f;
		Width = Width / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			{ D3DXVECTOR3(-Width, -Height, 0), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-Width, Height, 0), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(Width, Height, 0), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(Width, -Height, 0), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		numIndex = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createCube()
	{
		float cubeSize = Size / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			// Front Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Back Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },

			// Top Face
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Bottom Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },

			// Left Face
			{ D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f) },

			// Right Face
			{ D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			// Front Face
			0, 1, 2,
			0, 2, 3,

			// Back Face
			4, 5, 6,
			4, 6, 7,

			// Top Face
			8, 9, 10,
			8, 10, 11,

			// Bottom Face
			12, 13, 14,
			12, 14, 15,

			// Left Face
			16, 17, 18,
			16, 18, 19,

			// Right Face
			20, 21, 22,
			20, 22, 23
		};
		numIndex = ARRAYSIZE(indices);
		setTangentBinormal(vertex, indices, 3);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createSphere()
	{
		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		const float thetax = D3DX_PI;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < Stacks; i++)
		{
			float y = (float)i / (Stacks - 1);
			for (int j = 0; j < Slices; j++)
			{
				float x = (float)j / (Slices - 1);

				vertex[index].pos.x = Radius * sin(y*thetax) * cos(x*phix);
				vertex[index].pos.y = Radius * sin(y*thetax) * sin(x*phix);
				vertex[index].pos.z = Radius * cos(y*thetax);

				vertex[index].tex.x = x;
				vertex[index].tex.y = y;

				vertex[index].normal = vertex[index].pos;

				index++;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;

		for (int i = 0; i < Stacks - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1 = (Slices *   i) + j;
				int index2 = (Slices *   i) + (j + 1);
				int index3 = (Slices * (i + 1)) + j;
				int index4 = (Slices * (i + 1)) + (j + 1);

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index4;
				indices[index++] = index2;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &IndexBuffer);
	}

	void createTorus()
	{
		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		const float thetax = D3DX_PI;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < Stacks; i++)
		{
			float y = (float)i / (Stacks - 1);
			for (int j = 0; j < Slices; j++)
			{
				float x = (float)j / (Slices - 1);

				vertex[index].pos.x = (Radius + radius * cos(x*phix)) * cos(y*phix);
				vertex[index].pos.y = (Radius + radius * cos(x*phix)) * sin(y*phix);
				vertex[index].pos.z = radius * sin(x*phix);

				vertex[index].tex.x = x;
				vertex[index].tex.y = y;

				vertex[index].normal = vertex[index].pos;

				index++;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;

		for (int i = 0; i < Stacks - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1 = (Slices *   i) + j;
				int index2 = (Slices *   i) + (j + 1);
				int index3 = (Slices * (i + 1)) + j;
				int index4 = (Slices * (i + 1)) + (j + 1);

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index4;
				indices[index++] = index2;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &IndexBuffer);
	}

	void createCilinder()
	{
		//Create the vertex buffer
		int index = 0;
		const float fullCircle = D3DX_PI * 2.0;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for (int i = 0; i < CantOfSections; i++)
		{
			for (int j = 0; j < Slices; j++)
			{
				float segment = (float)j / (Slices - 1);

				switch (i)
				{
				case 0:
					vertex[index].pos.x = radius * cos(segment*fullCircle);
					vertex[index].pos.y = -Height / 2;
					vertex[index].pos.z = radius * sin(segment*fullCircle);

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 1;
					break;
				case 1:
					vertex[index].pos.x = Radius * cos(segment*fullCircle);
					vertex[index].pos.y = Height / 2;
					vertex[index].pos.z = Radius * sin(segment*fullCircle);

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 0;
					break;
				case 2:
					vertex[index].pos.x = 0;
					vertex[index].pos.y = -Height / 2;
					vertex[index].pos.z = 0;

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 0;
					break;
				case 3:
					vertex[index].pos.x = 0;
					vertex[index].pos.y = Height / 2;
					vertex[index].pos.z = 0;

					vertex[index].tex.x = segment;
					vertex[index].tex.y = 1;
					break;
				}
				vertex[index].normal = vertex[index].pos;
				index++;
			}
		}


		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD *indices = new DWORD[numIndex];
		index = 0;
		int indexBottom = Slices;

		for (int i = 0; i < CantOfSections - 1; i++)
		{
			for (int j = 0; j < Slices - 1; j++)
			{
				int index1, index2, index3, index4;
				int indexPoint = Slices * (i + 1);

				switch (i)
				{
				case 0:
					index1 = j;
					index2 = j + indexBottom;
					index3 = j + indexBottom + 1;
					index4 = j + 1;
					break;
				case 1:
					index1 = j + 1;
					index2 = j + indexPoint;
					index3 = j + indexPoint + 1;
					index4 = j;

					break;
				case 2:
					index1 = j + indexBottom;
					index2 = j + indexPoint;
					index3 = j + indexPoint + 1;
					index4 = j + indexBottom + 1;

					break;
				}

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index2;
				indices[index++] = index3;
			}
		}
		setTangentBinormal(vertex, indices, 6);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	void createBitMap(Camara *camara){

		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Textura del bitmap para GUI 

		// Inicialisamos la descripcion de la textura.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Llenamos la descripcion de la textura
		// Vamos a hacer que nuestra textura sea un cuadrado

		textureDesc.Width = Width / 2;
		textureDesc.Height = Height / 2;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// creamos la textura con el device
		d3d11Device->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureGUI);

		
		Height = Height / 2.0f;
		Width = Width / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			{ D3DXVECTOR3(-Width, -Height, 0), D3DXVECTOR2(0.0f, 1.0f) },
			{ D3DXVECTOR3(-Width, Height, 0), D3DXVECTOR2(0.0f, 0.0f) },
			{ D3DXVECTOR3(Width, Height, 0), D3DXVECTOR2(1.0f, 0.0f) },
			{ D3DXVECTOR3(Width, -Height, 0), D3DXVECTOR2(1.0f, 1.0f) },
		};
		numVertex = ARRAYSIZE(vertex);

		for (int i = 0; i < numVertex; i++)
			vertex[i].normal = vertex[i].pos;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
			&VertexBuffer);

		DWORD indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		numIndex = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc,
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc,
			&iinitData,
			&IndexBuffer);
	}

	

	//Funcion para leer el heightmap
	void LoadHeightMap(char* filename)
	{
		FILE* filePtr;
		int error;
		unsigned int count;
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		int imageSize, i, j, k, index;
		unsigned char* bitmapImage;
		unsigned char height;

		//  Abrimos el heightmap in binario.
		error = fopen_s(&filePtr, filename, "rb");
		if (error != 0)
		{
			//Error
		}

		// Leemos la cabecera del archivo.
		count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
		if (count != 1)
		{
			//Error
		}

		// Leemos la cabecera de info del archivo.
		count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
		if (count != 1)
		{
			//Error
		}

		// Guardamos el ancho y largo del terreno.
		tWidth = bitmapInfoHeader.biWidth;
		tHeight = bitmapInfoHeader.biHeight;

		//Calculamos el tamaño.
		imageSize = tWidth * tHeight * 3;

		//  Apartamos memoria para usar el bitmap.
		bitmapImage = new unsigned char[imageSize];
		if (!bitmapImage)
		{
			//return false;
		}

		// Nos movemos al principio de nuestro bitmap.
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

		// Leemos la info del bitmap
		count = fread(bitmapImage, 1, imageSize, filePtr);
		if (count != imageSize)
		{
			//return false;
		}

		// Cerramos el archivo.
		error = fclose(filePtr);
		if (error != 0)
		{
			//return false;
		}

		//  Creamos la estructura para guardar la info del heightmap.
		SimpleVertex *heightMap = new SimpleVertex[tWidth * tHeight];
		int numVertexInTwoTriangles = 6;
		numVertex = (tWidth - 1) * (tHeight - 1) * numVertexInTwoTriangles;
		numIndex = numVertex;
		if (!heightMap)
		{
			//return false;
		}

		
		k = 0;

		float max = 0;

		// Leemos la imagen para sacar la altura.
		for (j = 0; j<tHeight; j++)
		{
			for (i = 0; i<tWidth; i++)
			{
				height = bitmapImage[k];

				index = (tHeight * j) + i;

				heightMap[index].pos.x = (float)i;
				heightMap[index].pos.y = (float)(height /= 15.0f);
				heightMap[index].pos.z = (float)j;

				if (max < heightMap[index].pos.y)
					max = heightMap[index].pos.y;

				k += 3;
			}
		}

		consBuff.MaxH = max;
		// Liberamos nuestro bitmap.
		delete[] bitmapImage;
		bitmapImage = 0;

		int incrementCount, tuCount, tvCount;
		float incrementValue, tuCoordinate, tvCoordinate;


		// Calculamos que tanto vamos a incrementar las uvs.
		incrementValue = (float)TEXTURE_REPEAT / (float)tWidth;

		// Calculamos cuantas veces vamos a repetir la textura.
		incrementCount = tWidth / TEXTURE_REPEAT;

		// Indices para las tu y tv.
		tuCoordinate = 0.0f;
		tvCoordinate = 1.0f;

		
		tuCount = 0;
		tvCount = 0;

		// Loop para calcular las tu y tv de todo el heightmap.
		for (j = 0; j<tHeight; j++)
		{
			for (i = 0; i<tWidth; i++)
			{
				//  Guardamos las coordenadas de textura en el terreno.
				heightMap[(tHeight * j) + i].tex.x = tuCoordinate;
				heightMap[(tHeight * j) + i].tex.y = tvCoordinate;

				//  incrementamos el valor de la tu y el indice tu por 1.
				tuCoordinate += incrementValue;
				tuCount++;

				// Verificamos que no estamos al limite de la derecha en la textura y si es asi, regresamos.
				if (tuCount == incrementCount)
				{
					tuCoordinate = 0.0f;
					tuCount = 0;
				}
			}

			//  incrementamos el valor de las tv y el indice tu por 1.
			tvCoordinate -= incrementValue;
			tvCount++;

			//  Verificamos que no estamos al limite de arriba en la textura y si es asi, regresamos al fondo.
			if (tvCount == incrementCount)
			{
				tvCoordinate = 1.0f;
				tvCount = 0;
			}
		}

		createTerrain(heightMap);
	}

	void createTerrain(SimpleVertex heightMap[]) 
	{
		int index, i, j;
		int index1, index2, index3, index4;
		float tu, tv;
		index = 0;

		//Creacion de vertices
		SimpleVertex *vertex = new SimpleVertex[numVertex];
		heightMapDummy = new SimpleVertex[numVertex];
		unsigned long* indices = new unsigned long[numVertex];
		for (j = 0; j<(tHeight - 1); j++)
		{
			for (i = 0; i<(tWidth - 1); i++)
			{
				index1 = (tHeight * j) + i;          // Bottom left.
				index2 = (tHeight * j) + (i + 1);      // Bottom right.
				index3 = (tHeight * (j + 1)) + i;      // Upper left.
				index4 = (tHeight * (j + 1)) + (i + 1);  // Upper right.

				// Upper left.
				tv = heightMap[index3].tex.y;

				//  Modificamos las coordenadas de textura para cubrir la orilla de arriba.
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(heightMap[index3].pos.x, heightMap[index3].pos.y, heightMap[index3].pos.z);
				vertex[index].tex = D3DXVECTOR2(heightMap[index3].tex.x, tv);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;

				// Upper right.
				tu = heightMap[index4].tex.x;
				tv = heightMap[index4].tex.y;

				// Modificamos las coordenadas de textura para cubrir la orilla de arriba y de la derecha.
				if (tu == 0.0f) { tu = 1.0f; }
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(heightMap[index4].pos.x, heightMap[index4].pos.y, heightMap[index4].pos.z);
				vertex[index].tex = D3DXVECTOR2(tu, tv);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;

				// Bottom left.
				vertex[index].pos = D3DXVECTOR3(heightMap[index1].pos.x, heightMap[index1].pos.y, heightMap[index1].pos.z);
				vertex[index].tex = D3DXVECTOR2(heightMap[index1].tex.x, heightMap[index1].tex.y);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;

				// Bottom left.
				vertex[index].pos = D3DXVECTOR3(heightMap[index1].pos.x, heightMap[index1].pos.y, heightMap[index1].pos.z);
				vertex[index].tex = D3DXVECTOR2(heightMap[index1].tex.x, heightMap[index1].tex.y);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;

				// Upper right.
				tu = heightMap[index4].tex.x;
				tv = heightMap[index4].tex.y;

				// Modificamos las coordenadas de textura para cubrir la orilla de arriba y la derecha.
				if (tu == 0.0f) { tu = 1.0f; }
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(heightMap[index4].pos.x, heightMap[index4].pos.y, heightMap[index4].pos.z);
				vertex[index].tex = D3DXVECTOR2(tu, tv);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;

				// Bottom right.
				tu = heightMap[index2].tex.x;

				// Modificamos las coordenadas de textura para cubrir la orilla de la derecha.
				if (tu == 0.0f) { tu = 1.0f; }

				vertex[index].pos = D3DXVECTOR3(heightMap[index2].pos.x, heightMap[index2].pos.y, heightMap[index2].pos.z);
				vertex[index].tex = D3DXVECTOR2(tu, heightMap[index2].tex.y);
				vertex[index].normal = vertex[index].pos;
				indices[index] = index;
				index++;
			}
		}

		heightMapDummy = vertex;

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)*numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, 
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, 
			&iinitData, 
			&IndexBuffer);
	}

	//Para cargar el obj
	bool LoadModel(char* filename)
	{

		ifstream fin;
		char input;
		int i;

		//Abrimos el archivo del modelo
		fin.open(filename);

		//Si no se pudo abrir el modelo regresa falso
		if (fin.fail())
			return false;

		//Leemos hasta los ":" para leer la cantidada de vertices
		fin.get(input);
		while (input != ':')
			fin.get(input);

		//Leemos la cantidad de vertices
		fin >> numVertex;

		//Ponemos que la cantidad de indices sea la misma que la cantidad de vertices
		numIndex = numVertex;

		//creamos la estructura para guardar los datos del modelo utilizando como referencia
		//para la cantidad de espacio que se ocupara la cantidad de vertices
		SimpleVertex *ModelObject = new SimpleVertex[numVertex];

		//Leemos desde el principio de la informacion
		fin.get(input);
		while (input != ':')
			fin.get(input);
		fin.get(input);
		fin.get(input);

		//Leemos la informacion de cada vertice
		for (i = 0; i<numVertex; i++){
			fin >> ModelObject[i].pos.x >> ModelObject[i].pos.y >> ModelObject[i].pos.z;
			fin >> ModelObject[i].tex.x >> ModelObject[i].tex.y;
			fin >> ModelObject[i].normal.x >> ModelObject[i].normal.y >> ModelObject[i].normal.z;
		}

		//Cerramos el archivo
		fin.close();

		createModelObject(ModelObject);

		return true;
	}

	void createModelObject(SimpleVertex *ModelObject)
	{
		//estas son las coordenadas de los vertices
		SimpleVertex* vertex = new SimpleVertex[numVertex];
		unsigned long* indices = new unsigned long[numVertex];
		for (int i = 0; i<numVertex; i++){

			vertex[i].pos = D3DXVECTOR3(ModelObject[i].pos.x, ModelObject[i].pos.y, ModelObject[i].pos.z);
			vertex[i].tex = D3DXVECTOR2(ModelObject[i].tex.x, ModelObject[i].tex.y);
			vertex[i].normal = D3DXVECTOR3(ModelObject[i].normal.x, ModelObject[i].normal.y, ModelObject[i].normal.z);

			indices[i] = i;
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)*numVertex;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);

		//los indices
		int numTriangles = numIndex / 3;
		for(int i = 0; i < numTriangles; ++i)
		{
			UINT i0 = indices[i*3+0];
			UINT i1 = indices[i*3+1];
			UINT i2 = indices[i*3+2];

			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i0].tangent, vertex[i0].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i1].tangent, vertex[i1].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i2].tangent, vertex[i2].biTangent);
		}

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, 
			sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD)* numIndex;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, 
			&iinitData, 
			&IndexBuffer);
	}

	void setTangentBinormal(SimpleVertex *vertex, DWORD *indices, int base) 
	{
		int numTriangles = numIndex / base;
		for(int i = 0; i < numTriangles; ++i)
		{
			UINT i0 = indices[i*3+0];
			UINT i1 = indices[i*3+1];
			UINT i2 = indices[i*3+2];

			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i0].tangent, vertex[i0].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i1].tangent, vertex[i1].biTangent);
			CalculateTangentBinormal(vertex[i0], vertex[i1], vertex[i2], vertex[i2].tangent, vertex[i2].biTangent);
		}
	}

	void CalculateTangentBinormal(SimpleVertex vertex1, SimpleVertex vertex2, SimpleVertex vertex3,
										  D3DXVECTOR3& tangent, D3DXVECTOR3& binormal)
	{

		D3DXVECTOR3 vector1, vector2;
		float tuVector[2], tvVector[2];
		float den;
		float length;


		// Calculate the two vectors for this face.
		vector1 = vertex2.pos - vertex1.pos;
		vector2 = vertex3.pos - vertex1.pos;

		// Calculate the tu and tv texture space vectors.
		tuVector[0] = vertex2.tex.x - vertex1.tex.x;
		tvVector[0] = vertex2.tex.y - vertex1.tex.y;

		tuVector[1] = vertex3.tex.x - vertex1.tex.x;
		tvVector[1] = vertex3.tex.y - vertex1.tex.y;

		// Calculate the denominator of the tangent/binormal equation.
		den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
		tangent = (tvVector[1] * vector1 - tvVector[0] * vector2) * den;
		binormal = (tuVector[0] * vector2 - tuVector[1] * vector1) * den;

		// Calculate the length of this normal.
		length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

		// Normalize the normal and then store it
		tangent /= length;

		// Calculate the length of this normal.
		length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

		// Normalize the normal and then store it
		binormal /= length;

		return;
	}

	void UpdateScene(D3DXMATRIX World)
	{
		this->World = World;
	}

	void DrawScene(Camara *camara)
	{
		if (faceToCam)
		{
			D3DXMATRIX rotMat;
			D3DXMatrixRotationY(&rotMat, camara->Yaw);
			World = rotMat * World;
		}
		D3DXMATRIX WorldViewProjection = World * camara->View * camara->Projection;
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &World);
		consBuff.CameraPos = camara->Position;
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		//Set the vertex and the index buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		d3d11DevCon->PSSetShaderResources(0, 1, &Texture);
		d3d11DevCon->PSSetShaderResources(1, 1, &NormalTexture );

		d3d11DevCon->PSSetShaderResources(2, 1, &Texture2);
		d3d11DevCon->PSSetSamplers(0, 1, &TexSamplerState);

		consBuffPS.light = light;
		d3d11DevCon->UpdateSubresource(cbBufferPS, 0, NULL, &consBuffPS, 0, 0);
		d3d11DevCon->PSSetConstantBuffers(0, 1, &cbBufferPS);

		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}

	void DrawBitmap(Camara *camara){
		
		// Dibujar la GUI
	
		/*Aqui se dibuja el bitmap, solo necesitamos un cuadrado donde poner la textura, por eso utilizamos los
		vertices de el billboard, lo estaremos dibujando directamente sobre la pantalla, recuerden que las 
		coordenadas 0,0 son el centro de la pantalla, los lados van de -1 a 1*/
		
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		//Cargamos el buffer de indices y vertices
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Seteamos la matriz WVP a una escala y traslacion dada apara acomodarla en pantalla

		D3DXMATRIX ScaleM, TranslationM;
		D3DXMatrixScaling(&ScaleM, .5, .5, 0);
		D3DXMatrixTranslation(&TranslationM, .5, -.5, .8);

		D3DXMATRIX WorldViewProjection = ScaleM * TranslationM;
		
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &World);
		consBuff.CameraPos = camara->Position;
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		d3d11DevCon->PSSetShaderResources(0, 1, &Texture);
		d3d11DevCon->PSSetShaderResources(1, 1, &NormalTexture);
		d3d11DevCon->PSSetSamplers(0, 1, &TexSamplerState);

		//Dibujamos el bitmap
		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}

	float getTerrainCollision(D3DXVECTOR2 XZ){

		if (heightMapDummy != NULL){

			for (int x = 0; x < numVertex; x++){

				if ((int)heightMapDummy[x].pos.x == (int)XZ.x && (int)heightMapDummy[x].pos.z == (int)XZ.y)
					return heightMapDummy[x].pos.y;
			}
		}
		return 0;
	}

	D3DXMATRIX getWorldMatrix(){
		return this->World;
	}

};

#endif