#ifdef _WIN32
# ifdef logistics_EXPORTS
#   define logistics_API  __declspec( dllexport )
# else
#   define logistics_API  __declspec( dllimport )
# endif
#else
# define logistics_API
#endif