#pragma once
#include <include.hpp>

namespace scanner
{
    uint64 find_pattern(uint64 base, size_t range, const char* pattern, const char* mask)
    {
        const auto check_mask = [](const char* base, const char* pattern, const char* mask) -> bool
            {
                for (; *mask; ++base, ++pattern, ++mask)
                {
                    if (*mask == 'x' && *base != *pattern)
                    {
                        return false;
                    }
                }

                return true;
            };

        range = range - crt::strlen(mask);

        for (size_t i = 0; i < range; ++i)
        {
            if (check_mask((const char*)base + i, pattern, mask))
            {
                return base + i;
            }
        }

        return NULL;
    }

    uint64 find_pattern(uint64 base, const char* pattern, const char* mask)
    {
        const PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
        const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);

        for (size_t i = 0; i < headers->FileHeader.NumberOfSections; i++)
        {
            const PIMAGE_SECTION_HEADER section = &sections[i];

            if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
            {
                const auto match = find_pattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);

                if (match)
                {
                    return match;
                }
            }
        }

        return 0;
    }

    uint64 find_pattern(uint64 module_base, const char* pattern)
    {
        auto pattern_ = pattern;
        uint64 first_match = 0;

        if (!module_base)
        {
            return 0;
        }

        const auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + reinterpret_cast<IMAGE_DOS_HEADER*>(module_base)->e_lfanew);

        for (uint64 current = module_base; current < module_base + nt->OptionalHeader.SizeOfImage; current++)
        {
            if (!*pattern_)
            {
                return first_match;
            }

            if (*(unsigned int*)pattern_ == '\?' || *(unsigned int*)current == get_byte(pattern_))
            {
                if (!first_match)
                    first_match = current;

                if (!pattern_[2])
                    return first_match;

                if (*(unsigned short*)pattern_ == '\?\?' || *(unsigned int*)pattern_ != '\?')
                    pattern_ += 3;

                else
                    pattern_ += 2;
            }
            else
            {
                pattern_ = pattern;
                first_match = 0;
            }
        }

        return 0;
    }
}