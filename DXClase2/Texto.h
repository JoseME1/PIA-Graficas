#ifndef _Texto_
#define _Texto_

#include <vector>
#include <math.h>

// Genera en 'bgra' una imagen BGRA (fila superior primero, con alfa) del texto
// sobre fondo transparente, usando GDI. Por defecto el texto va centrado; con
// 'izquierda' se alinea al borde izquierdo de la imagen.
// GDI no escribe el canal alfa, asi que se saca del brillo: el texto se dibuja
// en blanco sobre negro y ese valor pasa a ser el alfa.
inline void GenerarTextoBGRA(const wchar_t* texto, int ancho, int alto, int altoFuente,
	unsigned char r, unsigned char g, unsigned char b,
	std::vector<unsigned char>& bgra, bool izquierda = false)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	if (texto == NULL || texto[0] == L'\0')
		return;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = ancho;
	bi.bmiHeader.biHeight = -alto;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);

	// GDI no escribe el alfa: el fondo se deja negro y de ahi sale el alfa 0.
	PatBlt(dc, 0, 0, ancho, alto, BLACKNESS);

	HFONT fuente = CreateFontW(-altoFuente, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	HGDIOBJ oldFuente = SelectObject(dc, fuente);

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));

	// DT_VCENTER solo funciona con una linea; para varias se alinea arriba
	UINT alineacion = izquierda ? DT_LEFT : DT_CENTER;
	UINT formato = (wcschr(texto, L'\n') != NULL)
		? (alineacion | DT_TOP | DT_WORDBREAK | DT_NOPREFIX)
		: (alineacion | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	RECT rc = { 0, 0, ancho, alto };
	DrawTextW(dc, texto, -1, &rc, formato);

	SelectObject(dc, oldFuente);
	DeleteObject(fuente);
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < ancho * alto; i++)
	{
		bgra[i * 4 + 0] = b;
		bgra[i * 4 + 1] = g;
		bgra[i * 4 + 2] = r;
		bgra[i * 4 + 3] = p[i * 4];
	}

	DeleteObject(dib);
}

// Genera en 'bgra' una flecha opaca apuntando hacia arriba (la punta queda en la
// fila superior de la imagen), centrada en un cuadrado de 'tam' x 'tam' y girada
// 'grados' en sentido horario.
inline void GenerarFlechaBGRA(int tam, float grados, unsigned char r, unsigned char g, unsigned char b,
	std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)tam * tam * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = tam;
	bi.bmiHeader.biHeight = -tam;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);

	// GDI no escribe el alfa: el fondo se deja negro y de ahi sale el alfa 0.
	PatBlt(dc, 0, 0, tam, tam, BLACKNESS);

	// Rotacion alrededor del centro de la textura
	SetGraphicsMode(dc, GM_ADVANCED);
	float rad = grados * 3.14159265f / 180.0f;
	float coseno = cosf(rad);
	float seno = sinf(rad);
	float mitad = tam * 0.5f;

	XFORM xf;
	xf.eM11 = coseno;
	xf.eM12 = seno;
	xf.eM21 = -seno;
	xf.eM22 = coseno;
	xf.eDx = mitad - (xf.eM11 * mitad + xf.eM21 * mitad);
	xf.eDy = mitad - (xf.eM12 * mitad + xf.eM22 * mitad);
	SetWorldTransform(dc, &xf);

	// Contorno de la flecha: punta arriba, asta en el centro
	POINT puntos[7] = {
		{ tam / 2,          tam / 16 },
		{ tam - tam / 16,   tam * 5 / 8 },
		{ tam * 11 / 16,    tam * 5 / 8 },
		{ tam * 11 / 16,    tam - tam / 16 },
		{ tam * 5 / 16,     tam - tam / 16 },
		{ tam * 5 / 16,     tam * 5 / 8 },
		{ tam / 16,         tam * 5 / 8 }
	};

	HBRUSH pincel = CreateSolidBrush(RGB(255, 255, 255));
	HGDIOBJ oldPincel = SelectObject(dc, pincel);
	HGDIOBJ oldPluma = SelectObject(dc, GetStockObject(NULL_PEN));
	SetBkMode(dc, TRANSPARENT);

	Polygon(dc, puntos, 7);

	SelectObject(dc, oldPluma);
	SelectObject(dc, oldPincel);
	DeleteObject(pincel);
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < tam * tam; i++)
	{
		bgra[i * 4 + 0] = b;
		bgra[i * 4 + 1] = g;
		bgra[i * 4 + 2] = r;
		bgra[i * 4 + 3] = p[i * 4];
	}

	DeleteObject(dib);
}

