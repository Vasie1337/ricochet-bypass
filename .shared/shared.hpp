#pragma once

enum class _comm_type
{
	read,
	write,
	base,
	cr3
};

struct _comm_data
{
	short magic;
	unsigned int target_pid;

	_comm_type type;

	unsigned __int64 size;
	void* dst_address;
	void* src_address;
};