#pragma once

#ifdef _WIN32
# ifdef geometry_EXPORTS
#   define geometry_API  __declspec( dllexport )
# else
#   define geometry_API  __declspec( dllimport )
# endif
#else
# define geometry_API
#endif