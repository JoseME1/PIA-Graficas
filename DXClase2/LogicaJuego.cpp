#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <dinput.h>
#include <fstream>
using namespace std;
#include "DXRender.h"


void DXRender::CargarRuta()
{
		audio.DetenerMusicaCarrera();
		// Conserva que puntos ya se usaron en esta hora aunque se recargue el circuito
		std::vector<PuntoEntrenamiento> anteriores = puntosEntrenamiento;

		puntosRuta.clear();
		arosResistencia.clear();
		arosUsados.clear();
		puntosEntrenamiento.clear();
		qteActivo = false;

		radioMeta = 9.0f;
		radioPunto = 9.0f;
		radioAro = 6.0f;
		radioEntrenamiento = 6.0f;
		hudY = 0.62f;
		alturaMarcador = 0.35f;
		velocidadBase = 12.0f;
		aceleracionBase = 2.0f;
		resistenciaBase = 15.0f;

		FILE* archivo = NULL;
		if (fopen_s(&archivo, "ruta.txt", "r") != 0 || archivo == NULL)
		{
			estadoCarrera = CarreraSinRuta;
			return;
		}

		char linea[256];
		while (fgets(linea, sizeof(linea), archivo) != NULL)
		{
			float x = 0.0f;
			float z = 0.0f;

			if (strncmp(linea, "radio_meta", 10) == 0)
				sscanf_s(linea, "radio_meta = %f", &radioMeta);
			else if (strncmp(linea, "radio_punto", 11) == 0)
				sscanf_s(linea, "radio_punto = %f", &radioPunto);
			else if (strncmp(linea, "radio_aro", 9) == 0)
				sscanf_s(linea, "radio_aro = %f", &radioAro);
			else if (strncmp(linea, "radio_entrenamiento", 19) == 0)
				sscanf_s(linea, "radio_entrenamiento = %f", &radioEntrenamiento);
			else if (strncmp(linea, "hud_y", 5) == 0)
				sscanf_s(linea, "hud_y = %f", &hudY);
			else if (strncmp(linea, "altura_marcador", 15) == 0)
				sscanf_s(linea, "altura_marcador = %f", &alturaMarcador);
			else if (strncmp(linea, "velocidad", 9) == 0)
				sscanf_s(linea, "velocidad = %f", &velocidadBase);
			else if (strncmp(linea, "aceleracion", 11) == 0)
				sscanf_s(linea, "aceleracion = %f", &aceleracionBase);
			else if (strncmp(linea, "resistencia", 11) == 0)
				sscanf_s(linea, "resistencia = %f", &resistenciaBase);
			else if (strncmp(linea, "entrenamiento", 13) == 0)
			{
				char tipo[32] = "";
				if (sscanf_s(linea, "entrenamiento = %31[^,], %f , %f",
					tipo, (unsigned)sizeof(tipo), &x, &z) == 3)
				{
					PuntoEntrenamiento punto;
					punto.pos = D3DXVECTOR2(x, z);
					punto.usadoSlot = false;
					if (strncmp(tipo, "velocidad", 9) == 0)			punto.tipo = 0;
					else if (strncmp(tipo, "aceleracion", 11) == 0)	punto.tipo = 1;
					else											punto.tipo = 2;
					puntosEntrenamiento.push_back(punto);
				}
			}
			else if (strncmp(linea, "aro", 3) == 0)
			{
				if (sscanf_s(linea, "aro = %f , %f", &x, &z) == 2)
					arosResistencia.push_back(D3DXVECTOR2(x, z));
			}
			else if (sscanf_s(linea, "%f , %f", &x, &z) == 2)
				puntosRuta.push_back(D3DXVECTOR2(x, z));
		}
		fclose(archivo);

		// Recupera el estado "usado en esta hora" de los puntos que ya existian
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			for (size_t j = 0; j < anteriores.size(); j++)
			{
				if (puntosEntrenamiento[i].tipo == anteriores[j].tipo &&
					fabsf(puntosEntrenamiento[i].pos.x - anteriores[j].pos.x) < 0.02f &&
					fabsf(puntosEntrenamiento[i].pos.y - anteriores[j].pos.y) < 0.02f)
				{
					puntosEntrenamiento[i].usadoSlot = anteriores[j].usadoSlot;
					break;
				}
			}
		}

		RecalcularEstadisticas();

		arosUsados.assign(arosResistencia.size(), false);
		siguientePunto = 1;
		autoCorriendo = false;			// si se recarga a mitad de carrera, se corta la carrera automatica
		cuentaRegresiva = 0.0f;
		velocidadActual = 0.0f;
		resistenciaActual = 1.0f;
		estadoCarrera = (puntosRuta.size() >= 2) ? CarreraEsperando : CarreraSinRuta;
	}

	// Lee las vistas de camara desde camaras.txt. Una linea por vista, en orden:
	// "x, y, z, yaw, pitch" (yaw y pitch en grados).
	// Acepta la configuracion "seguir = 1" (la camara apunta a la calaca) o
	// "seguir = 0" (usa el yaw/pitch guardados tal cual).

