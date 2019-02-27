#ifndef PROTON_IMPERATIVE_EXPORTS_CONFIG_HPP
#define PROTON_IMPERATIVE_EXPORTS_CONFIG_HPP

// This file defines macros to tell Windows compiler
// that a class must be exported from a DLL
// or imported to a DLL

#ifdef _MSC_VER
// export if this is our own source, otherwise import:
#ifdef PROTON_IMPERATIVE_EXPORTS
# define PROTON_IMPERATIVE_API __declspec(dllexport)
#else
# define PROTON_IMPERATIVE_API __declspec(dllimport)
#endif  // PROTON_IMPERATIVE_EXPORTS

#endif

// if PROTON_IMPERATIVE_API isn't defined yet define it now:
#ifndef PROTON_IMPERATIVE_API
#define PROTON_IMPERATIVE_API
#endif

#endif
