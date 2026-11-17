#include "ModeloAssimp.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <functional>
#include <map>
#include <windows.h>
#include <assimp/material.h>

namespace {
aiVector3D VectorEnTiempo(const aiVectorKey* claves, unsigned int cantidad, double tiempo, const aiVector3D& defecto)
{
	if (!cantidad) return defecto;
	for (unsigned int i = 0; i + 1 < cantidad; ++i)
		if (tiempo < claves[i + 1].mTime) {
			double intervalo = claves[i + 1].mTime - claves[i].mTime;
			float t = intervalo > 0.0 ? (float)((tiempo - claves[i].mTime) / intervalo) : 0.0f;
			return claves[i].mValue + (claves[i + 1].mValue - claves[i].mValue) * t;
		}
	return claves[cantidad - 1].mValue;
}
aiQuaternion RotacionEnTiempo(const aiQuatKey* claves, unsigned int cantidad, double tiempo, const aiQuaternion& defecto)
{
	if (!cantidad) return defecto;
	for (unsigned int i = 0; i + 1 < cantidad; ++i)
		if (tiempo < claves[i + 1].mTime) {
			double intervalo = claves[i + 1].mTime - claves[i].mTime;
			float t = intervalo > 0.0 ? (float)((tiempo - claves[i].mTime) / intervalo) : 0.0f;
			aiQuaternion resultado;
			aiQuaternion::Interpolate(resultado, claves[i].mValue, claves[i + 1].mValue, t);
			return resultado.Normalize();
		}
	return claves[cantidad - 1].mValue;
}
}

void ModeloAssimp::Animar(double segundos, std::vector<aiVector3D>& posiciones, std::vector<aiVector3D>& normales, unsigned int indice) const
{
	if (!escena || !escena->mNumMeshes || indice >= escena->mNumAnimations) return;
	const aiMesh* mesh = escena->mMeshes[0];
	const aiAnimation* animacion = escena->mAnimations[indice];
	if (animacion->mDuration <= 0.0) return;
	double ticksPorSegundo = animacion->mTicksPerSecond > 0.0 ? animacion->mTicksPerSecond : 25.0;
	double segundosCiclo = animacion->mDuration / ticksPorSegundo;
	unsigned int fotograma = (unsigned int)(fmod(segundos, segundosCiclo) * 60.0);
	auto clave = std::make_pair(indice, fotograma);
	auto guardada = poses.find(clave);
	if (guardada != poses.end()) {
		posiciones = guardada->second.posiciones;
		normales = guardada->second.normales;
		return;
	}
	double tiempo = (fotograma / 60.0) * ticksPorSegundo;
	std::map<std::string, const aiNodeAnim*> canales;
	for (unsigned int i = 0; i < animacion->mNumChannels; ++i)
		canales[animacion->mChannels[i]->mNodeName.C_Str()] = animacion->mChannels[i];
	std::map<std::string, aiMatrix4x4> matrices;
	std::function<void(const aiNode*, const aiMatrix4x4&)> recorrer = [&](const aiNode* nodo, const aiMatrix4x4& padre) {
		aiMatrix4x4 local = nodo->mTransformation;
		auto it = canales.find(nodo->mName.C_Str());
		if (it != canales.end()) {
			aiVector3D escala, posicion; aiQuaternion rotacion;
			local.Decompose(escala, rotacion, posicion);
			const aiNodeAnim* canal = it->second;
			escala = VectorEnTiempo(canal->mScalingKeys, canal->mNumScalingKeys, tiempo, escala);
			posicion = VectorEnTiempo(canal->mPositionKeys, canal->mNumPositionKeys, tiempo, posicion);
			rotacion = RotacionEnTiempo(canal->mRotationKeys, canal->mNumRotationKeys, tiempo, rotacion);
			local = aiMatrix4x4(escala, rotacion, posicion);
		}
		aiMatrix4x4 global = padre * local;
		matrices[nodo->mName.C_Str()] = global;
		for (unsigned int i = 0; i < nodo->mNumChildren; ++i) recorrer(nodo->mChildren[i], global);
	};
	recorrer(escena->mRootNode, aiMatrix4x4());
	aiMatrix4x4 inversa = escena->mRootNode->mTransformation;
	inversa.Inverse();
	// El FBX puede almacenar la malla con una escala propia, separada del esqueleto.
	aiMatrix4x4 transformacionMalla;
	std::function<bool(const aiNode*)> buscarMalla = [&](const aiNode* nodo) {
		for (unsigned int i = 0; i < nodo->mNumMeshes; ++i)
			if (nodo->mMeshes[i] == 0) {
				transformacionMalla = inversa * matrices[nodo->mName.C_Str()];
				return true;
			}
		for (unsigned int i = 0; i < nodo->mNumChildren; ++i)
			if (buscarMalla(nodo->mChildren[i])) return true;
		return false;
	};
	buscarMalla(escena->mRootNode);
	aiMatrix3x3 normalMalla(transformacionMalla);
	normalMalla.Inverse().Transpose();
	posiciones.assign(mesh->mNumVertices, aiVector3D());
	normales.assign(mesh->mNumVertices, aiVector3D());
	std::vector<float> pesos(mesh->mNumVertices, 0.0f);
	for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
		const aiBone* hueso = mesh->mBones[i];
		auto it = matrices.find(hueso->mName.C_Str());
		if (it == matrices.end()) continue;
		aiMatrix4x4 matriz = inversa * it->second * hueso->mOffsetMatrix;
		aiMatrix3x3 normal(matriz); normal.Inverse().Transpose();
		for (unsigned int j = 0; j < hueso->mNumWeights; ++j) {
			unsigned int v = hueso->mWeights[j].mVertexId; float peso = hueso->mWeights[j].mWeight;
			if (v >= mesh->mNumVertices) continue;
			posiciones[v] += (matriz * (transformacionMalla * mesh->mVertices[v])) * peso;
			if (mesh->HasNormals()) normales[v] += (normal * (normalMalla * mesh->mNormals[v])) * peso;
			pesos[v] += peso;
		}
	}
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		if (pesos[i] > 0.0f) { posiciones[i] /= pesos[i]; normales[i].NormalizeSafe(); }
		else { posiciones[i] = transformacionMalla * mesh->mVertices[i]; if (mesh->HasNormals()) normales[i] = normalMalla * mesh->mNormals[i]; }
	}
	if (poses.size() >= 700) poses.erase(poses.begin());
	poses[clave] = { posiciones, normales };
}

