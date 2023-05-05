#ifndef UBTLIB_GLOBAL_H
#define UBTLIB_GLOBAL_H

#ifdef UBTLIB_EXPORTS
#define UBTLIB_DLL_EXPORT __declspec(dllexport)
#else
#define UBTLIB_DLL_EXPORT __declspec(dllimport)
#endif

#endif // UBTLIB_GLOBAL_H
