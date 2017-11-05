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
  LPVOID lpMsgBuf = nullptr;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPTSTR>(&lpMsgBuf),
    0, NULL);

  std::string resultingMessage(reinterpret_cast<LPTSTR>(lpMsgBuf));
  LocalFree(lpMsgBuf);

  return resultingMessage;
}

static inline PBYTE RvaAdjust(_Pre_notnull_ PIMAGE_DOS_HEADER pDosHeader, _In_ DWORD raddr)
{
  if (raddr != NULL) {
    return ((PBYTE)pDosHeader) + raddr;
  }
  return NULL;
}

std::vector<std::string> DoTheThing(const std::string& fileName)
{
  std::vector<std::string> result;

  HINSTANCE instance = LoadLibraryEx(fileName.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
  if (instance == nullptr)
  {
    std::stringstream stream;
    stream << "Unable to load " << fileName << ". Message: " << GetLastErrorText();
    throw std::runtime_error(stream.str());
  }

  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)instance;
  PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader + pDosHeader->e_lfanew);
  PIMAGE_EXPORT_DIRECTORY pExportDir
    = (PIMAGE_EXPORT_DIRECTORY)
    RvaAdjust(pDosHeader,
      pNtHeader->OptionalHeader
      .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

  if (!pExportDir)
    return {};

  PBYTE pExportDirEnd = (PBYTE)pExportDir + pNtHeader->OptionalHeader
    .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
  PDWORD pdwFunctions = (PDWORD)RvaAdjust(pDosHeader, pExportDir->AddressOfFunctions);
  PDWORD pdwNames = (PDWORD)RvaAdjust(pDosHeader, pExportDir->AddressOfNames);
  PWORD pwOrdinals = (PWORD)RvaAdjust(pDosHeader, pExportDir->AddressOfNameOrdinals);

  for (DWORD nFunc = 0; nFunc < pExportDir->NumberOfFunctions; nFunc++) {
    PBYTE pbCode = (pdwFunctions != NULL)
      ? (PBYTE)RvaAdjust(pDosHeader, pdwFunctions[nFunc]) : NULL;
    PCHAR pszName = NULL;

    // if the pointer is in the export region, then it is a forwarder.
    if (pbCode >(PBYTE)pExportDir && pbCode < pExportDirEnd) {
      pbCode = NULL;
    }

    for (DWORD n = 0; n < pExportDir->NumberOfNames; n++) {
      if (pwOrdinals[n] == nFunc) {
        pszName = (pdwNames != NULL)
          ? (PCHAR)RvaAdjust(pDosHeader, pdwNames[n]) : NULL;
        break;
      }
    }
    ULONG nOrdinal = pExportDir->Base + nFunc;

    result.push_back(std::string(pszName));
  }

  return result;
}

}

std::vector<std::string> Exporter::GetDllExports(const std::string& fileName)
{
  return DoTheThing(fileName);
}
