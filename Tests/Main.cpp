// UnitTests.cpp : Defines the entry point for the console application.
//

#include "../Exporter/Exporter.h"

#include "gtest/gtest.h"

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