// Panel de estadisticas del HUD: velocidad, aceleracion y resistencia, cada una
// con su barra. Se dibuja con GDI sobre negro, igual que el texto: los pixeles
// grises (el texto) quedan blancos con el brillo como alfa, y los de color
// (las barras) conservan su color con alfa opaco.
inline void GenerarPanelBGRA(int ancho, int alto,
	float velocidad, float velocidadMax,
	float tiempoAceleracion, float intensidadAceleracion,
	float resistencia, int penalizacion,
	std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = ancho;
	bi.bmiHeader.biHeight = -alto;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);
	PatBlt(dc, 0, 0, ancho, alto, BLACKNESS);

	HFONT fuente = CreateFontW(-21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	HGDIOBJ oldFuente = SelectObject(dc, fuente);

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));

	const int margen = 16;
	const int anchoBarra = ancho - margen * 2;

	// Etiqueta o valor: una linea, alineada a la izquierda o a la derecha
	auto texto = [&](const wchar_t* t, int y, UINT alineacion)
	{
		RECT rc = { margen, y, margen + anchoBarra, y + 28 };
		DrawTextW(dc, t, -1, &rc, alineacion | DT_SINGLELINE | DT_NOPREFIX);
	};

	// Barra: pista oscura y relleno del color indicado
	auto barra = [&](int y, int altoBarra, float fraccion, COLORREF color)
	{
		HBRUSH pista = CreateSolidBrush(RGB(50, 60, 80));
		RECT r = { margen, y, margen + anchoBarra, y + altoBarra };
		FillRect(dc, &r, pista);
		DeleteObject(pista);

		if (fraccion > 1.0f) fraccion = 1.0f;
		if (fraccion < 0.0f) fraccion = 0.0f;

		int relleno = (int)(anchoBarra * fraccion);
		if (relleno > 0)
		{
			HBRUSH pincel = CreateSolidBrush(color);
			RECT f = { margen, y, margen + relleno, y + altoBarra };
			FillRect(dc, &f, pincel);
			DeleteObject(pincel);
		}
	};

	wchar_t valor[64];

	// Velocidad actual contra la maxima
	texto(L"VELOCIDAD", 0, DT_LEFT);
	swprintf_s(valor, L"%.1f / %.1f", velocidad, velocidadMax);
	texto(valor, 0, DT_RIGHT);
	barra(30, 14, (velocidadMax > 0.0f) ? velocidad / velocidadMax : 0.0f, RGB(80, 200, 255));

	// Aceleracion: barra llena mientras acelera, vacia cuando ya va a tope
	texto(L"ACELERACION", 52, DT_LEFT);
	swprintf_s(valor, L"%.1f s", tiempoAceleracion);
	texto(valor, 52, DT_RIGHT);
	barra(82, 14, intensidadAceleracion, RGB(255, 170, 60));

	// Resistencia: verde, con la penalizacion activa si la hay
	texto(L"RESISTENCIA", 104, DT_LEFT);
	if (penalizacion > 0)
		swprintf_s(valor, L"%.0f%%   -%d%%", resistencia * 100.0f, penalizacion);
	else
		swprintf_s(valor, L"%.0f%%", resistencia * 100.0f);
	texto(valor, 104, DT_RIGHT);
	barra(134, 18, resistencia, RGB(80, 230, 110));

	SelectObject(dc, oldFuente);
	DeleteObject(fuente);
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < ancho * alto; i++)
	{
		unsigned char r = p[i * 4 + 2];
		unsigned char g = p[i * 4 + 1];
		unsigned char b = p[i * 4 + 0];

		if (r == g && g == b)
		{
			// Texto: el brillo pasa a ser el alfa y el color queda blanco
			bgra[i * 4 + 0] = 255;
			bgra[i * 4 + 1] = 255;
			bgra[i * 4 + 2] = 255;
			bgra[i * 4 + 3] = r;
		}
		else
		{
			// Barras: se conserva el color
			unsigned char mayor = r;
			if (g > mayor) mayor = g;
			if (b > mayor) mayor = b;

			bgra[i * 4 + 0] = b;
			bgra[i * 4 + 1] = g;
			bgra[i * 4 + 2] = r;
			bgra[i * 4 + 3] = mayor;
		}
	}

	DeleteObject(dib);
}

