#pragma once

#ifdef DLLWITHEXPORTS_EXPORTS
#define API_FUNCTION __declspec(dllexport)
#else
#define API_FUNCTION __declspec(dllimport)
#endif

class WithExports
{
  API_FUNCTION static WithExports* Create(int);
  API_FUNCTION static void Destroy(WithExports*);
};