void DXRender::CargarVistas()
{
		vistasCamara.clear();
		vistaActual = -1;

		vistasSeguir = true;
		velocidadVuelo = 20.0f;
		suavizadoCamara = 5.0f;

		FILE* archivo = NULL;
		if (fopen_s(&archivo, "camaras.txt", "r") != 0 || archivo == NULL)
			return;

		char linea[256];
		while (fgets(linea, sizeof(linea), archivo) != NULL)
		{
			float x = 0.0f, y = 0.0f, z = 0.0f, yaw = 0.0f, pitch = 0.0f;

			if (strncmp(linea, "seguir", 6) == 0)
			{
				int seguir = 1;
				sscanf_s(linea, "seguir = %d", &seguir);
				vistasSeguir = (seguir != 0);
			}
			else if (strncmp(linea, "velocidad_vuelo", 15) == 0)
				sscanf_s(linea, "velocidad_vuelo = %f", &velocidadVuelo);
			else if (strncmp(linea, "suavizado", 9) == 0)
				sscanf_s(linea, "suavizado = %f", &suavizadoCamara);
			else if (sscanf_s(linea, "%f , %f , %f , %f , %f", &x, &y, &z, &yaw, &pitch) == 5)
			{
				VistaCamara vista;
				vista.pos = D3DXVECTOR3(x, y, z);
				vista.yaw = D3DXToRadian(yaw);
				vista.pitch = D3DXToRadian(pitch);
				vistasCamara.push_back(vista);
			}
		}
		fclose(archivo);
	}


void DXRender::GuardarVistas()
{
		FILE* archivo = NULL;
		if (fopen_s(&archivo, "camaras.txt", "w") != 0 || archivo == NULL)
			return;

		fprintf(archivo, "# Vistas de camara de la carrera: la vista 1 se usa en el tramo hacia el\n");
		fprintf(archivo, "# checkpoint 1, la 2 hacia el checkpoint 2, etc. Si hay mas checkpoints que\n");
		fprintf(archivo, "# vistas, los tramos sobrantes se ven en tercera persona.\n");
		fprintf(archivo, "# Formato: x, y, z, yaw, pitch  (yaw y pitch en grados).\n");
		fprintf(archivo, "# Generado dentro del juego con F3 + P. Editable a mano.\n");
		fprintf(archivo, "seguir = %d\n", vistasSeguir ? 1 : 0);
		fprintf(archivo, "velocidad_vuelo = %.2f\n", velocidadVuelo);
		fprintf(archivo, "suavizado = %.2f\n", suavizadoCamara);
		for (size_t i = 0; i < vistasCamara.size(); i++)
		{
			fprintf(archivo, "%.2f, %.2f, %.2f, %.2f, %.2f\n",
				vistasCamara[i].pos.x, vistasCamara[i].pos.y, vistasCamara[i].pos.z,
				D3DXToDegree(vistasCamara[i].yaw), D3DXToDegree(vistasCamara[i].pitch));
		}
		fclose(archivo);
	}


void DXRender::GuardarRuta()
{
		FILE* archivo = NULL;
		if (fopen_s(&archivo, "ruta.txt", "w") != 0 || archivo == NULL)
			return;

		fprintf(archivo, "# Circuito: la primera linea de puntos es la meta, luego los checkpoints en orden.\n");
		fprintf(archivo, "# Generado dentro del juego con F2 + P. Los valores de abajo se pueden editar a mano.\n");
		fprintf(archivo, "radio_meta = %.2f\n", radioMeta);
		fprintf(archivo, "radio_punto = %.2f\n", radioPunto);
		fprintf(archivo, "radio_aro = %.2f\n", radioAro);
		fprintf(archivo, "radio_entrenamiento = %.2f\n", radioEntrenamiento);
		fprintf(archivo, "hud_y = %.2f\n", hudY);
		fprintf(archivo, "altura_marcador = %.2f\n", alturaMarcador);
		fprintf(archivo, "velocidad = %.2f\n", velocidadBase);
		fprintf(archivo, "aceleracion = %.2f\n", aceleracionBase);
		fprintf(archivo, "resistencia = %.2f\n", resistenciaBase);
		for (size_t i = 0; i < puntosRuta.size(); i++)
			fprintf(archivo, "%.2f, %.2f\n", puntosRuta[i].x, puntosRuta[i].y);
		for (size_t i = 0; i < arosResistencia.size(); i++)
			fprintf(archivo, "aro = %.2f, %.2f\n", arosResistencia[i].x, arosResistencia[i].y);
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			const char* tipo = (puntosEntrenamiento[i].tipo == 0) ? "velocidad"
				: (puntosEntrenamiento[i].tipo == 1) ? "aceleracion" : "resistencia";
			fprintf(archivo, "entrenamiento = %s, %.2f, %.2f\n", tipo,
				puntosEntrenamiento[i].pos.x, puntosEntrenamiento[i].pos.y);
		}
		fclose(archivo);
	}

	// Penalizacion por resistencia agotada:
	//   50% -> velocidad y aceleracion -10%
	//   25% -> -20%     0% -> -30%
	static float PenalizacionPorResistencia(float resistencia)
	{
		if (resistencia <= 0.001f) return 0.30f;
		if (resistencia <= 0.25f) return 0.20f;
		if (resistencia <= 0.50f) return 0.10f;
		return 0.0f;
	}


float DXRender::PenalizacionResistencia()
{
		return PenalizacionPorResistencia(resistenciaActual);
	}

	// Segundos que tarda la calaca en llegar a su velocidad maxima (con la
	// penalizacion de resistencia ya aplicada)

float DXRender::TiempoAceleracion()
{
		return aceleracionCarrera * (1.0f + PenalizacionResistencia());
	}

	// Lanza el quick time event de un aro: 4 flechas al azar (se pueden repetir)
	// y 2 segundos para pulsarlas en orden.

void DXRender::IniciarQTE()
{
		for (int i = 0; i < 4; i++)
		{
			qteSecuencia[i] = rand() % 4;
			qteSlotDir[i] = -1;			// fuerza a redibujar las flechas
			qteSlotVerde[i] = false;
		}

		qteProgreso = 0;
		qteTiempo = 2.0f;
		qteActivo = true;
	}

	// Aplica las mejoras de entrenamiento sobre los valores base de ruta.txt.
	// Cada entrenamiento suma +10% del valor base (aditivo). En aceleracion,
	// mejorar significa tardar menos, asi que el tiempo baja un 10%.