// Tabla de estadisticas de las calacas rivales: 5 filas (cabecera + 4 calacas)
// y 4 columnas (nombre, velocidad, aceleracion, resistencia). Usa la misma
// paleta oscura del aviso del dia, con el fondo semitransparente.
inline void GenerarTablaBGRA(int ancho, int alto,
	const wchar_t* celdas[5][4], std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = ancho;
	bi.bmiHeader.biHeight = -alto;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);
	PatBlt(dc, 0, 0, ancho, alto, BLACKNESS);

	// Limites de las columnas y de las filas, proporcionales al tamano de la
	// tabla para que se pueda escalar sin tocar estos valores.
	const int columnas[5] = {
		0, ancho * 300 / 730, ancho * 430 / 730, ancho * 580 / 730, ancho
	};
	const int filas[6] = {
		0, alto * 56 / 264, alto * 108 / 264, alto * 160 / 264, alto * 212 / 264, alto
	};

	// Misma paleta que el aviso del dia: fondo oscuro, cabecera un poco mas
	// clara, texto blanco en la cabecera y beige en el cuerpo.
	const COLORREF fondoCabecera = RGB(38, 43, 58);
	const COLORREF fondoCuerpo = RGB(15, 18, 28);
	const COLORREF textoCabecera = RGB(255, 255, 255);
	const COLORREF textoCuerpo = RGB(230, 220, 190);

	HFONT fuenteCabecera = CreateFontW(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	HFONT fuenteCuerpo = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	HGDIOBJ oldFuente = SelectObject(dc, fuenteCuerpo);
	SetBkMode(dc, TRANSPARENT);

	for (int f = 0; f < 5; f++)
	{
		SelectObject(dc, f == 0 ? fuenteCabecera : fuenteCuerpo);

		for (int c = 0; c < 4; c++)
		{
			RECT r = { columnas[c], filas[f], columnas[c + 1], filas[f + 1] };

			HBRUSH pincel = CreateSolidBrush(f == 0 ? fondoCabecera : fondoCuerpo);
			FillRect(dc, &r, pincel);
			DeleteObject(pincel);

			if (celdas[f][c] != NULL)
			{
				SetTextColor(dc, f == 0 ? textoCabecera : textoCuerpo);
				DrawTextW(dc, celdas[f][c], -1, &r,
					DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			}
		}
	}

	// Cuadricula tenue entre celdas
	HPEN pluma = CreatePen(PS_SOLID, 2, RGB(70, 76, 92));
	HGDIOBJ oldPluma = SelectObject(dc, pluma);
	for (int c = 1; c < 4; c++)
	{
		MoveToEx(dc, columnas[c], 0, NULL);
		LineTo(dc, columnas[c], alto);
	}
	for (int f = 1; f < 5; f++)
	{
		MoveToEx(dc, 0, filas[f], NULL);
		LineTo(dc, ancho, filas[f]);
	}

	// Borde exterior beige, como el subtitulo del aviso
	HPEN borde = CreatePen(PS_SOLID, 2, RGB(230, 220, 190));
	SelectObject(dc, borde);
	HGDIOBJ oldPincel = SelectObject(dc, GetStockObject(NULL_BRUSH));
	Rectangle(dc, 1, 1, ancho - 1, alto - 1);
	SelectObject(dc, oldPincel);
	SelectObject(dc, oldPluma);
	DeleteObject(borde);
	DeleteObject(pluma);

	SelectObject(dc, oldFuente);
	DeleteObject(fuenteCabecera);
	DeleteObject(fuenteCuerpo);
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	// Como el fondo del aviso del dia: el relleno de las celdas queda
	// semitransparente y el texto y las lineas opacos.
	unsigned char* p = (unsigned char*)bits;
	for (int y = 0; y < alto; y++)
	{
		int f = 0;
		while (f < 4 && y >= filas[f + 1])
			f++;

		for (int x = 0; x < ancho; x++)
		{
			int c = 0;
			while (c < 3 && x >= columnas[c + 1])
				c++;

			COLORREF relleno = (f == 0) ? fondoCabecera : fondoCuerpo;
			int i = y * ancho + x;
			unsigned char r = p[i * 4 + 2];
			unsigned char g = p[i * 4 + 1];
			unsigned char b = p[i * 4 + 0];

			bgra[i * 4 + 0] = b;
			bgra[i * 4 + 1] = g;
			bgra[i * 4 + 2] = r;
			bgra[i * 4 + 3] = (r == GetRValue(relleno) && g == GetGValue(relleno) &&
				b == GetBValue(relleno)) ? 175 : 255;
		}
	}

	DeleteObject(dib);
}

// Ulti: anillo que se llena de fuego con una calavera roja en el centro.
// 'carga' va de 0 a 1 (fraccion del anillo encendida) y 'semilla' cambia la
// forma de las llamas para que titilen al regenerar la textura.
inline void GenerarUltiBGRA(int ancho, int alto, float carga, int semilla,
	std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = ancho;
	bi.bmiHeader.biHeight = -alto;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);

	// Magenta de clave: esos pixeles quedan transparentes al copiar
	HBRUSH clave = CreateSolidBrush(RGB(255, 0, 255));
	RECT todo = { 0, 0, ancho, alto };
	FillRect(dc, &todo, clave);
	DeleteObject(clave);

	const float cx = ancho * 0.5f;
	const float cy = alto * 0.5f;
	const float radio = ancho * 0.5f - 3.0f;
	const float grosor = ancho * 0.085f;
	const float radioMedio = radio - grosor * 0.5f;

	HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));

	// Anillo de fondo apagado
	HPEN penFondo = CreatePen(PS_SOLID, (int)grosor, RGB(30, 34, 48));
	HGDIOBJ oldPen = SelectObject(dc, penFondo);
	Ellipse(dc, (int)(cx - radioMedio), (int)(cy - radioMedio),
		(int)(cx + radioMedio), (int)(cy + radioMedio));

	// Fuego: recorre el arco desde arriba en sentido horario. La punta que
	// avanza queda amarilla y la cola roja, como si la carga se estuviera
	// quemando.
	if (carga > 0.001f)
	{
		const float dosPi = 6.2831853f;
		float anguloTotal = carga * dosPi;
		int pasos = (int)(carga * 140.0f) + 2;

		// Sin pluma: cada llama es un circulo relleno, no un aro
		HGDIOBJ oldPenFuego = SelectObject(dc, GetStockObject(NULL_PEN));

		for (int i = 0; i <= pasos; i++)
		{
			float t = (float)i / pasos;
			float angulo = -1.5707963f + t * anguloTotal;

			// Ondulacion del radio para que parezca llama
			float onda = sinf(angulo * 7.0f + semilla * 1.3f) * grosor * 0.14f
				+ sinf(angulo * 17.0f + semilla * 2.1f) * grosor * 0.07f;
			float x = cx + cosf(angulo) * (radioMedio + onda);
			float y = cy + sinf(angulo) * (radioMedio + onda);

			int r, g, b;
			if (t > 0.88f)		{ r = 255; g = 240; b = 140; }	// punta
			else if (t > 0.60f)	{ r = 255; g = 175; b = 55; }	// naranja
			else				{ r = 220; g = 65; b = 22; }	// cola

			float pulso = 0.88f + 0.12f * sinf(angulo * 11.0f + semilla * 0.9f);
			r = (int)(r * pulso);
			g = (int)(g * pulso);
			b = (int)(b * pulso);

			HBRUSH llama = CreateSolidBrush(RGB(r, g, b));
			HGDIOBJ oldB = SelectObject(dc, llama);

			float radioLlama = grosor * 0.72f;
			Ellipse(dc, (int)(x - radioLlama), (int)(y - radioLlama),
				(int)(x + radioLlama), (int)(y + radioLlama));

			SelectObject(dc, oldB);
			DeleteObject(llama);
		}

		SelectObject(dc, oldPenFuego);
	}

	// Calavera roja con contorno negro
	HBRUSH rojo = CreateSolidBrush(RGB(220, 40, 40));
	HPEN negro = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	HGDIOBJ oldB2 = SelectObject(dc, rojo);
	HGDIOBJ oldP2 = SelectObject(dc, negro);

	float cw = ancho * 0.34f;	// ancho del craneo
	float ch = alto * 0.36f;	// alto del craneo
	float arriba = cy - ch * 0.62f;
	float abajo = arriba + ch;

	Ellipse(dc, (int)(cx - cw * 0.5f), (int)arriba,
		(int)(cx + cw * 0.5f), (int)abajo);

	RoundRect(dc, (int)(cx - cw * 0.36f), (int)(abajo - ch * 0.12f),
		(int)(cx + cw * 0.36f), (int)(abajo + ch * 0.42f),
		(int)(cw * 0.25f), (int)(ch * 0.18f));

	// Cuencas, nariz y dientes en negro
	HBRUSH negroRelleno = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(dc, negroRelleno);

	float ojoW = cw * 0.30f, ojoH = ch * 0.30f;
	float ojoY = cy - ch * 0.05f;
	Ellipse(dc, (int)(cx - cw * 0.27f - ojoW * 0.5f), (int)(ojoY - ojoH * 0.5f),
		(int)(cx - cw * 0.27f + ojoW * 0.5f), (int)(ojoY + ojoH * 0.5f));
	Ellipse(dc, (int)(cx + cw * 0.27f - ojoW * 0.5f), (int)(ojoY - ojoH * 0.5f),
		(int)(cx + cw * 0.27f + ojoW * 0.5f), (int)(ojoY + ojoH * 0.5f));

	POINT nariz[3] = {
		{ (LONG)cx, (LONG)(cy + ch * 0.02f) },
		{ (LONG)(cx - cw * 0.09f), (LONG)(cy + ch * 0.20f) },
		{ (LONG)(cx + cw * 0.09f), (LONG)(cy + ch * 0.20f) }
	};
	Polygon(dc, nariz, 3);

	for (int i = -2; i <= 2; i++)
	{
		float x = cx + i * cw * 0.075f;
		MoveToEx(dc, (int)x, (int)(abajo - 2.0f), NULL);
		LineTo(dc, (int)x, (int)(abajo + ch * 0.28f));
	}

	SelectObject(dc, oldB2);
	SelectObject(dc, oldP2);
	DeleteObject(rojo);
	DeleteObject(negro);
	DeleteObject(negroRelleno);

	SelectObject(dc, oldBrush);
	SelectObject(dc, oldPen);
	DeleteObject(penFondo);

	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	// Copia con alfa: magenta = fondo transparente, anillo apagado translucido
	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < ancho * alto; i++)
	{
		unsigned char b = p[i * 4 + 0];
		unsigned char g = p[i * 4 + 1];
		unsigned char r = p[i * 4 + 2];

		bgra[i * 4 + 0] = b;
		bgra[i * 4 + 1] = g;
		bgra[i * 4 + 2] = r;

		if (r == 255 && g == 0 && b == 255)
			bgra[i * 4 + 3] = 0;
		else if (r == 30 && g == 34 && b == 48)
			bgra[i * 4 + 3] = 180;
		else
			bgra[i * 4 + 3] = 255;
	}

	DeleteObject(dib);
}

