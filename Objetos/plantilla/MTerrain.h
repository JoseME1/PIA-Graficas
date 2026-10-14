#ifndef _MTerrain_
#define _MTerrain_

const int TEXTURE_REPEAT = 8;
class MTerrain
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
	int cantIndices;
	int m_terrainWidth, m_terrainHeight;
	
	struct HeightMapType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	//Estructura constante del buffer
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

	//Estructura de vertice 
	struct SimpleVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 text;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 biTangent;
	};

	HeightMapType* m_heightMap;

	MTerrain(ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon,LPCWSTR text,char* heightMapFilename,HWND hwnd)
	{
		D3DXMatrixIdentity(&World);

		this->d3d11Device = d3d11Device;
		this->d3d11DevCon = d3d11DevCon;

		this->Size = Size;
		this->text = text;
		LoadHeightMap(heightMapFilename);
		NormalizeHeightMap();
		CalculateNormals();
		CalculateTextureCoordinates();

		InitScene(hwnd);
	}

	//con todas las figuras es igual, simplemente cambio los vertices y los indices
	bool InitScene(HWND hwnd)
	{
		int index, i, j;
		int index1, index2, index3, index4;
		float tu, tv;
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

		//Compilamos los shaders
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

		//Creamos los objetos del shader
		d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
		d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);
		
		index = 0;

		//Creacion de vertices
		int const CantidadVertices = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;
		SimpleVertex *vertex = new SimpleVertex[CantidadVertices];
		unsigned long* indices = new unsigned long[CantidadVertices];
		for (j = 0; j<(m_terrainHeight - 1); j++)
		{
			for (i = 0; i<(m_terrainWidth - 1); i++)
			{
				index1 = (m_terrainHeight * j) + i;          // Bottom left.
				index2 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
				index3 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
				index4 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.

				// Upper left.
				tv = m_heightMap[index3].tv;

				//  Modificamos las coordenadas de textura para cubrir la orilla de arriba.
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				vertex[index].text = D3DXVECTOR2(m_heightMap[index3].tu, tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				indices[index] = index;
				index++;

				// Upper right.
				tu = m_heightMap[index4].tu;
				tv = m_heightMap[index4].tv;

				// Modificamos las coordenadas de textura para cubrir la orilla de arriba y de la derecha.
				if (tu == 0.0f) { tu = 1.0f; }
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				vertex[index].text = D3DXVECTOR2(tu, tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertex[index].pos = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				vertex[index].text = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertex[index].pos = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				vertex[index].text = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				indices[index] = index;
				index++;

				// Upper right.
				tu = m_heightMap[index4].tu;
				tv = m_heightMap[index4].tv;

				// Modificamos las coordenadas de textura para cubrir la orilla de arriba y la derecha.
				if (tu == 0.0f) { tu = 1.0f; }
				if (tv == 1.0f) { tv = 0.0f; }

				vertex[index].pos = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				vertex[index].text = D3DXVECTOR2(tu, tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom right.
				tu = m_heightMap[index2].tu;

				// Modificamos las coordenadas de textura para cubrir la orilla de la derecha.
				if (tu == 0.0f) { tu = 1.0f; }

				vertex[index].pos = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				vertex[index].text = D3DXVECTOR2(tu, m_heightMap[index2].tv);
				vertex[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				indices[index] = index;
				index++;
			}
		}

		
		int numeroDeVertices = CantidadVertices;



		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SimpleVertex)*numeroDeVertices;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertex;
		d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);

		//Creamos el input layout
		d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		//Lo pasamos al device context
		d3d11DevCon->IASetInputLayout(vertLayout);

		//Ponemos el modo de topologia de primitivos
		d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		
		cantIndices = CantidadVertices;
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
		indexBufferDesc.ByteWidth = sizeof(unsigned long)* cantIndices;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexBufferDesc, 
			&iinitData, 
			&IndexBuffer);

		//Creamos el buffer para mandar el buffer constante al shader
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(ConstantBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;

		d3d11Device->CreateBuffer(&cbbd, NULL, 
			&cbBuffer);


		// Spasamos las dos texturas la normal y la de el mapa de normales en caso de que haya
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

	//aqui esta el update 
	void UpdateScene(D3DXMATRIX World)
	{
		this->World = World;
	}

	//y este draw dibuja en base a las cosas que hemos creado
	void DrawScene(Camara *camara)
	{
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		//Colocamos el buffer de indices
		d3d11DevCon->IASetIndexBuffer( IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Colocamos el buffer de vertices
		d3d11DevCon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );

		//Seteamos el pixel y vertex shader
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
		m_terrainWidth = bitmapInfoHeader.biWidth;
		m_terrainHeight = bitmapInfoHeader.biHeight;

		//Calculamos el tamaño.
		imageSize = m_terrainWidth * m_terrainHeight * 3;

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
		m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
		if (!m_heightMap)
		{
			//return false;
		}

		
		k = 0;

		// Leemos la imagen para sacar la altura.
		for (j = 0; j<m_terrainHeight; j++)
		{
			for (i = 0; i<m_terrainWidth; i++)
			{
				height = bitmapImage[k];

				index = (m_terrainHeight * j) + i;

				m_heightMap[index].x = (float)i;
				m_heightMap[index].y = (float)height;
				m_heightMap[index].z = (float)j;

				k += 3;
			}
		}

		// Liberamos nuestro bitmap.
		delete[] bitmapImage;
		bitmapImage = 0;

	}

	void NormalizeHeightMap()
	{
		int i, j;


		for (j = 0; j<m_terrainHeight; j++)
		{
			for (i = 0; i<m_terrainWidth; i++)
			{
				m_heightMap[(m_terrainHeight * j) + i].y /= 15.0f;
			}
		}

		return;
	}

	void CalculateNormals()
	{
		int i, j, index1, index2, index3, index, count;
		float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
		D3DXVECTOR3* normals;


		// Creamos un arreglo temportal.
		normals = new D3DXVECTOR3[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
		if (!normals)
		{
			//return false;
		}

		// Calculamos las normales de todas las caras
		for (j = 0; j<(m_terrainHeight - 1); j++)
		{
			for (i = 0; i<(m_terrainWidth - 1); i++)
			{
				index1 = (j * m_terrainHeight) + i;
				index2 = (j * m_terrainHeight) + (i + 1);
				index3 = ((j + 1) * m_terrainHeight) + i;

				// Tomamos tres vertices de la cara.
				vertex1[0] = m_heightMap[index1].x;
				vertex1[1] = m_heightMap[index1].y;
				vertex1[2] = m_heightMap[index1].z;

				vertex2[0] = m_heightMap[index2].x;
				vertex2[1] = m_heightMap[index2].y;
				vertex2[2] = m_heightMap[index2].z;

				vertex3[0] = m_heightMap[index3].x;
				vertex3[1] = m_heightMap[index3].y;
				vertex3[2] = m_heightMap[index3].z;

				// Calculamos los dos vectores para esta cara.
				vector1[0] = vertex1[0] - vertex3[0];
				vector1[1] = vertex1[1] - vertex3[1];
				vector1[2] = vertex1[2] - vertex3[2];
				vector2[0] = vertex3[0] - vertex2[0];
				vector2[1] = vertex3[1] - vertex2[1];
				vector2[2] = vertex3[2] - vertex2[2];

				index = (j * (m_terrainHeight - 1)) + i;

				// Calculamos el producto cruz de estos dos vectores para obtener
				// el valor aun no normalizado para esta cara
				normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
				normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
				normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
			}
		}

		// Ahora pasamos por todos los vertices y tomas un promedio de todas las normales que ese 
		// vertice toca para sacar un promedio
		for (j = 0; j<m_terrainHeight; j++)
		{
			for (i = 0; i<m_terrainWidth; i++)
			{
				
				sum[0] = 0.0f;
				sum[1] = 0.0f;
				sum[2] = 0.0f;

				
				count = 0;

				// Bottom left face.
				if (((i - 1) >= 0) && ((j - 1) >= 0))
				{
					index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Bottom right face.
				if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
				{
					index = ((j - 1) * (m_terrainHeight - 1)) + i;

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Upper left face.
				if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
				{
					index = (j * (m_terrainHeight - 1)) + (i - 1);

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Upper right face.
				if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
				{
					index = (j * (m_terrainHeight - 1)) + i;

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Tomamos el promedio de las caras que tocan este vertice.
				sum[0] = (sum[0] / (float)count);
				sum[1] = (sum[1] / (float)count);
				sum[2] = (sum[2] / (float)count);

				// Calculamos la longitud de esta normal.
				length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

				//  Checamos el indice para localizar el vertice en el arreglo.
				index = (j * m_terrainHeight) + i;

				// Normalizamos la normal compartida para este vertice y la guardamos en el areglo.
				m_heightMap[index].nx = (sum[0] / length);
				m_heightMap[index].ny = (sum[1] / length);
				m_heightMap[index].nz = (sum[2] / length);
			}
		}

		// Liberamos las normales temporales.
		delete[] normals;
		normals = 0;

	}

	void CalculateTextureCoordinates()
	{
		int incrementCount, i, j, tuCount, tvCount;
		float incrementValue, tuCoordinate, tvCoordinate;


		// Calculamos que tanto vamos a incrementar las uvs.
		incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

		// Calculamos cuantas veces vamos a repetir la textura.
		incrementCount = m_terrainWidth / TEXTURE_REPEAT;

		// Indices para las tu y tv.
		tuCoordinate = 0.0f;
		tvCoordinate = 1.0f;

		
		tuCount = 0;
		tvCount = 0;

		// Loop para calcular las tu y tv de todo el heightmap.
		for (j = 0; j<m_terrainHeight; j++)
		{
			for (i = 0; i<m_terrainWidth; i++)
			{
				//  Guardamos las coordenadas de textura en el terreno.
				m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
				m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

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

		return;
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