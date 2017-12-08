#pragma once
#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <Vfw.h>
DECLARE_HANDLE(HAVI);
typedef struct _AVI_INFO
{
	IAVIFile *pFile;
	INT period;
	IAVIStream *aStream;
	IAVIStream *pStream;
	IAVIStream *pStCmp;
	ULONG nFrame;
	ULONG nSample;
}AVI_INFO;
HAVI CreateAVI(TCHAR *fileName, INT period);
HRESULT AVIAddFrame(HAVI hAvi, HBITMAP hBitmap, DWORD dwRate);
HRESULT AVISetCompressionMode(HAVI hAvi, HBITMAP hBitmap, AVICOMPRESSOPTIONS *acOpt, DWORD dwRate);
HRESULT CloseAVI(HAVI hAvi);
