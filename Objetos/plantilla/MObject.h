#ifndef _MObject_
#define _MObject_

class MObject
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
	ID3D11ShaderResourceView* NormalTexture;
	ID3D11SamplerState* TextSamplerState;
	LPCWSTR text;
	LPCWSTR normalText;

	D3DXMATRIX World;

	float Size;
	int cantIndices,cantVertices;

	
	//La estructura de buffer constante
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

	//La estructura de vertices
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 text;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 biTangent;
	};

	struct ModelType{

		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	ModelType* ModelDummy;

	MObject(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon, char* modelFileName, LPCWSTR textureFilename, HWND hwnd)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		LoadModel(modelFileName);
		
		this->text = textureFilename;
		
		InitScene(hwnd);
	}

	//con todas las figuras es igual, simplemente se cambian los vertices y los indices
	bool InitScene(HWND hwnd)
	{
		//este es el layout que va hacia el shader
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
		result = D3DX11CompileFromFile(L"pixelDiffuse.ps", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, &errorMessage, 0);
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

		//Creamos los objetos de shader
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		float cubeSize = Size / 2.0f;
		//estas son las coordenadas de los vertices del cubo
		SimpleVertex* vertex = new SimpleVertex[cantVertices];
		unsigned long* indices = new unsigned long[cantVertices];
		for (int i = 0; i<cantVertices; i++){

			vertex[i].pos = D3DXVECTOR3(ModelDummy[i].x, ModelDummy[i].y, ModelDummy[i].z);
			vertex[i].text = D3DXVECTOR2(ModelDummy[i].tu, ModelDummy[i].tv);
			vertex[i].normal = D3DXVECTOR3(ModelDummy[i].nx, ModelDummy[i].ny, ModelDummy[i].nz);

			indices[i] = i;
		}
		



		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)*cantVertices;
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



		//los indices del cubo
		
		
		int numTriangles = cantIndices / 3;
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
		indexBufferDesc.ByteWidth = sizeof(DWORD)* cantIndices;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, 
			&iinitData, 
			&IndexBuffer);

		//Creamos el buffer para mandarselo al cbuffer en el shader
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(ConstantBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbd, NULL, 
			&cbBuffer);


		// Creamos las texturas text y normalText
		D3DX11CreateShaderResourceViewFromFile( d3d11Device, text, NULL, NULL, &Texture, NULL );

		normalText = L"Texturas/escudoNormal.jpg";
		D3DX11CreateShaderResourceViewFromFile( d3d11Device, normalText, NULL, NULL, &NormalTexture, NULL );

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

	//aqui se hace el calculo del bump map
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
		tuVector[0] = vertex2.text.x - vertex1.text.x;
		tvVector[0] = vertex2.text.y - vertex1.text.y;

		tuVector[1] = vertex3.text.x - vertex1.text.x;
		tvVector[1] = vertex3.text.y - vertex1.text.y;

		// Calculate the denominator of the tangent/binormal equation.
		den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
		tangent = (tvVector[1] * vector1 - tvVector[0] * vector2) * den;
		binormal = (tuVector[0] * vector2 - tuVector[1] * vector1) * den;

		// Calculate the length of this normal.
		length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

		// Normalize the normal and then store it
		tangent = tangent / length;

		// Calculate the length of this normal.
		length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

		// Normalize the normal and then store it
		binormal = binormal / length;

		return;
	}

	//aqui esta el update que se usa para actualizar el cubo
	void UpdateScene(D3DXMATRIX World)
	{
		this->World = World;
	}

	//y este draw dibuja en base a las cosas que hemos creado
	void DrawScene(Camara *camara)
	{
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		
		//Pasamos el buffer de indices
		d3d11DevCon->IASetIndexBuffer( IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Pasamos el buffer de vertices
		d3d11DevCon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );

		//Seteamos los shaders 
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);

		consBuff.WorldViewProjection = World * camara->View * camara->Projection;
		consBuff.World = World;
		D3DXMatrixTranspose(&consBuff.WorldViewProjection, &consBuff.WorldViewProjection);
		D3DXMatrixTranspose(&consBuff.World, &consBuff.World);
		d3d11DevCon->UpdateSubresource(cbBuffer, 0, NULL, &consBuff, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbBuffer);
		
		d3d11DevCon->PSSetShaderResources( 0, 1, &Texture );
		d3d11DevCon->PSSetShaderResources( 1, 1, &NormalTexture );
		d3d11DevCon->PSSetSamplers(0, 1, &TextSamplerState);
		
		consBuffPS.light = light;
		d3d11DevCon->UpdateSubresource(cbBufferPS, 0, NULL, &consBuffPS, 0, 0);
		d3d11DevCon->PSSetConstantBuffers(0, 1, &cbBufferPS);
		
		d3d11DevCon->DrawIndexed(cantIndices, 0, 0);

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


	//Para cargar el obj
	bool LoadModel(char* filename){

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
		fin >> cantVertices;

		//Ponemos que la cantidad de indices sea la misma que la cantidad de vertices
		cantIndices = cantVertices;

		//creamos la estructura para guardar los datos del modelo utilizando como referencia
		//para la cantidad de espacio que se ocupara la cantidad de vertices
		ModelDummy = new ModelType[cantVertices];

		//Leemos desde el principio de la informacion
		fin.get(input);
		while (input != ':')
			fin.get(input);
		fin.get(input);
		fin.get(input);

		//Leemos la informacion de cada vertice
		for (i = 0; i<cantVertices; i++){

			fin >> ModelDummy[i].x >> ModelDummy[i].y >> ModelDummy[i].z;
			fin >> ModelDummy[i].tu >> ModelDummy[i].tv;
			fin >> ModelDummy[i].nx >> ModelDummy[i].ny >> ModelDummy[i].nz;


		}

		//Cerramos el archivo
		fin.close();
		return true;
	}


};

#endif