void DXRender::RecalcularEstadisticas()
{
		velocidadCarrera = velocidadBase * (1.0f + 0.10f * mejorasVelocidad);

		float factorAceleracion = 1.0f - 0.10f * mejorasAceleracion;
		if (factorAceleracion < 0.2f)
			factorAceleracion = 0.2f;
		aceleracionCarrera = aceleracionBase * factorAceleracion;

		resistenciaCarrera = resistenciaBase * (1.0f + 0.10f * mejorasResistencia);
	}


const wchar_t* DXRender::NombreSlot(int slot)
{
		switch (slot)
		{
		case 0: return L"Ma\u00F1ana";
		case 1: return L"D\u00EDa";
		case 2: return L"Tarde";
		default: return L"Noche";
		}
	}

	// Valores de destino de cada momento del dia. El angulo tambien lo usa el shader del
	// cielo (g_DayAngle) para que el fondo combine con la iluminacion.

void DXRender::AplicarLuzDelDia()
{
		bool veniaDeNoche = luzInicializada && sinf(anguloDestino) < 0.0f;
		if (amanecerConAudio) { audioAmanecer.Shutdown(); amanecerConAudio = false; amanecerPausado = false; }
		luzInicial = light;
		anguloInicial = g_DayAngle;
		luzDestino = light;
		int slot = (diaActual >= 3) ? 1 : slotActual;	// dia de carrera = pleno dia

		switch (slot)
		{
		case 0:		// mañana: sol bajo, tono calido
			anguloDestino = 0.55f;
			luzDestino.ambient = D3DXVECTOR4(0.32f, 0.31f, 0.30f, 1.0f);
			luzDestino.diffuse = D3DXVECTOR4(0.95f, 0.88f, 0.78f, 1.0f);
			luzDestino.specular = D3DXVECTOR4(0.70f, 0.65f, 0.60f, 1.0f);
			break;
		case 1:		// dia: sol alto y blanco
			anguloDestino = 1.40f;
			luzDestino.ambient = D3DXVECTOR4(0.40f, 0.40f, 0.40f, 1.0f);
			luzDestino.diffuse = D3DXVECTOR4(1.00f, 1.00f, 1.00f, 1.0f);
			luzDestino.specular = D3DXVECTOR4(1.00f, 1.00f, 1.00f, 1.0f);
			break;
		case 2:		// tarde: sol bajo, naranja
			anguloDestino = 2.90f;
			luzDestino.ambient = D3DXVECTOR4(0.30f, 0.26f, 0.23f, 1.0f);
			luzDestino.diffuse = D3DXVECTOR4(1.00f, 0.72f, 0.45f, 1.0f);
			luzDestino.specular = D3DXVECTOR4(0.90f, 0.65f, 0.40f, 1.0f);
			break;
		default:	// noche: luz de luna azulada
			anguloDestino = 3.80f;
			luzDestino.ambient = D3DXVECTOR4(0.16f, 0.17f, 0.24f, 1.0f);
			luzDestino.diffuse = D3DXVECTOR4(0.12f, 0.13f, 0.22f, 1.0f);
			luzDestino.specular = D3DXVECTOR4(0.05f, 0.05f, 0.08f, 1.0f);
			break;
		}
		luzDestino.specular.x *= 0.55f;
		luzDestino.specular.y *= 0.55f;
		luzDestino.specular.z *= 0.55f;

		// Al comenzar un nuevo dia el sol continua por debajo del horizonte.
		while (anguloDestino < anguloInicial)
			anguloDestino += 6.2831853f;
		transicionLuz = 0.0f;
		if (veniaDeNoche && (slot == 0 || (diaActual >= 3 && slot == 1)))
		{
			amanecerConAudio = audioAmanecer.Init(L"Sonidos/operasun.wav", audioVolume, false);
			if (!amanecerConAudio) audioAmanecer.Shutdown();
			else
			{
				// Busca el horizonte del terreno en la direccion de salida del sol.
				const float direccionX = 0.857493f, direccionZ = 0.514496f;
				float pendiente = 0.0f;
				for (float distancia = 8.0f; distancia <= 420.0f; distancia += 2.0f)
				{
					float x = camara->Position.x + direccionX * distancia;
					float z = camara->Position.z + direccionZ * distancia;
					if (x < 0.0f || x >= 299.0f || z < 0.0f || z >= 299.0f) break;
					float altura = terreno->getTerrainCollision(D3DXVECTOR2(x, z));
					pendiente = (std::max)(pendiente, (altura - camara->Position.y) / distancia);
				}
				float horizonte = atanf(pendiente * 1.16619f) - 0.02f;
				float inicioCiclo = anguloDestino - fmodf(anguloDestino, 6.2831853f);
				anguloInicial = inicioCiclo + horizonte;
				if (anguloInicial > anguloDestino - 0.02f) anguloInicial = anguloDestino - 0.02f;
			}
		}
		if (!luzInicializada)
		{
			transicionLuz = 1.0f;
			luzInicializada = true;
			ActualizarLuzDelDia();
		}
	}


