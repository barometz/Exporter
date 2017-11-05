#pragma once

#include <string>
#include <vector>

namespace Exporter
{

std::vector<std::string> GetDllExports(const std::string& fileName);

}
