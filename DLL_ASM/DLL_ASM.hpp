#pragma once

#ifdef DLLASM_EXPORTS
#define DLLASM_API __declspec(dllexport)
#else
#define DLLASM_API __declspec(dllimport)
#endif