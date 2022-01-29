#include "ElfFile.h"

#include "Elf.h"
#include <cstring>

namespace VS
{
    ElfFilePrototype::ElfFilePrototype(const std::string& Filepath)
        : File(Filepath, EFileType::Binary, false)
    {
        UByte HeaderBeginning = std::vector<UByte>(Read(4, 2))[0];

        // 32-bit
        if(HeaderBeginning == 1)
        {
            Arch = EArchitecture::x86;
        }
        else if(HeaderBeginning == 2) // 64-bit
        {
            Arch = EArchitecture::x64;
        }
    }

    std::variant<ElfFile32, ElfFile64> ParseElf(const ElfFilePrototype& FilePrototype)
    {
        switch(FilePrototype.GetArch())
        {
        case EArchitecture::x86:
            return ElfFile32(FilePrototype.GetFilePath());
        case EArchitecture::x64:
            return ElfFile64(FilePrototype.GetFilePath());
        default:
            return ElfFile32(""); // Invalid, this should never be reached
        }
    }

    // 32-bit
    ElfFile32::ElfFile32(const std::string& FilePath)
        : ElfFilePrototype(FilePath)
    {
        std::vector<UByte> ElfHeaderBytes = Read(sizeof(ElfHeader32));
        std::memcpy(&ElfHeader, ElfHeaderBytes.data(), sizeof(ElfHeader32));
        
        if (!CheckElf())
        {
            // Error handling
            return;
        }

        ProgramHeaders.reserve(ElfHeader.ProgramHeaderCount);

        // Load program headers
        for (size_t i = 0; i < ElfHeader.ProgramHeaderCount; ++i)
        {
            std::vector<UByte> ProgramHeaderData = Read(ElfHeader.ProgramHeaderTableOffset + ElfHeader.ProgramHeaderSize * i, ElfHeader.ProgramHeaderSize);
            ProgramHeaders.push_back(*reinterpret_cast<ElfProgramHeader32*>(ProgramHeaderData.data()));
        }

        LoadSections();
    }