// Tecla de espacio que avisa que la ulti esta lista.
inline void GenerarTeclaEspacioBGRA(int ancho, int alto, std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = ancho;
	bi.bmiHeader.biHeight = -alto;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);

	HBRUSH clave = CreateSolidBrush(RGB(255, 0, 255));
	RECT todo = { 0, 0, ancho, alto };
	FillRect(dc, &todo, clave);
	DeleteObject(clave);

	HBRUSH fondo = CreateSolidBrush(RGB(15, 18, 28));
	HPEN borde = CreatePen(PS_SOLID, 4, RGB(230, 220, 190));
	HGDIOBJ oldB = SelectObject(dc, fondo);
	HGDIOBJ oldP = SelectObject(dc, borde);

	RoundRect(dc, 3, 3, ancho - 3, alto - 3, 26, 26);

	HFONT fuente = CreateFontW(-30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	HGDIOBJ oldFuente = SelectObject(dc, fuente);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));

	RECT r = { 0, 0, ancho, alto };
	DrawTextW(dc, L"ESPACIO", -1, &r,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	SelectObject(dc, oldFuente);
	DeleteObject(fuente);
	SelectObject(dc, oldB);
	SelectObject(dc, oldP);
	DeleteObject(fondo);
	DeleteObject(borde);

	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < ancho * alto; i++)
	{
		unsigned char b = p[i * 4 + 0];
		unsigned char g = p[i * 4 + 1];
		unsigned char r = p[i * 4 + 2];

		bgra[i * 4 + 0] = b;
		bgra[i * 4 + 1] = g;
		bgra[i * 4 + 2] = r;

		if (r == 255 && g == 0 && b == 255)
			bgra[i * 4 + 3] = 0;
		else if (r == 15 && g == 18 && b == 28)
			bgra[i * 4 + 3] = 205;
		else
			bgra[i * 4 + 3] = 255;
	}

	DeleteObject(dib);
}