void DXRender::ActualizarLuzDelDia()
{
		if (transicionLuz >= 1.0f)
			transicionLuz = 1.0f;
		else if (amanecerConAudio)
		{
			if (menuEstado != 0 && !amanecerPausado) { audioAmanecer.Pausar(); amanecerPausado = true; }
			else if (menuEstado == 0 && amanecerPausado) { audioAmanecer.Reanudar(); amanecerPausado = false; }
			transicionLuz = audioAmanecer.Progreso();
			if (transicionLuz >= 1.0f) { audioAmanecer.Shutdown(); amanecerConAudio = false; amanecerPausado = false; }
		}
		else if (menuEstado == 0)
		{
			transicionLuz += deltaTime / 4.0f;
			if (transicionLuz > 1.0f) transicionLuz = 1.0f;
		}

		float t = transicionLuz * transicionLuz * (3.0f - 2.0f * transicionLuz);
		g_DayAngle = anguloInicial + (anguloDestino - anguloInicial) * t;
		D3DXVec4Lerp(&light.ambient, &luzInicial.ambient, &luzDestino.ambient, t);
		D3DXVec4Lerp(&light.diffuse, &luzInicial.diffuse, &luzDestino.diffuse, t);
		D3DXVec4Lerp(&light.specular, &luzInicial.specular, &luzDestino.specular, t);
		D3DXVECTOR3 sol(cosf(g_DayAngle), sinf(g_DayAngle), cosf(g_DayAngle) * 0.6f);
		D3DXVec3Normalize(&sol, &sol);
		light.direction = sol;
	}

void DXRender::MostrarBannerDia()
{
		if (diaActual == 1)
		{
			bannerTitulo = L"D\u00EDa 1, Entrenamiento";
			bannerSub = L"Faltan 2 d\u00EDas para la carrera";
		}
		else if (diaActual == 2)
		{
			bannerTitulo = L"D\u00EDa 2, Entrenamiento";
			bannerSub = L"Falta 1 d\u00EDa para la carrera";
		}
		else
		{
			bannerTitulo = L"D\u00EDa de la Carrera";
			bannerSub = L"Ve a la meta y pulsa E";
		}

		bannerTiempo = 5.0f;
	}

	// Aviso de fin de carrera, con el mismo estilo que el aviso del dia.

void DXRender::MostrarBannerFinal()
{
		bannerTitulo = L"\u00A1Carrera terminada!";

		wchar_t sub[64];
		swprintf_s(sub, L"%d\u00B0 lugar", RangoJugador());
		bannerSub = sub;

		bannerTiempo = 6.0f;
	}

	// Termina el dia y arranca el siguiente (o el dia de la carrera)

void DXRender::AvanzarDia()
{
		diaActual++;
		entrenamientosHoy = 0;
		slotActual = 0;
		tiempoMensajeEntrenamiento = 0.0f;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
			puntosEntrenamiento[i].usadoSlot = false;

		AplicarLuzDelDia();
		MostrarBannerDia();
	}

	// Completa una sesion de entrenamiento del tipo indicado

void DXRender::Entrenar(int tipo)
{
		if (tipo == 0)			mejorasVelocidad++;
		else if (tipo == 1)		mejorasAceleracion++;
		else					mejorasResistencia++;

		RecalcularEstadisticas();

		entrenamientosHoy++;
		slotActual = entrenamientosHoy;

		MejorarCalacasIA();		// las rivales tambien entrenan una vez por hora

		wchar_t buffer[192];
		if (tipo == 0)
			swprintf_s(buffer, L"Entrenamiento de velocidad completado!    Velocidad: %.1f", velocidadCarrera);
		else if (tipo == 1)
			swprintf_s(buffer, L"Entrenamiento de aceleraci\u00F3n completado!    Aceleraci\u00F3n: %.2f s", aceleracionCarrera);
		else
			swprintf_s(buffer, L"Entrenamiento de resistencia completado!    Resistencia: %.1f", resistenciaCarrera);

		mensajeEntrenamiento = buffer;
		tiempoMensajeEntrenamiento = 3.0f;

		if (entrenamientosHoy >= 4)
			IniciarFaseAlmas();		// al terminar el dia, salen las almas de poder
		else
		{
			// Nueva hora del dia: cada punto vuelve a estar disponible, asi que
			// se puede usar el mismo punto una vez por franja (4 veces al dia).
			for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
				puntosEntrenamiento[i].usadoSlot = false;

			AplicarLuzDelDia();
		}
	}

	// E junto a un punto de entrenamiento disponible

void DXRender::IntentarEntrenar()
{
		if (diaActual >= 3 || entrenamientosHoy >= 4)
			return;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			float dx = puntosEntrenamiento[i].pos.x - playerPos.x;
			float dz = puntosEntrenamiento[i].pos.y - playerPos.z;

			if (dx * dx + dz * dz > radioEntrenamiento * radioEntrenamiento)
				continue;

			if (puntosEntrenamiento[i].usadoSlot)
				continue;

			puntosEntrenamiento[i].usadoSlot = true;
			Entrenar(puntosEntrenamiento[i].tipo);
			return;
		}
	}

	// Gasta un alma en el stat elegido. Cuando no quedan almas por consumir,
	// arranca el dia siguiente (o la carrera si era el dia 2).

void DXRender::ConsumirAlma(int stat)
{
		if (!consumiendoAlmas || almasConsumidas >= almasRecogidas)
			return;

		if (stat == 0)			mejorasVelocidad++;
		else if (stat == 1)		mejorasAceleracion++;
		else					mejorasResistencia++;

		almasBonus[stat] += 10;		// cada alma sube un 10% ese stat
		almasConsumidas++;
		RecalcularEstadisticas();

		if (almasConsumidas >= almasRecogidas)
		{
			consumiendoAlmas = false;
			AvanzarDia();
		}
	}

	// Las almas que el jugador no recogio se reparten entre las rivales:
	// 50% elite, 30% experimentada, 10% aficionada y 10% principiante.
	// Cada una sube 8% a un stat al azar (extras, aparte de sus mejoras).

