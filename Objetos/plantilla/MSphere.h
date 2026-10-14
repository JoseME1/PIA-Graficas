#ifndef _MSphere_
#define _MSphere_

class MSphere
{
public:
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
	ID3D11SamplerState* TextSamplerState;
	LPCWSTR text;


	D3DXMATRIX World;

	int Slices;
	int Stacks;
	float Radius;

	int numVertInTwoTriangleFaces;

	//The constant buffer Structure
	struct ConstantBuffer
	{
		D3DXMATRIX WorldViewProjection;
		D3DXMATRIX World;
		D3DXVECTOR3 CameraPos;
		float padding;
	} consBuff;

	struct ConstantBufferPS
	{
		Light  light;
	} consBuffPS;

	//The vertex Structure
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 text;
		D3DXVECTOR3 normal;
	};

	MSphere(ID3D11Device *d3d11Device, ID3D11DeviceContext* d3d11DevCon, int Slices, int Stacks, float Radius, LPCWSTR text, HWND hwnd)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		this->Slices = Slices;
		this->Stacks = Stacks;
		this->Radius = Radius;

		numVertInTwoTriangleFaces = 6;

		this->text = text;


		InitScene(hwnd);
	}

	bool InitScene(HWND hwnd)
	{
		//The input-layout description
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);

		//Compile Shaders from shader file
		HRESULT result;
		ID3D10Blob* errorMessage;
		errorMessage = 0;

		result = D3DX11CompileFromFile(L"vertex.vs", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, &errorMessage, 0);
		if (FAILED(result))
		{
			//Si el shader no se compilo correctamente se escribe el error en el txt
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, L"Vector Shader");
			}
			// Si no habia nada en el mensaje de error simplemente se imprime que no se encontro el archivo
			else
			{
				MessageBox(hwnd, L"Vector Shader", L"No se encontro el archivo", MB_OK);
			}

			return false;
		}
		result = D3DX11CompileFromFile(L"pixel.ps", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, &errorMessage, 0);
		if (FAILED(result))
		{
			//Si el shader no se compilo correctamente se escribe el error en el txt
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, L"Pixel Shader");
			}
			// Si no habia nada en el mensaje de error simplemente se imprime que no se encontro el archivo
			else
			{
				MessageBox(hwnd, L"Pixel Shader", L"No se encontro el archivo", MB_OK);
			}

			return false;
		}

		//Create the Shader Objects
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		//Create the vertex buffer
		int index = 0;
		const float phix = D3DX_PI * 2.0;
		const float thetax = D3DX_PI;

		
		SimpleVertex *vertex = new SimpleVertex[Slices * Stacks];

		for(int i = 0; i < Stacks; i++)
		{
			float y = (float) i/(Stacks-1);
			for(int j = 0; j < Slices; j++)
			{
				float x = (float) j/(Slices-1);

				vertex[index].pos.x   = Radius * sin(y*thetax) * cos(x*phix);
				vertex[index].pos.y   = Radius * sin(y*thetax) * sin(x*phix);
				vertex[index].pos.z   = Radius * cos(y*thetax);

				vertex[index].text.x = x;
				vertex[index].text.y = y;

				vertex[index].normal = vertex[index].pos;

				index++;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)* (Slices * Stacks);
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);

		//Create the Input Layout
		d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Set the Input Layout
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Set Primitive Topology
		d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DWORD *indices = new DWORD[Slices * Stacks * numVertInTwoTriangleFaces];
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
		indexBufferDesc.ByteWidth = sizeof(DWORD)* (Slices * Stacks) * numVertInTwoTriangleFaces;
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

		//Create resource from file 'text'

		D3DX11CreateShaderResourceViewFromFile(d3d11Device, text, NULL, NULL, &Texture, NULL);

		//DESCRIBE THE SAMPLE STATE
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		//create the sample state
		d3d11Device->CreateSamplerState(&sampDesc, &TextSamplerState);

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

	void DrawScene(Camara *camara)
	{
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		//Set the spheres index buffer
		d3d11DevCon->IASetIndexBuffer( IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the spheres vertex buffer
		d3d11DevCon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );

		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		consBuff.WorldViewProjection = World * camara->View * camara->Projection;
		consBuff.World = World;
		//consBuff.CameraPos = camara->Position;
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &consBuff.WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &consBuff.World);
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);
		
		d3d11DevCon->PSSetShaderResources(0, 1, &Texture);
		d3d11DevCon->PSSetSamplers(0, 1, &TextSamplerState);
		
		consBuffPS.light = light;
		d3d11DevCon->UpdateSubresource(cbBufferPS, 0, NULL, &consBuffPS, 0, 0);
		d3d11DevCon->PSSetConstantBuffers(0, 1, &cbBufferPS);

		d3d11DevCon->DrawIndexed((Slices * Stacks * numVertInTwoTriangleFaces), 0, 0);
	}

	//Funcion para escribir errores de compilacion de shaders en un txt dentro de la carpeta de proyecto
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
	{
		char* compileErrors;
		unsigned long bufferSize, i;
		ofstream fout;


		// Obtenemos un puntero al buffer de texto de errormessage.
		compileErrors = (char*)(errorMessage->GetBufferPointer());

		// Obtenemos el tamaño de el buffer.
		bufferSize = errorMessage->GetBufferSize();

		// Abrimos el archivo en el que vamos a escribir.
		fout.open("shader-error.txt");

		// Escribimos el mensaje.
		for (i = 0; i<bufferSize; i++)
		{
			fout << compileErrors[i];
		}

		// Cerramos el archivo.
		fout.close();

		// Se libera el buffer errormessage.
		errorMessage->Release();
		errorMessage = 0;

		// Mostramos en pantalla un mensaje para que el usuario verifique el txt para ver el error.
		MessageBox(hwnd, L"Error en la compilacion del shader.  Revise el archivo shader-error.txt para verificar el mensaje.", shaderFilename, MB_OK);

		return;
	}
};

#endif