// Genera en 'bgra' un alma: una llama vertical blanca/dorada con un halo suave,
// pensada para dibujarse como billboard con blending alfa. 'semilla' cambia la
// forma para que cada alma se vea un poco distinta.
inline void GenerarAlmaBGRA(int ancho, int alto, float semilla, std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)ancho * alto * 4, 0);

	for (int y = 0; y < alto; y++)
	{
		float v = (y + 0.5f) / (float)alto;		// 0 = punta, 1 = base

		// La llama se afina en la punta y se apaga en la base
		float punta = (v < 0.14f) ? (v / 0.14f) : 1.0f;
		float base = (v > 0.88f) ? (1.0f - (v - 0.88f) / 0.12f) : 1.0f;
		float cuerpo = punta * base;

		// Perfil de gota: ancha abajo y angosta arriba, con una leve ondulacion
		float radio = 0.10f + 0.62f * powf(v, 0.75f);
		float centro = 0.09f * sinf(v * 6.5f + semilla) * (1.0f - v);

		for (int x = 0; x < ancho; x++)
		{
			float u = ((x + 0.5f) / (float)ancho) * 2.0f - 1.0f;
			float d = (u - centro) / radio;

			// Nucleo blanco + halo dorado ancho
			float nucleo = expf(-d * d * 3.0f);
			float halo = expf(-d * d * 0.55f) * 0.55f;
			float brillo = (nucleo + halo) * cuerpo;

			// Ruido fijo para que el borde no sea perfecto
			brillo *= 0.90f + 0.10f * sinf(x * 12.9898f + y * 78.233f + semilla * 43.7f);

			if (brillo <= 0.004f)
				continue;

			float t = (brillo > 1.0f) ? 1.0f : brillo;

			// Blanco en el centro, dorado hacia el borde
			float k = (nucleo * cuerpo > 1.0f) ? 1.0f : nucleo * cuerpo;

			size_t i = ((size_t)y * ancho + x) * 4;
			bgra[i + 0] = (unsigned char)(110.0f + 145.0f * k + 0.5f);
			bgra[i + 1] = (unsigned char)(198.0f + 57.0f * k + 0.5f);
			bgra[i + 2] = 255;
			bgra[i + 3] = (unsigned char)(t * 255.0f + 0.5f);
		}
	}
}

