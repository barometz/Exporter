// DllWithExports.cpp : Defines the exported functions for the DLL application.
//

#include "DllWithExports.h"

API_FUNCTION WithExports* WithExports::Create(int)
{
  return new WithExports();
}

API_FUNCTION void WithExports::Destroy(WithExports* instance)
{
  delete instance;
}
