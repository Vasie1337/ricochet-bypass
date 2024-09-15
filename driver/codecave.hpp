#pragma once
#include <include.hpp>

namespace codecave
{
	uint8 shellcode[] = {
		0x48, 0x89, 0xC0,
		0x90,
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x90,
		0xFF, 0xE0
	};

	void* find(uint64 start, uint64 end, size_t size = sizeof(shellcode))
	{
		uint8* ptr = (uint8*)start;
		while (ptr < (uint8*)end)
		{
			if (ptr[0] == 0xCC)
			{
				uint8* code_cave = ptr;
				for (size_t i = 0; i < size; i++)
				{
					if (code_cave[i] != 0xCC)
					{
						code_cave = nullptr;
						break;
					}
				}

				if (code_cave)
				{
					return code_cave;
				}
			}

			ptr++;
		}

		return nullptr;
	}

	bool setup(void* code_cave, void* target)
	{
		*(void**)&shellcode[6] = target;

		auto cr0 = __readcr0();
		__writecr0(cr0 & ~(1 << 16));

		crt::memcpy(code_cave, shellcode, sizeof(shellcode));
		printf("Cave at: %p\n", code_cave);

		__writecr0(cr0);

		return true;
	}
}