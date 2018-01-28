// DXT.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4018)
//#include "dxtlib.h"
#pragma warning(pop)
#include "DXT.h"
#include "ETextureParams.h"
#include "dds.h"
#include <ddraw.h>

#include <nvtt\nvtt.h>

BOOL APIENTRY DllMain(HANDLE hModule, u32 ul_reason_for_call, LPVOID lpReserved) {
	return TRUE;
}

const u32 fcc_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
const u32 fcc_DXT2 = MAKEFOURCC('D', 'X', 'T', '2');
const u32 fcc_DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
const u32 fcc_DXT4 = MAKEFOURCC('D', 'X', 'T', '4');
const u32 fcc_DXT5 = MAKEFOURCC('D', 'X', 'T', '5');

ENGINE_API u32* Build32MipLevel(u32 &_w, u32 &_h, u32 &_p, u32 *pdwPixelSrc, STextureParams* fmt, FLOAT blend) {
	R_ASSERT(pdwPixelSrc);
	R_ASSERT((_w % 2) == 0);
	R_ASSERT((_h % 2) == 0);
	R_ASSERT((_p % 4) == 0);

	u32	dwDestPitch = (_w / 2) * 4;
	u32*	pNewData = xr_alloc<u32>((_h / 2)*dwDestPitch);
	u8*		pDest = (u8 *)pNewData;
	u8*		pSrc = (u8 *)pdwPixelSrc;

	float	mixed_a = (float)u8(fmt->fade_color >> 24);
	float	mixed_r = (float)u8(fmt->fade_color >> 16);
	float	mixed_g = (float)u8(fmt->fade_color >> 8);
	float	mixed_b = (float)u8(fmt->fade_color >> 0);

	float	inv_blend = 1.f - blend;
	for (register u32 y = 0; y < _h; y += 2){
		u8* pScanline = pSrc + y*_p;
		for (register u32 x = 0; x < _w; x += 2){
			u8*	p1 = pScanline + x * 4;
			u8*	p2 = p1 + 4;					if (1 == _w)	p2 = p1;
			u8*	p3 = p1 + _p;				if (1 == _h)  p3 = p1;
			u8*	p4 = p2 + _p;				if (1 == _h)  p4 = p2;
			float	c_r = float(u32(p1[0]) + u32(p2[0]) + u32(p3[0]) + u32(p4[0])) / 4.f;
			float	c_g = float(u32(p1[1]) + u32(p2[1]) + u32(p3[1]) + u32(p4[1])) / 4.f;
			float	c_b = float(u32(p1[2]) + u32(p2[2]) + u32(p3[2]) + u32(p4[2])) / 4.f;
			float	c_a = float(u32(p1[3]) + u32(p2[3]) + u32(p3[3]) + u32(p4[3])) / 4.f;

			if (fmt->flags.is(STextureParams::flFadeToColor)){
				c_r = c_r*inv_blend + mixed_r*blend;
				c_g = c_g*inv_blend + mixed_g*blend;
				c_b = c_b*inv_blend + mixed_b*blend;
			}
			if (fmt->flags.is(STextureParams::flFadeToAlpha)){
				c_a = c_a*inv_blend + mixed_a*blend;
			}

			float	A = (c_a + c_a / 8.f);
			int _r = int(c_r);	clamp(_r, 0, 255);	*pDest++ = u8(_r);
			int _g = int(c_g);	clamp(_g, 0, 255);	*pDest++ = u8(_g);
			int _b = int(c_b);	clamp(_b, 0, 255);	*pDest++ = u8(_b);
			int _a = int(A);	clamp(_a, 0, 255);	*pDest++ = u8(_a);
		}
	}
	_w /= 2; _h /= 2; _p = _w * 4;
	return pNewData;
}

/*
bool IsThisNumberPowerOfTwo(unsigned int n) {
return (((n & (n-1)) == 0) && (n != 0));
}
*/

IC u32 GetPowerOf2Plus1(u32 v) {
	u32 cnt = 0;
	while (v) { v >>= 1; cnt++; };
	return cnt;
}

