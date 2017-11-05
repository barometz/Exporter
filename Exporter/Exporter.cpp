#include "Exporter.h"

#include <exception>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace
{

std::string GetLastErrorText()
{
  // Borrowed from https://msdn.microsoft.com/en-us/library/ms680582(v=vs.85).aspx
  LPTSTR messageBuffer = nullptr;
  DWORD errorCode = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorCode,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPTSTR>(&messageBuffer),
    0, NULL);

  std::string result(messageBuffer);
  LocalFree(messageBuffer);

  return result;
}

const BYTE* RvaAdjust(const IMAGE_DOS_HEADER* dosHeader, DWORD raddr)
{
  if (raddr)
  {
    const BYTE* byteHeaderAddress = reinterpret_cast<const BYTE*>(dosHeader);
    return byteHeaderAddress + raddr;
  }
  else
  {
    return nullptr;
  }
}

HINSTANCE GetLibraryHandle(const std::string& fileName)
{
  HINSTANCE instance = LoadLibraryEx(fileName.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);

  if (instance == nullptr)
  {
    std::stringstream stream;
    stream << "Unable to load " << fileName << ": " << GetLastErrorText();
    throw std::runtime_error(stream.str());
  }

  return instance;
}

const IMAGE_EXPORT_DIRECTORY* GetExportDirectory(const IMAGE_DOS_HEADER* dosHeader)
{
  const IMAGE_NT_HEADERS* ntHeader = 
    reinterpret_cast<const IMAGE_NT_HEADERS*>(RvaAdjust(dosHeader, dosHeader->e_lfanew));
  const IMAGE_DATA_DIRECTORY* directory = ntHeader->OptionalHeader.DataDirectory;
  const DWORD exportsOffset = directory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  const IMAGE_EXPORT_DIRECTORY* exportDirectory = 
    reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(RvaAdjust(dosHeader, exportsOffset));

  return exportDirectory;
}

}

std::vector<std::string> Exporter::GetDllExports(const std::string& fileName)
{
  std::vector<std::string> result;

  const DWORD* nameOffsets = nullptr;
  HINSTANCE instance = GetLibraryHandle(fileName);
  const IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(instance);
  const IMAGE_EXPORT_DIRECTORY* exportDirectory = GetExportDirectory(dosHeader);

  if (exportDirectory)
    nameOffsets = (DWORD*)RvaAdjust(dosHeader, exportDirectory->AddressOfNames);

  if (nameOffsets)
  {
    const DWORD* end = nameOffsets + exportDirectory->NumberOfNames;
    for (const DWORD* offset = nameOffsets; offset < end; ++offset)
    {
      const CHAR* name = reinterpret_cast<const CHAR*>(RvaAdjust(dosHeader, *offset));
      if (name)
        result.push_back(std::string(name));
    }
  }

  FreeLibrary(instance);

  return result;
}
