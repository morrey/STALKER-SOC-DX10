#pragma once
#include <windows.h>

#define COLORPICKER_API				__declspec(dllexport)
#define COLORPICKER_COLOR_DEFAULT	0x80000000

/**
 * @param	currentColor
 * @param	originalColor
 * @param	initialExpansionState:
 *				0 - use defaults
 *				1 - force expanded    show expansion button
 *				2 - force expanded    hide the expansion button
 *				3 - force compressed  show expansion button
 *				4 - force compressed  hide expansion button
 * @return	true = "OK", false = "Cancel"
 */
extern COLORPICKER_API bool WINAPI FSColorPickerDoModal(LPDWORD currentColor, LPDWORD originalColor, const INT initialExpansionState);