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

#endif
