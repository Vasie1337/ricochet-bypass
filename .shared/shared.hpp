#pragma once

enum class _comm_type
{
	read,
	write,
	base,
	proc,
	cr3,
	peb,
};

struct _comm_data
{
	short magic;
	void* target_proc;

	_comm_type type;

	unsigned __int64 size;
	void* dst_address;
	void* src_address;

	char str_buffer[128];
};

constexpr int xor_key = 0x285;

void xor_comm_data(_comm_data* data)
{
	for (size_t i = 0; i < sizeof(_comm_data); i++)
		reinterpret_cast<char*>(data)[i] ^= xor_key;
}