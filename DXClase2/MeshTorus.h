#ifndef _MeshTorus_
#define _MeshTorus_

class MTorus {
private:
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;
	ID3D11Buffer* cbBuffer;

	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;

	ID3D11ShaderResourceView* Texture;
	ID3D11SamplerState* TexSamplerState;

	UINT numVertex;
	UINT numIndex;

	D3DXMATRIX World;

	int Stacks;
	int Slices;
	float Radius;
	float radius;
	LPCWSTR texturePath;

	//The constant buffer Structure
	struct ConstantBuffer
	{
		D3DXMATRIX WorldViewProjection;
	} consBuff;

	//The vertex Structure
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 tex;
	};

public:

	MTorus(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Stacks, int Slices, float Radius, float radius, LPCWSTR texturePath)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		this->Stacks = Stacks;
		this->Slices = Slices;
		this->Radius = Radius;
		this->radius = radius;
		this->texturePath = texturePath;

		int numVertexInTwoTriangles = 6;
		numVertex = Slices * Stacks;
		numIndex = Slices * Stacks * numVertexInTwoTriangles;

		InitScene();
	}

	bool InitScene()
	{
		//The input-layout description
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
			D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);

		//Compile Shaders from shader file
		D3DX11CompileFromFile(L"vertexShader.vs", 0, 0, "VS",
			"vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
		D3DX11CompileFromFile(L"pixelShader.ps", 0, 0, "PS",
			"ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

		//Create the Shader Objects
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),
			PS_Buffer->GetBufferSize(), NULL, &PS);

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		SimpleVertex *vertex = new SimpleVertex[numVertex];

		for(int i = 0; i < Stacks; i++)
		{
			float y = (float) i/(Stacks-1);
			for(int j = 0; j < Slices; j++)
			{
				float x = (float) j/(Slices-1);

				vertex[index].pos.x   = (Radius + radius * cos(x*phix)) * cos(y*phix);
				vertex[index].pos.y   = (Radius + radius * cos(x*phix)) * sin(y*phix);
				vertex[index].pos.z   = radius * sin(x*phix);

				vertex[index].tex.x = x;
				vertex[index].tex.y = y;

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

		//Create the Input Layout
		d3d11Device->CreateInputLayout(layout, numElements,
			VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Set the Input Layout
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Set Primitive Topology
		d3d11DevCon->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DWORD *indices = new DWORD[numIndex];
		index = 0;

		for(int i=0; i < Stacks - 1; i++)
		{
			for(int j=0; j < Slices - 1; j++)
			{
				int index1 = (Slices *   i)   +   j; 
				int index2 = (Slices *   i)   + (j+1);
				int index3 = (Slices * (i+1)) +   j;
				int index4 = (Slices * (i+1)) + (j+1);

				indices[index++] = index1; //314
				indices[index++] = index3;
				indices[index++] = index4;

				indices[index++] = index1; //124
				indices[index++] = index4;
				indices[index++] = index2;
			}
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
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);

		//Set the vertex and the index buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		d3d11DevCon->IASetIndexBuffer(IndexBuffer,DXGI_FORMAT_R32_UINT,0);

		d3d11DevCon->PSSetShaderResources( 0, 1, &Texture );
		d3d11DevCon->PSSetSamplers( 0, 1, &TexSamplerState );

		d3d11DevCon->DrawIndexed(numIndex, 0, 0);
	}

};

#endif