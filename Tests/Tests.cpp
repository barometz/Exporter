#include "../Exporter/Exporter.h"
#include "gtest/gtest.h"

namespace
{
const std::string DllNameWith = "DllWithExports.dll";
const std::string DllNameWithout = "DllWithOutExports.dll";
}

TEST(WithExports, NoThrow)
{
  ASSERT_NO_THROW(Exporter::GetDllExports(DllNameWith));
}

TEST(WithExports, TwoItems)
{
  std::vector<std::string> results;
  ASSERT_NO_THROW(results = Exporter::GetDllExports(DllNameWith));
  ASSERT_EQ(2, results.size());
}

TEST(WithExports, NamesMatch)
{
  std::vector<std::string> results;
  ASSERT_NO_THROW(results = Exporter::GetDllExports(DllNameWith));
  EXPECT_EQ(results.at(0), "?Create@WithExports@@CAPAV1@H@Z");
  EXPECT_EQ(results.at(1), "?Destroy@WithExports@@CAXPAV1@@Z");
}

TEST(WithoutExports, NoThrow)
{
  ASSERT_NO_THROW(Exporter::GetDllExports(DllNameWithout));
}

TEST(WithoutExports, NoItems)
{
  std::vector<std::string> results;
  ASSERT_NO_THROW(results = Exporter::GetDllExports(DllNameWithout));
  ASSERT_EQ(0, results.size());
}
