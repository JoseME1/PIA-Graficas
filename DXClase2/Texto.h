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

#endif
