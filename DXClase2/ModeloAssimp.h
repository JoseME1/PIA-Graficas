#pragma once

#include <assimp/Importer.hpp>
#include <string>
#include <vector>
#include <assimp/scene.h>

struct aiScene;

// Conserva la escena mientras se consultan sus mallas, huesos y animaciones.
class ModeloAssimp
{
public:
	bool Cargar(const char* ruta, bool invertirUV = false);
	const aiScene* Escena() const { return escena; }
	const std::string& Error() const { return error; }
	void Animar(double segundos, std::vector<aiVector3D>& posiciones, std::vector<aiVector3D>& normales, unsigned int indice = 0) const;

private:
	Assimp::Importer importador;
	const aiScene* escena = nullptr;
	std::string error;
};
