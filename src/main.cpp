#include <windows.h>
#include <string>
#include <cstdlib>
#include <cstdint>
#include "samp.hpp"

static constexpr uintptr_t GPCI_MEMORY_ADDRESS = 0x00C9236C;
static constexpr size_t    GPCI_LENGTH = 32;

static const char* REGISTRY_PATH = "Software\\SAMP\\";
static const char* REGISTRY_VALUE_NAME = "GPCI_SPECIAL";

static std::string GenerateRandomGpci()
{
    std::string value;
    value.reserve(GPCI_LENGTH);
    for (size_t i = 0; i < GPCI_LENGTH; i++)
        value += static_cast<char>('0' + (rand() % 10));
    return value;
}

static bool WriteGpciToRegistry(const std::string& value)
{
    HKEY key;
    if (RegOpenKeyA(HKEY_CURRENT_USER, REGISTRY_PATH, &key) != ERROR_SUCCESS)
        return false;

    LONG result = RegSetValueExA(key, REGISTRY_VALUE_NAME, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>(value.length()));
    RegCloseKey(key);

    return (result == ERROR_SUCCESS);
}

static void PatchGpciInMemory(const std::string& value)
{
    DWORD oldProtect;
    size_t len = (value.length() < GPCI_LENGTH) ? value.length() : GPCI_LENGTH;

    if (!VirtualProtect(reinterpret_cast<void*>(GPCI_MEMORY_ADDRESS), GPCI_LENGTH,
        PAGE_EXECUTE_READWRITE, &oldProtect))
        return;

    for (size_t i = 0; i < len; i++)
    {
        *reinterpret_cast<char*>(GPCI_MEMORY_ADDRESS + i) = value[i];
    }

    VirtualProtect(reinterpret_cast<void*>(GPCI_MEMORY_ADDRESS), GPCI_LENGTH,
        oldProtect, &oldProtect);
}

static void ThreadMain()
{
    while (!samp.isAvailable())
        Sleep(100);

    srand(static_cast<unsigned int>(GetTickCount64()));
    std::string value = GenerateRandomGpci();

    WriteGpciToRegistry(value);
    PatchGpciInMemory(value);
}

class cEntry
{
public:
    cEntry()
    {
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadMain), nullptr, 0, nullptr);
    }
} _entry;