// UnitTests.cpp : Defines the entry point for the console application.
//

#include "../Exporter/Exporter.h"

#include "gtest/gtest.h"

int main()
{
  GetDllExports("Hi.txt");
  return 0;
}
