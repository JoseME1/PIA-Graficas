#ifndef _MeshCube_
#define _MeshCube_

class MCube {
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
	ID3D11SamplerState* TexSamplerState;

	UINT numVertex;
	UINT numIndex;

	D3DXMATRIX World;

	float Size;
	LPCWSTR texturePath;

	struct Light
	{

		D3DXVECTOR3 direction;
		float padding;
		D3DXVECTOR4 ambient;
		D3DXVECTOR4 diffuse;
	} light; 

	//The constant buffer Structure
	struct ConstantBuffer
	{
		D3DXMATRIX WorldViewProjection;
		D3DXMATRIX World;
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
	};

public:

	MCube(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, float Size, LPCWSTR texturePath)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		this->Size = Size;
		this->texturePath = texturePath;

		light.direction = D3DXVECTOR3(5.0f, 0.0f, 0.0f);
		light.ambient = D3DXVECTOR4(0.5f, 0.5f, 0.5f, 1.0f);
		light.diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

		InitScene();
	}

	bool InitScene()
	{
		//The input-layout description
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		UINT numElements = ARRAYSIZE(layout);

		//Compile Shaders from shader file
		D3DX11CompileFromFile(L"vertexShader.vs", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
		D3DX11CompileFromFile(L"pixelShader.ps", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

		//Create the Shader Objects
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),
			PS_Buffer->GetBufferSize(), NULL, &PS);

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		float cubeSize = Size / 2.0f;
		//Create the vertex buffer
		SimpleVertex vertex[] =
		{
			// Front Face
			{D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3(-cubeSize,  cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3(cubeSize,  cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f)},
			{D3DXVECTOR3(cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f)},

			// Back Face
			{D3DXVECTOR3(-cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(1.0f, 1.0f)},
			{D3DXVECTOR3( cubeSize, -cubeSize, cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3( cubeSize,  cubeSize, cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3(-cubeSize,  cubeSize, cubeSize), D3DXVECTOR2(1.0f, 0.0f)},

			// Top Face
			{D3DXVECTOR3(-cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3(-cubeSize, cubeSize,  cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3( cubeSize, cubeSize,  cubeSize), D3DXVECTOR2(1.0f, 0.0f)},
			{D3DXVECTOR3( cubeSize, cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f)},

			// Bottom Face
			{D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f)},
			{D3DXVECTOR3( cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3( cubeSize, -cubeSize,  cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3(-cubeSize, -cubeSize,  cubeSize), D3DXVECTOR2(1.0f, 0.0f)},

			// Left Face
			{D3DXVECTOR3(-cubeSize, -cubeSize,  cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3(-cubeSize,  cubeSize,  cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3(-cubeSize,  cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 0.0f)},
			{D3DXVECTOR3(-cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(1.0f, 1.0f)},

			// Right Face
			{D3DXVECTOR3( cubeSize, -cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 1.0f)},
			{D3DXVECTOR3( cubeSize,  cubeSize, -cubeSize), D3DXVECTOR2(0.0f, 0.0f)},
			{D3DXVECTOR3( cubeSize,  cubeSize,  cubeSize), D3DXVECTOR2(1.0f, 0.0f)},
			{D3DXVECTOR3( cubeSize, -cubeSize,  cubeSize), D3DXVECTOR2(1.0f, 1.0f)},
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

		//Create the Input Layout
		d3d11Device->CreateInputLayout(layout, numElements,
			VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Set the Input Layout
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Set Primitive Topology
		d3d11DevCon->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DWORD indices[] = {
			// Front Face
			0,  1,  2,
			0,  2,  3,

			// Back Face
			4,  5,  6,
			4,  6,  7,

			// Top Face
			8,  9, 10,
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
		D3DX11CreateShaderResourceViewFromFile( d3d11Device, texturePath,
			NULL, NULL, &Texture, NULL );

		// Describe the Sample State
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
		//Create the Sample State
		d3d11Device->CreateSamplerState( &sampDesc, &TexSamplerState );

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

	void UpdateScene(D3DXMATRIX World)
	{
		this->World = World;
	}

	void DrawScene(D3DXMATRIX View, D3DXMATRIX Projection)
	{
		D3DXMATRIX WorldViewProjection = World * View * Projection;
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &World);
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		//Set the vertex and the index buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer,DXGI_FORMAT_R32_UINT,0);

		d3d11DevCon->PSSetShaderResources( 0, 1, &Texture );
		d3d11DevCon->PSSetSamplers( 0, 1, &TexSamplerState );

		consBuffPS.light = light;
		d3d11DevCon->UpdateSubresource(cbBufferPS, 0, NULL, &consBuffPS, 0, 0);
		d3d11DevCon->PSSetConstantBuffers(0, 1, &cbBufferPS);

		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}
};

#endif