void DXRender::RepartirAlmasPerdidas(int cantidad)
{
		for (int i = 0; i < cantidad; i++)
		{
			int tirada = rand() % 100;
			int indice = (tirada < 50) ? 0 : (tirada < 80) ? 1 : (tirada < 90) ? 2 : 3;

			calacasIA[indice].mejorasExtra[rand() % 3]++;
		}
	}

	// Devuelve false si (x, z) cae dentro de un objeto con colisiones.

bool DXRender::PosicionLibreAlma(float x, float z)
{
		for (int i = 0; i < 9; i++)
		{
			if (i == 5) continue;
			float dx = x - colliders[i].x;
			float dz = z - colliders[i].z;
			float r = colliders[i].radius + 2.0f;

			if (dx * dx + dz * dz < r * r)
				return false;
		}

		for (size_t i = 0; i < 25 + colisionesCementerio.size(); i++)
		{
			if (i == 20) continue;
			const BoxCollider& caja = i < 25 ? castleWalls[i] : colisionesCementerio[i - 25];
			if (x > caja.minX - 2.0f && x < caja.maxX + 2.0f &&
				z > caja.minZ - 2.0f && z < caja.maxZ + 2.0f)
				return false;
		}

		return true;
	}

	// Devuelve true si (x, z) esta lo bastante lejos de las almas ya colocadas.

bool DXRender::AlmaLejos(float x, float z, int hasta)
{
		for (int j = 0; j < hasta; j++)
		{
			float dx = x - almasPos[j].x;
			float dz = z - almasPos[j].z;

			if (dx * dx + dz * dz < 55.0f * 55.0f)
				return false;
		}

		return true;
	}

	// Reparte las 5 almas por cualquier parte del mapa, evitando los objetos
	// con colisiones y sin que queden apiladas entre si. La primera siempre
	// queda escondida dentro del castillo.

void DXRender::ColocarAlmas()
{
		const float margen = 15.0f;			// el terreno mide 300 x 300
		const float rango = 300.0f - 2.0f * margen;

		for (int i = 0; i < almasTotal; i++)
		{
			almaDisponible[i] = true;

			float x = 150.0f, z = 150.0f;
			bool encontrada = false;

			// Alma escondida en el patio del castillo (X 120-180, Z 232-268)
			if (i == 0)
			{
				for (int intento = 0; intento < 400 && !encontrada; intento++)
				{
					float px = 120.0f + (float)(rand() % 60);
					float pz = 232.0f + (float)(rand() % 36);

					if (PosicionLibreAlma(px, pz))
					{
						x = px;
						z = pz;
						encontrada = true;
					}
				}
			}

			for (int intento = 0; intento < 400 && !encontrada; intento++)
			{
				float px = margen + (float)(rand() % (int)rango);
				float pz = margen + (float)(rand() % (int)rango);

				if (!PosicionLibreAlma(px, pz))
					continue;

				if (!AlmaLejos(px, pz, i))
					continue;

				x = px;
				z = pz;
				encontrada = true;
			}

			almasPos[i].x = x;
			almasPos[i].z = z;
			almasPos[i].y = terreno->getTerrainCollision(D3DXVECTOR2(x, z));
		}
	}

	// Arranca la fase de las almas: aviso grande y despues el minuto.

void DXRender::IniciarFaseAlmas()
{
		faseAlmas = true;
		almasTiempo = 60.0f;
		almasRecogidas = 0;
		consumiendoAlmas = false;
		almasConsumidas = 0;
		almasBonus[0] = almasBonus[1] = almasBonus[2] = 0;

		bannerTitulo = L"Busca las almas de poder";
		bannerSub = L"Las almas de los ca\u00EDdos mejoran el poder de las calacas";
		bannerTiempo = 5.0f;

		ColocarAlmas();
		hudTexto.clear();
	}

	// Se acabo el minuto: las que quedaron se reparten entre las rivales y, si
	// el jugador junto alguna, aparece el menu para gastarlas.

void DXRender::TerminarFaseAlmas()
{
		int perdidas = 0;
		for (int i = 0; i < almasTotal; i++)
			if (almaDisponible[i])
				perdidas++;

		RepartirAlmasPerdidas(perdidas);

		faseAlmas = false;
		almasTiempo = 0.0f;
		hudTexto.clear();

		if (almasRecogidas > 0)
		{
			consumiendoAlmas = true;
			almasConsumidas = 0;
			almasBonus[0] = almasBonus[1] = almasBonus[2] = 0;
		}
		else
		{
			AvanzarDia();
		}
	}

	// Indice del alma que el jugador tiene a tiro, o -1 si no hay ninguna.

int DXRender::AlmaCercana()
{
		for (int i = 0; i < almasTotal; i++)
		{
			if (!almaDisponible[i])
				continue;

			float dx = almasPos[i].x - playerPos.x;
			float dz = almasPos[i].z - playerPos.z;

			if (dx * dx + dz * dz <= radioAlma * radioAlma)
				return i;
		}

		return -1;
	}

	// Recoge el alma que este a tiro (tecla E)

void DXRender::RecogerAlmaCercana()
{
		int i = AlmaCercana();
		if (i < 0)
			return;

		almaDisponible[i] = false;
		almasRecogidas++;
	}

	// Contador y recogida de almas. El minuto empieza cuando se va el aviso y
	// termina al agotarse el tiempo o al juntar las 5.