void FillRect(u8* data, u8* new_data, u32 offs, u32 pitch, u32 h, u32 full_pitch){
	for (u32 i = 0; i < h; i++) {
		CopyMemory(data + (full_pitch * i + offs), new_data + i * pitch, pitch);
	}
}

INT XRDXT_CALL DXTCompressImage(LPCSTR out_name, u8* raw_data, u32 w, u32 h, u32 pitch, STextureParams* fmt, u32 depth) {

	R_ASSERT((0 != w) && (0 != h));

	nvtt::CompressionOptions	nvCoOpt;
	nvtt::InputOptions			nvInOpt;
	nvtt::OutputOptions			nvOuOpt;

	nvOuOpt.setFileName(out_name);
	nvInOpt.setMipmapGeneration(!!(fmt->flags.is(STextureParams::flGenerateMipMaps)));
	nvInOpt.setTextureLayout(fmt->type == STextureParams::ttCubeMap ? nvtt::TextureType_Cube : nvtt::TextureType_2D, w, h);

	nvCoOpt.setQuantization(!!(fmt->flags.is(STextureParams::flDitherColor)), false, !!(fmt->flags.is(STextureParams::flBinaryAlpha)));

	switch (fmt->fmt) {
		//DXT Compression:
	case STextureParams::tfDXT1: 	nvCoOpt.setFormat(nvtt::Format_DXT1);	break;
	case STextureParams::tfADXT1: 	nvCoOpt.setFormat(nvtt::Format_DXT1a);	break;
	case STextureParams::tfDXT3: 	nvCoOpt.setFormat(nvtt::Format_DXT3);	break;
	case STextureParams::tfDXT5: 	nvCoOpt.setFormat(nvtt::Format_DXT5);	break;

		// Uncompressed:
	default:
		nvCoOpt.setFormat(nvtt::Format_RGBA);
		switch (fmt->fmt) {
		case STextureParams::tfRGBA:	nvCoOpt.setPixelFormat(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);	break;
		case STextureParams::tfRGB:		nvCoOpt.setPixelFormat(24, 0x0000ff, 0x00ff00, 0xff0000, 0);				break;
		case STextureParams::tf4444:	nvCoOpt.setPixelFormat(16, 0x00f0, 0x0f00, 0xf000, 0x000f);					break;
		case STextureParams::tf1555:	nvCoOpt.setPixelFormat(16, 0x001f, 0x03e0, 0x7c00, 0x8000);					break;
		case STextureParams::tf565:		nvCoOpt.setPixelFormat(16, 0x001f, 0x07e0, 0xf800, 0);						break;

		case STextureParams::tfA8: 		nvCoOpt.setQuantization(false, true, !!(fmt->flags.is(STextureParams::flBinaryAlpha)));	break;
		case STextureParams::tfL8: 		nvCoOpt.setQuantization(true, false, !!(fmt->flags.is(STextureParams::flBinaryAlpha))); break;
		case STextureParams::tfA8L8: 	nvCoOpt.setQuantization(true, true, !!(fmt->flags.is(STextureParams::flBinaryAlpha))); break;

		default:
			NODEFAULT;
		}
		break;
	}

	// MipMap Filter:
	switch (fmt->mip_filter) {
	case STextureParams::kMIPFilterBox:			nvInOpt.setMipmapFilter(nvtt::MipmapFilter_Box);		break;
	case STextureParams::kMIPFilterKaiser:		nvInOpt.setMipmapFilter(nvtt::MipmapFilter_Kaiser);		break;
	case STextureParams::kMIPFilterTriangle:	nvInOpt.setMipmapFilter(nvtt::MipmapFilter_Triangle);	break;
	case STextureParams::kMIPFilterAdvanced:	break;
	default:
		nvInOpt.setMipmapFilter(nvtt::MipmapFilter_Box);
	}


	//-------------------
	if ((fmt->flags.is(STextureParams::flGenerateMipMaps)) && (STextureParams::kMIPFilterAdvanced == fmt->mip_filter)) {
		//nvOpt.MipMapType	= dUseExistingMipMaps;
		//nvInOpt.setMipmapGeneration(false);

		//u8* pImagePixels	= 0;
		int numMipmaps = GetPowerOf2Plus1(__min(w, h));
		u32 line_pitch = w * 2 * 4;
		//pImagePixels		= xr_alloc<u8>(line_pitch*h);
		u32 w_offs = 0;
		u32 dwW = w;
		u32 dwH = h;
		u32 dwP = pitch;
		u32* pLastMip = xr_alloc<u32>(w*h * 4);
		CopyMemory(pLastMip, raw_data, w*h * 4);
		//FillRect			(pImagePixels,(u8*)pLastMip,w_offs,pitch,dwH,line_pitch);
		w_offs += dwP;
		R_CHK2(nvInOpt.setMipmapData(pLastMip, dwW, dwH, 1, 0, 0), "Failed to set mipmap data.");

		float	inv_fade = clampr(1.f - float(fmt->fade_amount) / 100.f, 0.f, 1.f);
		float	blend = fmt->flags.is_any(STextureParams::flFadeToColor | STextureParams::flFadeToAlpha) ? inv_fade : 1.f;
		for (int i = 1; i<numMipmaps; i++){
			u32* pNewMip = Build32MipLevel(dwW, dwH, dwP, pLastMip, fmt, i<fmt->fade_delay ? 0.f : 1.f - blend);
			//FillRect		(pImagePixels,(u8*)pNewMip,w_offs,dwP,dwH,line_pitch);
			xr_free(pLastMip);
			pLastMip = pNewMip;
			pNewMip = 0;
			w_offs += dwP;
			R_CHK2(nvInOpt.setMipmapData(pLastMip, dwW, dwH, 1, 0, i), "Failed to set mipmap data.");
		}
		xr_free(pLastMip);

		//nvOpt.bFadeColor	= false;
		//nvOpt.bFadeAlpha	= false;
	}
	else {
		R_CHK2(nvInOpt.setMipmapData(raw_data, w, h), "Failed to set image data.");
	}

	nvtt::Compressor nvCompressor;
	bool result = nvCompressor.process(nvInOpt, nvCoOpt, nvOuOpt);

	R_ASSERT2(result == true, "Failed to compress image!");

	//_close					(gFileOut);
	if (result == false){
		/*
		switch (hr){
		case DXTERR_INPUT_POINTER_ZERO:			MessageBox(0,"Empty source.","DXT compress error",MB_ICONERROR|MB_OK);			break;
		case DXTERR_DEPTH_IS_NOT_3_OR_4:		MessageBox(0,"Source depth is not 3 or 4.","DXT compress error",MB_ICONERROR|MB_OK);			break;
		case DXTERR_NON_POWER_2:				MessageBox(0,"Source size non power 2.","DXT compress error",MB_ICONERROR|MB_OK);					break;
		case DXTERR_INCORRECT_NUMBER_OF_PLANES:	MessageBox(0,"Source incorrect number of planes.","DXT compress error",MB_ICONERROR|MB_OK);	break;
		case DXTERR_NON_MUL4:					MessageBox(0,"Source size non mul 4.","DXT compress error",MB_ICONERROR|MB_OK);						break;
		}
		*/
		unlink(out_name);
		return 0;
	}
	else {
		return 1;
	}
}


INT XRDXT_CALL DXTCompress(LPCSTR out_name, u8* raw_data, u8* normal_map, u32 w, u32 h, u32 pitch, STextureParams* fmt, u32 depth) {
	switch (fmt->type) {
	case STextureParams::ttImage:
	case STextureParams::ttCubeMap:
	case STextureParams::ttNormalMap:
	case STextureParams::ttTerrain:
		return DXTCompressImage(out_name, raw_data, w, h, pitch, fmt, depth);
		break;
	case STextureParams::ttBumpMap:
		return DXTCompressBump(out_name, raw_data, normal_map, w, h, pitch, fmt, depth);
		break;
	default:
		NODEFAULT;
	}
	return -1;
}

