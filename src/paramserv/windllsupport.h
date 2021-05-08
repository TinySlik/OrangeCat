#ifndef WINDLLSUPPORT_H
#define WINDLLSUPPORT_H

#ifdef OS_WIN
#ifdef _WIN_EXPORTING
#define CLASS_DECLSPEC __declspec(dllexport)
#else
#define CLASS_DECLSPEC __declspec(dllimport)
#endif
#else
#define CLASS_DECLSPEC  
#endif

#endif // WINDLLSUPPORT_H