void DXRender::ActualizarFaseAlmas()
{
		if (bannerTiempo > 0.0f)
			return;

		almasTiempo -= deltaTime;

		if (almasRecogidas >= almasTotal || almasTiempo <= 0.0f)
		{
			TerminarFaseAlmas();
			return;
		}

		wchar_t buffer[160];
		if (AlmaCercana() >= 0)
			swprintf_s(buffer, L"Almas de poder: %d/%d\nPulsa  E  para recoger",
				almasRecogidas, almasTotal);
		else
			swprintf_s(buffer, L"Almas de poder: %d/%d", almasRecogidas, almasTotal);
		hudTexto = buffer;
	}

	// HUD de los dias de entrenamiento: aviso cerca del punto y estado del dia

void DXRender::ActualizarEntrenamiento()
{
		if (tiempoMensajeEntrenamiento > 0.0f)
		{
			tiempoMensajeEntrenamiento -= deltaTime;
			hudTexto = mensajeEntrenamiento;
			return;
		}

		wchar_t buffer[256];

		// Punto de entrenamiento mas cercano dentro del radio
		int cerca = -1;
		float mejorDistancia = radioEntrenamiento * radioEntrenamiento;
		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
		{
			float dx = puntosEntrenamiento[i].pos.x - playerPos.x;
			float dz = puntosEntrenamiento[i].pos.y - playerPos.z;
			float distancia = dx * dx + dz * dz;

			if (distancia <= mejorDistancia)
			{
				mejorDistancia = distancia;
				cerca = (int)i;
			}
		}

		if (cerca >= 0)
		{
			const wchar_t* nombre = (puntosEntrenamiento[cerca].tipo == 0) ? L"Velocidad"
				: (puntosEntrenamiento[cerca].tipo == 1) ? L"Aceleraci\u00F3n" : L"Resistencia";

			if (puntosEntrenamiento[cerca].usadoSlot)
				swprintf_s(buffer, L"Ya entrenaste %s en esta hora.    Entrenamientos: %d/4",
					nombre, entrenamientosHoy);
			else
				swprintf_s(buffer, L"Presiona E para entrenar: %s    (%d/4 hoy)",
					nombre, entrenamientosHoy);

			hudTexto = buffer;
			return;
		}

		swprintf_s(buffer, L"D\u00EDa %d - %s    Entrenamientos: %d/4",
			diaActual, NombreSlot(slotActual), entrenamientosHoy);
		hudTexto = buffer;
	}

	// Estado inicial: dia 1 por la mañana

void DXRender::IniciarDias()
{
		diaActual = 1;
		luzInicializada = false;
		entrenamientosHoy = 0;
		slotActual = 0;
		mejorasVelocidad = 0;
		mejorasAceleracion = 0;
		mejorasResistencia = 0;
		RecalcularEstadisticas();

		// Las rivales empiezan sin mejoras, igual que el jugador
		for (int i = 0; i < 4; i++)
		{
			calacasIA[i].mejoras[0] = calacasIA[i].mejoras[1] = calacasIA[i].mejoras[2] = 0;
			calacasIA[i].mejorasExtra[0] = calacasIA[i].mejorasExtra[1] = calacasIA[i].mejorasExtra[2] = 0;
		}

		faseAlmas = false;
		almasRecogidas = 0;
		consumiendoAlmas = false;
		almasConsumidas = 0;

		for (size_t i = 0; i < puntosEntrenamiento.size(); i++)
			puntosEntrenamiento[i].usadoSlot = false;

		AplicarLuzDelDia();
		MostrarBannerDia();
	}

	// Cada hora de entrenamiento las rivales mejoran un stat al azar

void DXRender::MejorarCalacasIA()
{
		for (int i = 0; i < 4; i++)
			calacasIA[i].mejoras[rand() % 3]++;
	}

	// Calcula velocidad, aceleracion y resistencia efectivas de una rival con
	// las mejoras que lleva acumuladas. Misma formula que usa la carrera.

void DXRender::CalcularStatsIA(CalacaIA& ia)
{
		ia.velocidad = velocidadBase * (1.0f + ia.porcentajeMejora * ia.mejoras[0]
			+ 0.08f * ia.mejorasExtra[0]);

		float factorAceleracion = 1.0f - ia.porcentajeMejora * ia.mejoras[1]
			- 0.08f * ia.mejorasExtra[1];
		if (factorAceleracion < 0.2f) factorAceleracion = 0.2f;
		ia.aceleracion = aceleracionBase * factorAceleracion;

		ia.resistencia = resistenciaBase * (1.0f + ia.porcentajeMejora * ia.mejoras[2]
			+ 0.08f * ia.mejorasExtra[2]);
	}

	// Prepara a las rivales al empezar la carrera: aplica las mejoras de los
	// dos dias de entrenamiento y las coloca en la linea de salida.

void DXRender::IniciarCalacasIA()
{
		static const float desvio[4][2] = {
			{ 2.5f, 2.5f }, { -2.5f, 2.5f }, { 2.5f, -2.5f }, { -2.5f, -2.5f }
		};

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];

			CalcularStatsIA(ia);

			ia.pos = D3DXVECTOR3(puntosRuta[0].x + desvio[i][0], 0.0f,
				puntosRuta[0].y + desvio[i][1]);
			ia.pos.y = terreno->getTerrainCollision(D3DXVECTOR2(ia.pos.x, ia.pos.z));

			ia.velocidadActual = 0.0f;
			ia.resistenciaActual = 1.0f;
			ia.siguientePunto = 1;
			ia.terminado = false;
			ia.tiempoLlegada = 0.0f;
			ia.yaw = 0.0f;
			ia.arosUsados.assign(arosResistencia.size(), false);
		}

		relojCarrera = 0.0f;
		tiempoLlegadaJugador = 0.0f;
	}

	// Mueve a las 4 rivales igual que la calaca del jugador: aceleran hacia su
	// velocidad maxima, gastan resistencia y pasan los aros con su probabilidad.

