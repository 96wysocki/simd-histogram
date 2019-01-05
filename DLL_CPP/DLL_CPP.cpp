#include "DLL_CPP.hpp"
#include <cstring>

DLLCPP_API
bool _cpp_calculate_histogram(unsigned __int32* histogram,
                              const unsigned __int8* pixels,
                              unsigned __int32 number_of_pixels) {
	if (!number_of_pixels) { return false; }
	memset(histogram, 0, 256 * sizeof(unsigned __int32));
	for (unsigned __int32 pixel_index{0}; pixel_index < number_of_pixels; ++pixel_index) {
		++histogram[pixels[pixel_index]];
	}
	return true;
}

//
// // To jest przykład wyeksportowanej zmiennej
// DLLCPP_API int nDLLCPP=0;
//
// // To jest przykład wyeksportowanej funkcji.
// DLLCPP_API int fnDLLCPP(void)
// {
//     return 42;
// }
//
// // To jest konstruktor wyeksportowanej klasy.
// CDLLCPP::CDLLCPP()
// {
//     return;
// }
