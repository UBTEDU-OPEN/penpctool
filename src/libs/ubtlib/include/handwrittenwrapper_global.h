#ifndef HAND_WRITTEN_WRAPPER_GLOBAL_H
#define HAND_WRITTEN_WRAPPER_GLOBAL_H

#ifdef HAND_WRITTEN_WRAPPER_EXPORTS
#define HAND_WRITTEN_WRAPPER_DLL_EXPORT __declspec(dllexport)
#else
#define HAND_WRITTEN_WRAPPER_DLL_EXPORT __declspec(dllimport)
#endif

#endif // HAND_WRITTEN_WRAPPER_GLOBAL_H
