#include <include.hpp>

int main() 
{
	if (!drv::init_handler("notepad.exe"))
	{
		printf("Failed to init handler\n");
		return 1;
	}
	
	uint64_t proc = drv::proc();
	uint64_t base = drv::base();
	uint64_t cr3 = drv::cr3();
	uint64_t peb = drv::peb();
	HANDLE pid = drv::pid(); 
	
	printf("PROC: %llx\n", proc);
	printf("BASE: %llx\n", base);
	printf("CR3: %llx\n", cr3);
	printf("PEB: %llx\n", peb);
	printf("PID: %p\n\n", pid);

	std::thread([&]()
	{
		while (true)
		{
			uint64_t base = drv::base();
			short mz = drv::read<short>(base);
			printf("MZ: %x\n", mz);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}).detach();

	overlay::create();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	overlay::destroy();

	return 0;
}