// Circulo relleno con borde suave, para el boton de la barra de volumen.
inline void GenerarCirculoBGRA(int diametro, unsigned char r, unsigned char g, unsigned char b,
	std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)diametro * diametro * 4, 0);

	float centro = (diametro - 1) * 0.5f;
	float radio = diametro * 0.5f;

	for (int y = 0; y < diametro; y++)
	{
		for (int x = 0; x < diametro; x++)
		{
			float dx = x - centro;
			float dy = y - centro;
			float d = sqrtf(dx * dx + dy * dy);
			if (d > radio)
				continue;

			float alfa = 1.0f;
			if (d > radio - 1.5f)
				alfa = (radio - d) / 1.5f;

			size_t i = ((size_t)y * diametro + x) * 4;
			bgra[i + 0] = b;
			bgra[i + 1] = g;
			bgra[i + 2] = r;
			bgra[i + 3] = (unsigned char)(alfa * 255.0f + 0.5f);
		}
	}
}

// Cursor del menu: flecha blanca con contorno negro sobre fondo transparente.
inline void GenerarCursorBGRA(int tam, std::vector<unsigned char>& bgra)
{
	bgra.assign((size_t)tam * tam * 4, 0);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = tam;
	bi.bmiHeader.biHeight = -tam;	// negativo = fila superior primero
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (dib == NULL || bits == NULL)
		return;

	HDC dc = CreateCompatibleDC(NULL);
	HGDIOBJ oldBmp = SelectObject(dc, dib);

	// Magenta de clave: esos pixeles quedan transparentes al copiar
	HBRUSH clave = CreateSolidBrush(RGB(255, 0, 255));
	RECT todo = { 0, 0, tam, tam };
	FillRect(dc, &todo, clave);
	DeleteObject(clave);

	// Flecha clasica con la punta arriba a la izquierda
	float s = tam / 20.0f;
	POINT puntos[7] = {
		{ (LONG)(1 * s),  (LONG)(1 * s) },
		{ (LONG)(1 * s),  (LONG)(17 * s) },
		{ (LONG)(5 * s),  (LONG)(13 * s) },
		{ (LONG)(8 * s),  (LONG)(20 * s) },
		{ (LONG)(11 * s), (LONG)(18 * s) },
		{ (LONG)(8 * s),  (LONG)(11 * s) },
		{ (LONG)(13 * s), (LONG)(11 * s) }
	};

	HBRUSH blanco = CreateSolidBrush(RGB(255, 255, 255));
	HPEN negro = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	HGDIOBJ oldB = SelectObject(dc, blanco);
	HGDIOBJ oldP = SelectObject(dc, negro);
	SetBkMode(dc, TRANSPARENT);
	Polygon(dc, puntos, 7);

	SelectObject(dc, oldB);
	SelectObject(dc, oldP);
	DeleteObject(blanco);
	DeleteObject(negro);
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	unsigned char* p = (unsigned char*)bits;
	for (int i = 0; i < tam * tam; i++)
	{
		unsigned char b = p[i * 4 + 0];
		unsigned char g = p[i * 4 + 1];
		unsigned char r = p[i * 4 + 2];

		bgra[i * 4 + 0] = b;
		bgra[i * 4 + 1] = g;
		bgra[i * 4 + 2] = r;
		bgra[i * 4 + 3] = (r == 255 && g == 0 && b == 255) ? 0 : 255;
	}

	DeleteObject(dib);
}

#endif
