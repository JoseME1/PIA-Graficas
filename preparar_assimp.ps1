$ErrorActionPreference = 'Stop'
$raiz = $PSScriptRoot
$fuente = Join-Path $raiz 'external/assimp'
$build = Join-Path $raiz 'external/assimp-build'

if (-not (Test-Path (Join-Path $fuente 'CMakeLists.txt'))) {
    git clone --depth 1 --branch v5.4.3 https://github.com/assimp/assimp.git $fuente
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo descargar Assimp.' }
}

cmake -S $fuente -B $build -A Win32 -T v143 `
    -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF `
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_INSTALL=OFF `
    -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_GLTF_IMPORTER=ON -DASSIMP_BUILD_FBX_IMPORTER=ON `
    -DASSIMP_BUILD_ZLIB=ON -DASSIMP_NO_EXPORT=ON `
    -DCMAKE_POLICY_DEFAULT_CMP0141=NEW -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded `
    '-DCMAKE_C_FLAGS_DEBUG=/MDd /Od /Z7' '-DCMAKE_CXX_FLAGS_DEBUG=/MDd /Od /Z7'
if ($LASTEXITCODE -ne 0) { throw 'No se pudo configurar Assimp.' }
cmake --build $build --config Debug --target assimp --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'No se pudo compilar Assimp.' }
