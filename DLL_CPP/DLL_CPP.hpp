#ifdef DLLCPP_EXPORTS
#define DLLCPP_API __declspec(dllexport)
#else
#define DLLCPP_API __declspec(dllimport)
#endif
//
// // Ta klasa została wyeksportowana z pliku dll
// class DLLCPP_API CDLLCPP {
// public:
// 	CDLLCPP(void);
// 	// TODO: w tym miejscu dodaj metody.
// };
//
// extern DLLCPP_API int nDLLCPP;
//
// DLLCPP_API int fnDLLCPP(void);

extern "C" {

DLLCPP_API bool _cpp_calculate_histogram(unsigned __int32* histogram,
                                         const unsigned __int8* pixels,
                                         unsigned __int32 number_of_pixels);

}
