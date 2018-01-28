#pragma once

#if defined(XRDXT_EXPORTS) || defined(DXT_EXPORTS)
#	define XRDXT_API extern "C" __declspec(dllexport)
#	define XRDXT_CALL __stdcall
#else
#	define XRDXT_API extern "C" __declspec(dllimport)
#	pragma comment(lib, "dxt.lib")
#	define XRDXT_CALL __stdcall
#endif

#ifndef ECORE_API
#	define ECORE_API
#endif // !ECORE_API


#include <xrCore.h>
#include "ETextureParams.h"

/**
* Compress image
*
* Do NOT use directly, use DXTCompress instead!
*/
INT XRDXT_CALL DXTCompressImage(LPCSTR out_name, u8 * raw_data, u32 w, u32 h, u32 pitch, STextureParams * fmt, u32 depth);

/**
* Compress bump texture
*
* Do NOT use directly, use DXTCompress instead!
*/
INT XRDXT_CALL DXTCompressBump(LPCSTR out_name, u8 * raw_data, u8 * normal_map, u32 w, u32 h, u32 pitch, STextureParams  * fmt, u32 depth);

/**
* Compress raw data and save output to file.
*
* @param	out_name	Destination file name
* @param	raw_data	Source RGBA (?) raster data
* @param	normal_map	- (?)
* @param	w			Source width
* @param	h			Source height
* @param	pitch		Source stride (?)
* @param	fmt			pointer to STextureParams struct containing destination format definition
* @param	depth		- (?)
*/
XRDXT_API INT XRDXT_CALL DXTCompress(LPCSTR out_name, u8 * raw_data, u8 * normal_map, u32 w, u32 h, u32 pitch, STextureParams * fmt, u32 depth);