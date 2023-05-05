#ifndef PORTLIB_GLOBAL_H
#define PORTLIB_GLOBAL_H

#ifdef PORTLIB_EXPORTS
#define PORTLIB_DLL_EXPORT __declspec(dllexport)
#else
#define PORTLIB_DLL_EXPORT __declspec(dllimport)
#endif

#endif // PORTLIB_GLOBAL_H