    bool ElfFile32::CheckElf()
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (ElfHeader.ElfIdentifier[i] != ElfMagic[i])
                return false;
        }
        return true;
    }

    Address32 ElfFile32::FindMainFunction()
    {
        return Address32();
    }

    void ElfFile32::LoadSections()
    {
        std::vector<ElfSectionHeader32> SectionHeaders;
        std::vector<std::string> SectionNames;

        SectionHeaders.reserve(ElfHeader.SectionHeaderCount);
        SectionNames.reserve(ElfHeader.SectionHeaderCount);
        Sections.reserve(ElfHeader.SectionHeaderCount);

        // Load section headers
        for (size_t i = 0; i < ElfHeader.SectionHeaderCount; ++i)
        {
            std::vector<UByte> SectionHeaderData = Read(ElfHeader.SectionHeaderTableOffset + ElfHeader.SectionHeaderSize * i, ElfHeader.SectionHeaderSize);
            SectionHeaders.push_back(*reinterpret_cast<ElfSectionHeader32*>(SectionHeaderData.data()));
        }

        // Load section string table
        Offset32 StringTableStart = SectionHeaders[ElfHeader.StringTableIndex].SectionOffset;
        Offset32 StringTableEnd = Find(SectionHeaders[ElfHeader.StringTableIndex].SectionOffset + 1, { 0x0, 0x0 });

        for (Offset32 Offset = 1; StringTableStart + Offset < StringTableEnd;)
        {
            Offset32 StringStart = SectionHeaders[ElfHeader.StringTableIndex].SectionOffset + Offset;
            Address32 StringEnd = Find(StringStart, { 0x0 });
            std::vector<UByte> SectionNameBytes = Read(StringStart, StringEnd - StringStart);

            SectionNames.emplace_back(SectionNameBytes.begin(), SectionNameBytes.end());
            Offset += StringEnd - StringStart + 1;
        }

        std::unordered_map<UWord, UWord> StringTableIndices;
        StringTableIndices.reserve(ElfHeader.SectionHeaderCount);

        // Combine section headers with names into a map for easier access
        for (size_t i = 0; i < ElfHeader.SectionHeaderCount; ++i)
        {
            Sections.insert_or_assign(SectionNames[StringTableIndices[SectionHeaders[i].SectionName]], SectionHeaders[i]);
        }
    }

    // 64-bit 
    ElfFile64::ElfFile64(const std::string& FilePath)
        : ElfFilePrototype(FilePath)
    {
        std::vector<UByte> ElfHeaderBytes = Read(sizeof(ElfHeader64));
        std::memcpy(&ElfHeader, ElfHeaderBytes.data(), sizeof(ElfHeader64));

        if (!CheckElf())
        {
            // Error handling
            return;
        }

        ProgramHeaders.reserve(ElfHeader.ProgramHeaderCount);

        // Load program headers
        for (size_t i = 0; i < ElfHeader.ProgramHeaderCount; ++i)
        {
            std::vector<UByte> ProgramHeaderData = Read(ElfHeader.ProgramHeaderTableOffset + ElfHeader.ProgramHeaderSize * i, ElfHeader.ProgramHeaderSize);
            ProgramHeaders.emplace_back(*reinterpret_cast<ElfProgramHeader64*>(ProgramHeaderData.data()));
        }

        LoadSections();

        // Find the main function
    }

    bool ElfFile64::CheckElf()
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (ElfHeader.ElfIdentifier[i] != ElfMagic[i])
                return false;
        }
        return true;
    }
    Address64 ElfFile64::FindMainFunction()
    {
        return Address64();
    }
    void ElfFile64::LoadSections()
    {
        std::vector<ElfSectionHeader64> SectionHeaders;
        std::vector<std::string> SectionNames;

        SectionHeaders.reserve(ElfHeader.SectionHeaderCount);
        SectionNames.reserve(ElfHeader.SectionHeaderCount);
        Sections.reserve(ElfHeader.SectionHeaderCount);

        // Load section headers
        for (size_t i = 0; i < ElfHeader.SectionHeaderCount; ++i)
        {
            std::vector<UByte> SectionHeaderData = Read(ElfHeader.SectionHeaderTableOffset + ElfHeader.SectionHeaderSize * i, ElfHeader.SectionHeaderSize);
            SectionHeaders.emplace_back(*reinterpret_cast<ElfSectionHeader64*>(SectionHeaderData.data()));
        }

        // Load section string table
        Offset64 StringTableStart = SectionHeaders[ElfHeader.StringTableIndex].SectionOffset;
        Offset64 StringTableEnd = Find(SectionHeaders[ElfHeader.StringTableIndex].SectionOffset + 1, { 0x0, 0x0 });
        std::unordered_map<UWord, UWord> StringTableIndices;
        StringTableIndices.reserve(ElfHeader.SectionHeaderCount);

        for (Offset64 Offset = 1; StringTableStart + Offset < StringTableEnd;)
        {
            Offset64 StringStart = SectionHeaders[ElfHeader.StringTableIndex].SectionOffset + Offset;
            Address64 StringEnd = Find(StringStart, { 0x0 });
            std::vector<UByte> SectionNameBytes = Read(StringStart, StringEnd - StringStart);

            SectionNames.emplace_back(SectionNameBytes.begin(), SectionNameBytes.end());

            StringTableIndices.insert_or_assign(Offset, SectionNames.size() - 1);
            Offset += StringEnd - StringStart + 1;
        }

        // Combine section headers with names into a map for easier access
        for (size_t i = 0; i < ElfHeader.SectionHeaderCount; ++i)
        {
            Sections.insert_or_assign(SectionNames[StringTableIndices[SectionHeaders[i].SectionName]], SectionHeaders[i]);
        }
    }
}