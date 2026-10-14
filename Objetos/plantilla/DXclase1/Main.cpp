//////////////
// INCLUDES //
//////////////
#include <iostream>
#include <fstream>
using namespace std;


//////////////
// TYPEDEFS //
//////////////
typedef struct
{
	float x, y, z;
}VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
}FaceType;


///////////////////////////
//PROTOTIPOS DE FUNCIONES//
///////////////////////////
void GetModelFilename(char*);
bool ReadFileCounts(char*, int&, int&, int&, int&);
bool LoadDataStructures(char*, int, int, int, int);


//////////
// MAIN //
//////////
int main()
{
	bool result;
	char filename[256];
	int vertexCount, textureCount, normalCount, faceCount;
	char garbage;


	// Se lee el nombre del archivo
	GetModelFilename(filename);

	// Leemos el numero de vertices, uv's , normales y caras para inicializar las estructuras con el tamaño exacto.
	result = ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount);
	if (!result)
	{
		return -1;
	}

	// Desplegamos los totales a la pantalla solo para verificar
	cout << endl;
	cout << "Vertices: " << vertexCount << endl;
	cout << "UVs:      " << textureCount << endl;
	cout << "Normals:  " << normalCount << endl;
	cout << "Faces:    " << faceCount << endl;

	// Leemos la informacion del archivo en las estructuras y los escribimos en el archivo
	
	result = LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount);
	if (!result)
	{
		return -1;
	}

	// Notificamos que se pudo convertir el archivo.
	cout << "\nArchivo convertido exitosamente." << endl;
	cout << "\nDesea convertir otro archivo (s/n)? ";
	cin >> garbage;

	if (garbage == 's'){
		system("CLS");
		main();
	}
	return 0;
}


void GetModelFilename(char* filename)
{
	bool done;
	ifstream fin;


	// Nos quedamos esperando hasta que obtengamos un nombre de archivo
	done = false;
	while (!done)
	{
		// Preguntamos por el nombre de archivo
		cout << "Inserte el nombre del obj: ";

		// Leemos el nombre de archivo
		cin >> filename;

		// Intentamos abrirlo
		fin.open(filename);

		if (fin.good())
		{
			// Si el archivo existe y se puede abrir regresamos true
			done = true;
		}
		else
		{
			// Si el archivo no existe o hubo un problema al abrirlo notificamos al usuario
			fin.clear();
			cout << endl;
			cout << "El archivo " << filename << " no se pudo abrir." << endl << endl;
		}
	}

	return;
}


bool ReadFileCounts(char* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
{
	ifstream fin;
	char input;


	
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;


	fin.open(filename);

	// Verificamos que se haya podido abrir el archivo
	if (fin.fail() == true)
	{
		return false;
	}

	
	fin.get(input);
	while (!fin.eof())
	{
		// Si la linea comienza con V se lee ya sea el vertice, las uv's, o la normal.
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') { vertexCount++; }
			if (input == 't') { textureCount++; }
			if (input == 'n') { normalCount++; }
		}

		// Si encontramos una f, incrementamos el numero de caras
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ') { faceCount++; }
		}

		while (input != '\n')
		{
			fin.get(input);
		}

		// Comenzamos a leer la siguiente linea
		fin.get(input);
	}

	// Close the file.
	fin.close();

	return true;
}


bool LoadDataStructures(char* filename, int vertexCount, int textureCount, int normalCount, int faceCount)
{
	VertexType *vertices, *texcoords, *normals;
	FaceType *faces;
	ifstream fin;
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	char input, input2;
	ofstream fout;


	
	vertices = new VertexType[vertexCount];
	if (!vertices)
	{
		return false;
	}

	texcoords = new VertexType[textureCount];
	if (!texcoords)
	{
		return false;
	}

	normals = new VertexType[normalCount];
	if (!normals)
	{
		return false;
	}

	faces = new FaceType[faceCount];
	if (!faces)
	{
		return false;
	}

	
	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;

	
	fin.open(filename);

	// Checamos si se pudo abrir el archivo
	if (fin.fail() == true)
	{
		return false;
	}

	// Pasamos los vertices las uvs y las normales a las estructuras.
	// Importante: Convertimos el sistema de coordenadas a mano izquierda ya que maya utiliza un sistema de mano derecha.
	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);

			//Leemos los vertices
			if (input == ' ')
			{
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

				// Invertimos el vertice z para cambiar a sistema de mano izquierda.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++;
			}

			// Leemos las coordenadas de uvs
			if (input == 't')
			{
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

				// Invertimos la coordenada V por el sistema de mano izquierda
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++;
			}

			// Leemos las normales
			if (input == 'n')
			{
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

				// Invertimos la z de la normal 
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++;
			}
		}

		// Leemos las caras
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ')
			{
				// Leemos la informacion de cada cara al reves para pasar a sistema de mano izquierda.
				fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
					>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
					>> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
		}

		// Leemos lo que resta de la linea
		while (input != '\n')
		{
			fin.get(input);
		}

		// Comenzamos a leer la siguiente linea
		fin.get(input);
	}

	// Cerramos el archivo
	fin.close();

	// Creamos el archivo de salida.
	fout.open("modelo.txt");

	// Escribimos la cabezera del archivo que nuestro formato usara
	fout << "Vertex Count: " << (faceCount * 3) << endl;
	fout << endl;
	fout << "Data:" << endl;
	fout << endl;

	// Ahora haremos un loop por todas las caras e imprimiremos 3 vertices por cara
	for (int i = 0; i<faceIndex; i++)
	{
		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
			<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
			<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
			<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
			<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		fout << vertices[vIndex].x << ' ' << vertices[vIndex].y << ' ' << vertices[vIndex].z << ' '
			<< texcoords[tIndex].x << ' ' << texcoords[tIndex].y << ' '
			<< normals[nIndex].x << ' ' << normals[nIndex].y << ' ' << normals[nIndex].z << endl;
	}

	// Cerramos el archivo
	fout.close();

	// Liberamos la memoria de las estructuras.
	if (vertices)
	{
		delete[] vertices;
		vertices = 0;
	}
	if (texcoords)
	{
		delete[] texcoords;
		texcoords = 0;
	}
	if (normals)
	{
		delete[] normals;
		normals = 0;
	}
	if (faces)
	{
		delete[] faces;
		faces = 0;
	}

	return true;
}