bool ModeloAssimp::Cargar(const char* ruta, bool invertirUV)
{
	escena = nullptr;
	poses.clear();
	error.clear();
	texturasPorMaterial.clear();
	directorio.clear();
	if (!ruta || !ruta[0])
	{
		error = "Ruta de modelo vacia";
		return false;
	}
	std::string archivo(ruta);
	std::string::size_type separador = archivo.find_last_of("/\\");
	if (separador != std::string::npos) directorio = archivo.substr(0, separador + 1);

	unsigned int pasos = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
	if (invertirUV) pasos |= aiProcess_FlipUVs;
	escena = importador.ReadFile(ruta, pasos);
	if (!escena || !escena->mRootNode || (escena->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		error = importador.GetErrorString();
		if (error.empty()) error = "La escena esta incompleta";
		escena = nullptr;
		return false;
	}
	return true;
}

bool ModeloAssimp::Cargar(const char* ruta, const std::map<std::string, std::wstring>& texturas, bool invertirUV)
{
	if (!Cargar(ruta, invertirUV)) return false;
	texturasPorMaterial = texturas;
	return true;
}

std::wstring ModeloAssimp::TexturaDifusa(unsigned int indiceMalla) const
{
	if (!escena || indiceMalla >= escena->mNumMeshes) return L"";
	unsigned int indiceMaterial = escena->mMeshes[indiceMalla]->mMaterialIndex;
	if (indiceMaterial >= escena->mNumMaterials) return L"";
	const aiMaterial* material = escena->mMaterials[indiceMaterial];
	aiString nombre;
	if (material->Get(AI_MATKEY_NAME, nombre) == AI_SUCCESS)
	{
		auto it = texturasPorMaterial.find(nombre.C_Str());
		if (it != texturasPorMaterial.end()) return it->second;
	}
	aiString textura;
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textura) != AI_SUCCESS || textura.length == 0 || textura.C_Str()[0] == '*') return L"";
	std::string ruta = textura.C_Str();
	if (!(ruta.size() > 1 && ruta[1] == ':') && ruta[0] != '/' && ruta[0] != '\\') ruta = directorio + ruta;
	int longitud = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ruta.c_str(), -1, NULL, 0);
	if (!longitud) return L"";
	std::vector<wchar_t> resultado(longitud);
	if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ruta.c_str(), -1, resultado.data(), longitud)) return L"";
	return std::wstring(resultado.data());
}
