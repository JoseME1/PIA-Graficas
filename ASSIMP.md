# Prueba de Assimp

Esta rama prepara la importacion de modelos glTF 2.0 (`.gltf` y `.glb`) y FBX (`.fbx`) en
`Debug | Win32`. Los modelos TXT existentes se siguen cargando igual.

## Preparacion

Con Git, CMake y Visual Studio con el toolset v143 instalados, ejecutar desde
la raiz del repositorio:

```powershell
./preparar_assimp.ps1
```

El script descarga Assimp 5.4.3 y lo compila para Win32. La fuente y los
archivos generados quedan en `external/` y no se incluyen en Git. Despues,
abrir `DXClase2.sln` y compilar `Debug | Win32`.

`ModeloAssimp` conserva la escena importada, incluidas sus mallas, huesos y
animaciones, mientras exista el objeto. Una nueva carga invalida los punteros
obtenidos de la escena anterior. La calaca de prueba reproduce la primera
animacion de su primera malla; el renderizador aun no importa los materiales
del archivo. Cada modelo puede necesitar ajustes de escala y orientacion.
