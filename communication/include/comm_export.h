#pragma once

#ifdef _WIN32
# ifdef communication_EXPORTS
#   define communication_API  __declspec( dllexport )
# else
#   define communication_API  __declspec( dllimport )
# endif
#else
# define engine_API
#endif