#pragma once

#ifdef _WIN32
# ifdef thread_pool_EXPORTS
#   define thread_pool_API  __declspec( dllexport )
# else
#   define thread_pool_API  __declspec( dllimport )
# endif
#else
# define thread_pool_API
#endif