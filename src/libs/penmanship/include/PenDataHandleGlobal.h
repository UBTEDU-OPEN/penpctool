#ifndef PEN_DATA_HANDLE_GLOBAL_H
#define PEN_DATA_HANDLE_GLOBAL_H


#ifdef _WIN32
#ifdef PEN_DATA_HANDLE_LIB
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#endif

#endif // !PEN_DATA_HANDLE_GLOBAL_H
