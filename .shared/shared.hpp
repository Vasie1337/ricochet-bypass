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
	mouse,
};

struct _mouse_data
{
	short flags;
	int x;
	int y;
};

struct _comm_data // ik this is bad but i'm lazy
{
	short magic;

	_comm_type type;
	size_t size;

	void* target_proc;
	void* dst_address;
	void* src_address;

	char str_buffer[128];

	_mouse_data mouse_data;
};

constexpr int xor_key = 0x285;

inline void xor_comm_data(_comm_data* data)
{
	for (size_t i = 0; i < sizeof(_comm_data); i++)
		reinterpret_cast<char*>(data)[i] ^= xor_key;
}