void DXRender::ActualizarCalacasIA()
{
		relojCarrera += deltaTime;

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];
			if (ia.terminado)
				continue;

			float dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
			float dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
			float distancia = sqrtf(dx * dx + dz * dz);

			if (distancia > 0.001f)
			{
				float penalizacion = PenalizacionPorResistencia(ia.resistenciaActual);
				float velocidadObjetivo = ia.velocidad * (1.0f - penalizacion);

				float tiempoAceleracion = ia.aceleracion * (1.0f + penalizacion);
				if (tiempoAceleracion < 0.01f) tiempoAceleracion = 0.01f;
				float tasaAceleracion = velocidadObjetivo / tiempoAceleracion;

				if (ia.velocidadActual < velocidadObjetivo)
				{
					ia.velocidadActual += tasaAceleracion * deltaTime;
					if (ia.velocidadActual > velocidadObjetivo) ia.velocidadActual = velocidadObjetivo;
				}
				else
				{
					ia.velocidadActual -= tasaAceleracion * deltaTime;
					if (ia.velocidadActual < velocidadObjetivo) ia.velocidadActual = velocidadObjetivo;
				}

				float paso = ia.velocidadActual * deltaTime;
				if (paso > distancia) paso = distancia;

				float metrosDeResistencia = ia.resistencia * ia.velocidad;
				if (metrosDeResistencia > 0.001f)
				{
					ia.resistenciaActual -= paso / metrosDeResistencia;
					if (ia.resistenciaActual < 0.0f) ia.resistenciaActual = 0.0f;
				}

				ia.pos.x += dx / distancia * paso;
				ia.pos.z += dz / distancia * paso;
				ia.yaw = atan2f(dx, dz);

				dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
				dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
				distancia = sqrtf(dx * dx + dz * dz);
			}

			// Aros de resistencia: cada rival los pasa una sola vez y tira su
			// probabilidad de lograr el quick time event (+25% de resistencia).
			for (size_t a = 0; a < arosResistencia.size(); a++)
			{
				if (a < ia.arosUsados.size() && ia.arosUsados[a])
					continue;

				float ax = arosResistencia[a].x - ia.pos.x;
				float az = arosResistencia[a].y - ia.pos.z;

				if (ax * ax + az * az <= radioAro * radioAro)
				{
					ia.arosUsados[a] = true;

					float suerte = (float)rand() / (float)RAND_MAX;
					if (suerte <= ia.probabilidadAro)
					{
						ia.resistenciaActual += 0.25f;
						if (ia.resistenciaActual > 1.0f) ia.resistenciaActual = 1.0f;
					}
				}
			}

			if (distancia <= radioPunto)
			{
				ia.siguientePunto++;

				if (ia.siguientePunto >= (int)puntosRuta.size())
				{
					ia.terminado = true;
					ia.tiempoLlegada = relojCarrera;
				}
			}

			ia.pos.y = terreno->getTerrainCollision(D3DXVECTOR2(ia.pos.x, ia.pos.z));
		}
	}

	// Progreso en la carrera: mas alto = mas adelante.

float DXRender::ProgresoCalacaIA(int i)
{
		CalacaIA &ia = calacasIA[i];
		float dx = puntosRuta[ia.siguientePunto].x - ia.pos.x;
		float dz = puntosRuta[ia.siguientePunto].y - ia.pos.z;
		return (ia.siguientePunto - 1) * 10000.0f - sqrtf(dx * dx + dz * dz);
	}


float DXRender::ProgresoJugador()
{
		float dx = puntosRuta[siguientePunto].x - playerPos.x;
		float dz = puntosRuta[siguientePunto].y - playerPos.z;
		return (siguientePunto - 1) * 10000.0f - sqrtf(dx * dx + dz * dz);
	}


int DXRender::RangoJugador()
{
		bool llegue = (estadoCarrera == CarreraTerminada || siguientePunto >= (int)puntosRuta.size());
		int rango = 1;

		for (int i = 0; i < 4; i++)
		{
			CalacaIA &ia = calacasIA[i];

			if (llegue)
			{
				// Ya termine: solo me superan las que llegaron antes que yo.
				// (Comparar tiempos directamente evita que el float pierda
				// precision y empate llegadas con pocos segundos de diferencia.)
				if (ia.terminado && ia.tiempoLlegada < tiempoLlegadaJugador)
					rango++;
			}
			else if (ia.terminado || ProgresoCalacaIA(i) > ProgresoJugador())
			{
				rango++;
			}
		}

		return rango;
	}


