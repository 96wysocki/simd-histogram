#define WIN32_LEAN_AND_MEAN

#include <SDKDDKVer.h>
#include <windows.h>

#include <minwindef.h>
#include <winnt.h>

extern "C" const unsigned __int32 k_max_number_of_pixels = (1 << 30);

    BOOL APIENTRY
    DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
