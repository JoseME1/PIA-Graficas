#ifndef _Texto_
#define _Texto_

#include <vector>

// Genera en 'bgra' una imagen BGRA (fila superior primero, con alfa) del texto
// centrado sobre fondo transparente, usando GDI.
// GDI no escribe el canal alfa, asi que se saca del brillo: el texto se dibuja
// en blanco sobre negro y ese valor pasa a ser el alfa.
inline void GenerarTextoBGRA(const wchar_t* texto, int ancho, int alto, int altoFuente,
	unsigned char r, unsigned char g, unsigned char b,
	std::vector<unsigned char>& bgra)
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
	UINT formato = (wcschr(texto, L'\n') != NULL)
		? (DT_CENTER | DT_TOP | DT_WORDBREAK | DT_NOPREFIX)
		: (DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

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

#endif
