#pragma once

#include <assimp/Importer.hpp>
#include <string>
#include <vector>
#include <map>
#include <assimp/scene.h>

struct aiScene;

// Conserva la escena mientras se consultan sus mallas, huesos y animaciones.
class ModeloAssimp
{
public:
	bool Cargar(const char* ruta, bool invertirUV = false);
	bool Cargar(const char* ruta, const std::map<std::string, std::wstring>& texturas, bool invertirUV = false);
	std::wstring TexturaDifusa(unsigned int indiceMalla) const;
	const aiScene* Escena() const { return escena; }
	const std::string& Error() const { return error; }
	void Animar(double segundos, std::vector<aiVector3D>& posiciones, std::vector<aiVector3D>& normales, unsigned int indice = 0) const;

private:
	struct Pose { std::vector<aiVector3D> posiciones, normales; };
	mutable std::map<std::pair<unsigned int, unsigned int>, Pose> poses;
	Assimp::Importer importador;
	const aiScene* escena = nullptr;
	std::string error;
	std::string directorio;
	std::map<std::string, std::wstring> texturasPorMaterial;
};
