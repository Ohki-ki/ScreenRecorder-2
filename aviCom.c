#include "aviCom.h"

HRESULT CloseAVI(HAVI hAvi)
{
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	if (hAvi == NULL || hAvi == INVALID_HANDLE_VALUE) return AVIERR_BADHANDLE;
	
	if (ai->aStream) AVIStreamRelease(ai->aStream);
	if (ai->pStCmp) AVIStreamRelease(ai->pStCmp);
	if (ai->pStream) AVIStreamRelease(ai->pStream);
	if (ai->pFile) AVIFileRelease(ai->pFile);
	AVIFileExit();
	free(ai);
	return S_OK;
}

HAVI CreateAVI(TCHAR *fileName, INT period)
{
	IAVIFile *pFile;
	HRESULT hr;
	AVI_INFO *ai = malloc(sizeof(AVI_INFO));

	AVIFileInit();
	hr = AVIFileOpen(&pFile, fileName, OF_WRITE | OF_CREATE, NULL);
	if (hr != AVIERR_OK)
	{
		AVIFileExit();
		return (HAVI)INVALID_HANDLE_VALUE;
	}
	ai->pFile = pFile;

	ai->period = period;
	ai->aStream = NULL;
	ai->pStream = NULL;
	ai->pStCmp = NULL;
	ai->nFrame = 0;
	ai->nSample = 0;
	return (HAVI)ai;
}

HRESULT AVIAddFrame(HAVI hAvi, HBITMAP hBitmap, DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObject(hBitmap, sizeof(dbs), &dbs);
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	HRESULT hr;

	if (ai->pStream == NULL)
	{
		AVISTREAMINFO asInfo;
		//SecureZeroMemory(&asInfo, sizeof(AVISTREAMINFO));
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFO));
		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = 0;
		//asInfo.dwScale = ai->period;
		asInfo.dwRate = (DWORD)1000;
		asInfo.dwScale = dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStream(ai->pFile, &ai->pStream, &asInfo);
		if (hr != AVIERR_OK) return hr;
	}

	if (ai->pStCmp == NULL)
	{
		AVICOMPRESSOPTIONS acOpt;
		HRESULT hr;
		//SecureZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		ZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		acOpt.fccHandler = mmioFOURCC('C', 'V', 'I', 'D');
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, &acOpt, NULL);
		if (hr != AVIERR_OK) return hr;
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed*sizeof(RGBQUAD));
		if (hr != AVIERR_OK) return hr;
	}

	hr = AVIStreamWrite(ai->pStCmp, ai->nFrame, 1, dbs.dsBm.bmBits, dbs.dsBmih.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
	if (hr != AVIERR_OK) return hr;
	ai->nFrame++;

	return S_OK;
}

HRESULT AVISetCompressionMode(HAVI hAvi, HBITMAP hBitmap, AVICOMPRESSOPTIONS *acOpt, DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObject(hBitmap, sizeof(dbs), &dbs);
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	HRESULT hr;

	if (ai->pStream == NULL)
	{
		AVISTREAMINFO asInfo;
		//SecureZeroMemory(&asInfo, sizeof(AVISTREAMINFO));
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFO));
		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = 0;
		//asInfo.dwScale = ai->period;
		asInfo.dwRate = 1000;//24
		asInfo.dwScale = (DWORD)dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStream(ai->pFile, &ai->pStream, &asInfo);
		if (hr != AVIERR_OK)
		{
			return hr;
		}
	}

	if (ai->pStCmp != NULL) return AVIERR_COMPRESSOR;
	else
	{
		AVICOMPRESSOPTIONS newOpt, *farOpt[1];
		//SecureZeroMemory(&newOpt, sizeof(AVICOMPRESSOPTIONS));
		ZeroMemory(&newOpt, sizeof(AVICOMPRESSOPTIONS));
		if (acOpt != NULL)
		{
			farOpt[0] = acOpt;
		}
		else
		{
			farOpt[0] = &newOpt;
		}
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, farOpt[0], NULL);
		AVISaveOptionsFree(1, farOpt);
		if (hr != AVIERR_OK)
		{
			return hr;
		}
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed*sizeof(RGBQUAD));
		if (hr != AVIERR_OK)
		{
			return hr;
		}
	}

	return AVIERR_OK;
}