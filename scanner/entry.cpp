#include <include.hpp>

static_assert(sizeof(void*) == 8);

constexpr auto kernel_image_name = "ntoskrnl.exe";
const auto searched_imports = {
	"IofCompleteRequest",
};

namespace pe
{
	inline PIMAGE_NT_HEADERS get_nt_headers_unsafe(uint8_t* base)
	{
		const auto dos_header = PIMAGE_DOS_HEADER(base);
		const auto nt_headers = PIMAGE_NT_HEADERS(base + dos_header->e_lfanew);

		return nt_headers;
	}

	PIMAGE_NT_HEADERS get_nt_headers(uint8_t* base)
	{
		const auto dos_header = PIMAGE_DOS_HEADER(base);
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		const auto nt_headers = PIMAGE_NT_HEADERS(base + dos_header->e_lfanew);
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE || nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
			return nullptr;

		return nt_headers;
	}

	template <typename T>
	T* file_rva_to_va(uint8_t* base, const uint32_t rva)
	{
		const auto nt_headers = get_nt_headers_unsafe(base);
		const auto sections = IMAGE_FIRST_SECTION(nt_headers);

		for (size_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i)
		{
			const auto& section = sections[i];
			if (rva >= section.VirtualAddress && rva < section.VirtualAddress + section.SizeOfRawData)
				return (T*)(base + (rva - section.VirtualAddress + section.PointerToRawData));
		}

		return (T*)(base + rva);
	}
}

std::vector<uint8_t> read_file(const std::string& file_path)
{
	std::ifstream stream(file_path, std::ios::in | std::ios::ate | std::ios::binary);
	if (!stream)
		return {};

	const auto size = stream.tellg();
	stream.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	stream.read((char*)buffer.data(), size);

	return buffer;
}

struct found_driver
{
	std::string display_path;
	std::string file_path;
	size_t size;
};
std::vector<found_driver> found_drivers;

void check_file(const std::string& display_path, const std::string& file_path)
{
	auto buffer = read_file(file_path);
	if (buffer.empty())
	{
		std::printf("Failed to read file: %s.\n", display_path.c_str());
		return;
	}

	const auto base = buffer.data();
	const auto nt_headers = pe::get_nt_headers(base);
	if (!nt_headers || nt_headers->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE)
		return;

	const auto import_dir = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (!import_dir.Size || !import_dir.VirtualAddress)
		return;

	bool added = false;
	auto import_descriptor = pe::file_rva_to_va<IMAGE_IMPORT_DESCRIPTOR>(base, import_dir.VirtualAddress);
	for (; import_descriptor->Name; ++import_descriptor)
	{
		const auto module_name = pe::file_rva_to_va<const char>(base, import_descriptor->Name);
		if (strcmp(module_name, kernel_image_name) != 0)
			continue;

		auto imported_func = pe::file_rva_to_va<IMAGE_THUNK_DATA>(base,
			import_descriptor->OriginalFirstThunk ? import_descriptor->OriginalFirstThunk : import_descriptor->FirstThunk);

		for (; imported_func->u1.AddressOfData; ++imported_func)
		{
			if (imported_func->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				continue;

			const auto import_name = pe::file_rva_to_va<IMAGE_IMPORT_BY_NAME>(base,
				uint32_t(imported_func->u1.AddressOfData))->Name;

			if (std::find(std::begin(searched_imports), std::end(searched_imports), std::string(import_name))
				!= searched_imports.end())
			{
				if (!added)
				{
					found_driver driver;
					driver.display_path = display_path;
					driver.file_path = file_path;
					driver.size = buffer.size();

					found_drivers.push_back(driver);

					added = true;
				}
			}
		}
	}
}

void check_files_in_directory(const std::string& directory_path)
{
	namespace fs = std::filesystem;

	const auto normalized_path = fs::path(directory_path).string() + "\\";
	try
	{
		for (const auto& file : fs::recursive_directory_iterator(directory_path))
		{
			if (fs::is_regular_file(file) && file.path().extension() == ".sys")
			{
				const auto file_path = file.path().string();
				const auto has_substr = file_path.find_first_of(normalized_path) != std::string::npos;
				const auto display_path = has_substr ? (".\\" + file_path.substr(normalized_path.size())) : file_path;

				check_file(display_path, file_path);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::printf("Failed to iterate over directory: %s.\n", e.what());
	}
}

int main()
{
	check_files_in_directory("C:\\Windows\\system32");

	std::sort(std::begin(found_drivers), std::end(found_drivers), [](const auto& lhs, const auto& rhs)
		{
			return lhs.size < rhs.size;
		});

	for (const auto& driver : found_drivers)
	{
		std::printf("%s (%.2f MB)\n", driver.display_path.c_str(), driver.size / 1024.0f / 1024.0f);
	}
}