#pragma once

enum class _comm_type
{
	read,
	write,
	base,
	proc,
	cr3,
	peb,
	pid,
};

struct _comm_data
{
	short magic;

	_comm_type type;
	size_t size;

	void* target_proc;
	void* dst_address;
	void* src_address;

	char str_buffer[128];
};

constexpr int xor_key = 0x285;

inline void xor_comm_data(_comm_data* data)
{
	for (size_t i = 0; i < sizeof(_comm_data); i++)
		reinterpret_cast<char*>(data)[i] ^= xor_key;
}