void DXRender::ActualizarCarrera()
{
		if (amanecerConAudio)
		{
			hudTexto = L"Espera a que termine el amanecer";
			return;
		}
		if (modoRuta || modoCamara)
			return;

		// Los dos primeros dias son de entrenamiento; la carrera es el tercero
		if (diaActual < 3)
		{
			if (consumiendoAlmas)
				return;			// el menu espera a que se gasten las almas

			if (faseAlmas)
			{
				ActualizarFaseAlmas();
				return;
			}

			ActualizarEntrenamiento();
			return;
		}

		if (estadoCarrera == CarreraSinRuta)
		{
			hudTexto.clear();
			return;
		}

		if (estadoCarrera == CarreraEsperando)
		{
			float dx = playerPos.x - puntosRuta[0].x;
			float dz = playerPos.z - puntosRuta[0].y;
			if (dx * dx + dz * dz <= radioMeta * radioMeta)
				hudTexto = L"Presiona  E  para empezar la carrera";
			else
				hudTexto.clear();
			return;
		}

		if (estadoCarrera == CarreraCuenta)
		{
			cuentaRegresiva -= deltaTime;

			if (cuentaRegresiva > 3.0f)		hudTexto = L"3";
			else if (cuentaRegresiva > 2.0f)	hudTexto = L"2";
			else if (cuentaRegresiva > 1.0f)	hudTexto = L"1";
			else if (cuentaRegresiva > 0.0f)	hudTexto = L"\u00A1Ya!";
			else
			{
				estadoCarrera = CarreraCorriendo;
				audio.IniciarMusicaCarrera();
				autoCorriendo = true;
				hudTexto.clear();
			}
			return;
		}

		if (estadoCarrera == CarreraCorriendo)
		{
			ActualizarCalacasIA();

			// Ulti: mientras esta activa se apaga en 2 segundos; si no, se
			// recarga sola un 3% por segundo.
			if (ultiTiempo > 0.0f)
			{
				ultiTiempo -= deltaTime;
				if (ultiTiempo < 0.0f) ultiTiempo = 0.0f;
				cargaUlti = ultiTiempo / 2.0f;
			}
			else
			{
				cargaUlti += 0.03f * deltaTime;
				if (cargaUlti > 1.0f) cargaUlti = 1.0f;
			}

			float dx = puntosRuta[siguientePunto].x - playerPos.x;
			float dz = puntosRuta[siguientePunto].y - playerPos.z;
			float distancia = sqrtf(dx * dx + dz * dz);

			// La calaca avanza sola hacia el siguiente punto
			if (autoCorriendo && distancia > 0.001f)
			{
				// Estadisticas: con la resistencia baja, la velocidad maxima y la
				// aceleracion se reducen, asi que cuesta mas llegar a la meta.
				float penalizacion = PenalizacionResistencia();
				float velocidadObjetivo = velocidadCarrera * (1.0f - penalizacion);

				// La ulti da un 30% mas de velocidad mientras esta activa
				if (ultiTiempo > 0.0f)
					velocidadObjetivo *= 1.3f;

				float tiempoAceleracion = TiempoAceleracion();
				if (tiempoAceleracion < 0.01f) tiempoAceleracion = 0.01f;
				float tasaAceleracion = velocidadObjetivo / tiempoAceleracion;

				if (velocidadActual < velocidadObjetivo)
				{
					velocidadActual += tasaAceleracion * deltaTime;
					if (velocidadActual > velocidadObjetivo) velocidadActual = velocidadObjetivo;
				}
				else
				{
					velocidadActual -= tasaAceleracion * deltaTime;
					if (velocidadActual < velocidadObjetivo) velocidadActual = velocidadObjetivo;
				}

				float paso = velocidadActual * deltaTime;
				if (paso > distancia) paso = distancia;

				// La resistencia baja cada metro: a velocidad maxima dura
				// 'resistenciaCarrera' segundos.
				float metrosDeResistencia = resistenciaCarrera * velocidadCarrera;
				if (metrosDeResistencia > 0.001f)
				{
					resistenciaActual -= paso / metrosDeResistencia;
					if (resistenciaActual < 0.0f) resistenciaActual = 0.0f;
				}

				playerPos.x += dx / distancia * paso;
				playerPos.z += dz / distancia * paso;

				// La calaca mira hacia donde corre (la camara ya no define su rumbo)
				yawCalaca = atan2f(dx, dz);

				dx = puntosRuta[siguientePunto].x - playerPos.x;
				dz = puntosRuta[siguientePunto].y - playerPos.z;
				distancia = sqrtf(dx * dx + dz * dz);
			}

			// Aros de resistencia: al pasar sobre uno disponible se lanza el QTE.
			// Cada aro solo se usa una vez por carrera.
			if (!qteActivo)
			{
				for (size_t i = 0; i < arosResistencia.size(); i++)
				{
					if (i < arosUsados.size() && arosUsados[i])
						continue;

					float ax = arosResistencia[i].x - playerPos.x;
					float az = arosResistencia[i].y - playerPos.z;

					if (ax * ax + az * az <= radioAro * radioAro)
					{
						arosUsados[i] = true;
						IniciarQTE();
						break;
					}
				}
			}

			// Contrarreloj del QTE: si se agota, no se gana ni se pierde nada
			if (qteActivo)
			{
				qteTiempo -= deltaTime;
				if (qteTiempo <= 0.0f)
				{
					qteTiempo = 0.0f;
					qteActivo = false;
				}
			}

			if (distancia <= radioPunto)
			{
				siguientePunto++;

				if (siguientePunto >= (int)puntosRuta.size())
				{
					// Fin de la carrera: se devuelve el control al jugador
					tiempoLlegadaJugador = relojCarrera;
					estadoCarrera = CarreraTerminada;
					audio.DetenerMusicaCarrera();
					autoCorriendo = false;
					qteActivo = false;
					tiempoMensaje = 6.0f;
					MostrarBannerFinal();
					hudTexto.clear();
					return;
				}
			}

			wchar_t buffer[128];
			swprintf_s(buffer, L"Punto %d de %d    (%.0f m)",
				siguientePunto, (int)puntosRuta.size() - 1, distancia);
			hudTexto = buffer;
			return;
		}

		if (estadoCarrera == CarreraTerminada)
		{
			tiempoMensaje -= deltaTime;

			if (tiempoMensaje <= 0.0f)
			{
				estadoCarrera = CarreraEsperando;
				siguientePunto = 1;
				hudTexto.clear();
			}
		}
	}

	// Contador del minuto de las almas, arriba al centro de la pantalla.
