#pragma once

#ifdef _WIN32
# ifdef engine_EXPORTS
#   define engine_API  __declspec( dllexport )
# else
#   define engine_API  __declspec( dllimport )
# endif
#else
# define engine_API
#endif