#pragma once
#include <windows.h>
using CMDPROC = void(__cdecl*)(const char*);

class COffsets {
public:
    unsigned long CInput{ 0 }, RegisterChatCommand{ 0 };
    unsigned long CChat{ 0 }, AddChatMessage{ 0 };

    COffsets(unsigned char versionByte)
    {
        switch (versionByte)
        {
        case 0xD8: // R1
        {
            CInput = 0x21A0E8;
            RegisterChatCommand = 0x65AD0;
            CChat = 0x21A0E4;
            AddChatMessage = 0x645A0;
        } break;
        case 0xA8: // R2
        {
            CInput = 0x21A0F0;
            RegisterChatCommand = 0x65BA0;
            CChat = 0x21A0EC;
            AddChatMessage = 0x64670;
        } break;
        case 0x78: // R3
        {
            CInput = 0x26E8CC;
            RegisterChatCommand = 0x69000;
            CChat = 0x26E8C8;
            AddChatMessage = 0x679F0;
        } break;
        case 0x60: // R4
        {
            CInput = 0x26E9FC;
            RegisterChatCommand = 0x69730;
            CChat = 0x26E9F8;
            AddChatMessage = 0x68130;
        } break;
        case 0x40: // R5
        {
            CInput = 0x26EB84;
            RegisterChatCommand = 0x69770;
            CChat = 0x26EB80;
            AddChatMessage = 0x68170;
        } break;
        }
    }
};

class CSAMP {
    class COffsets* pOffsets{ nullptr };
    struct CInput* pInput{ nullptr };
    struct CChat* pChat{ nullptr };
public:
    unsigned long baseAddress{ 0 };
    unsigned char versionByte{ 0 };

    unsigned long getOffset(unsigned long offset) const
    {
        return baseAddress + offset;
    }

    unsigned char getVersionByte() const
    {
        IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(baseAddress);
        IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(baseAddress + dos->e_lfanew);
        switch (nt->OptionalHeader.AddressOfEntryPoint)
        {
        case 0x31DF13:  return 0xD8; // R1
        case 0x3195DD: return 0xA8; // R2
        case 0xCC4D0:  return 0x78; // R3
        case 0xCBCB0:  return 0x60; // R4
        case 0xCBC90:  return 0x40; // R5
        default:       return 0x00;
        }
    }

    bool isAvailable()
    {
        baseAddress = reinterpret_cast<unsigned long>(GetModuleHandleA("samp.dll"));
        if (!baseAddress) return false;

        versionByte = getVersionByte();
        if (!versionByte) return false;

        pOffsets = new COffsets(versionByte);
        if (!pOffsets) return false;

        pInput = *reinterpret_cast<struct CInput**>(getOffset(pOffsets->CInput));
        if (!pInput) return false;

        pChat = *reinterpret_cast<struct CChat**>(getOffset(pOffsets->CChat));
        if (!pChat) return false;

        return true;
    }

    void registerChatCommand(const char* szCommand, CMDPROC pFunction)
    {
        reinterpret_cast<void(__thiscall*)(CInput*, const char*, CMDPROC)>
            (getOffset(pOffsets->RegisterChatCommand))(pInput, szCommand, pFunction);
    }

    void addChatMessage(unsigned long ulColor, const char* szText)
    {
        reinterpret_cast<void(__thiscall*)(CChat*, unsigned long, const char*)>
            (getOffset(pOffsets->AddChatMessage))(pChat, ulColor, szText);
    }
} samp;
