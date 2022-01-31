#include "Application.h"

#include <vector>
#include <iostream>
#include <spdlog/spdlog.h>

#include "Injectors/ShellcodeParser.h"
#include "Disassembler/Disassembler.h"
#include <Core/FileFormats/ELF/ElfFile.h>
#include <Core/Log.h>

namespace VS
{
	VApplication::VApplication()
	{
		Log::Init("App");
	}
	void VApplication::Run()
	{
		std::vector<uint8_t> Shellcode = { 0x48, 0x83, 0xec, 0x08, 0x48, 0x8d, 0x3d, 0xa9, 0x0f, 0x00, 0x00, 0x31, 0xc0, 0xff, 0xd0, 0x31, 0xc0, 0x48, 0x83, 0xc4, 0x08, 0xc3, 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 };
		//std::vector<std::byte> Shellcode;
		
		Disassembler Disass;
		std::vector<size_t> CallInstructions = Disass.FindInstruction(Shellcode, ZYDIS_MNEMONIC_CALL);
		VS_LOG(error, "A");
		VS_LOG_EXTERNAL(Ext, error, "Hello!");

		ElfFilePrototype ElfFileProto("Test.elf");
		auto ElfFile = ParseElf(ElfFileProto);
		if (ElfFileProto.GetArch() == EArchitecture::x64)
		{
			std::vector<UByte> Shell = Disass.ExtractShellcode(std::get<ElfFile64>(ElfFile));
			
			int a = 0;
		}
		else
		{

		}
		

		int a = 0;
	}
}

// \x48\x83\xec\x08\x48\x8d\x3d\xa9\x0f\x00\x00\x31\xc0\xff\xd0\x31\xc0\x48\x83\xc4\x08\xc3\x0f\x1f\x80\x00\x00\x00\x00