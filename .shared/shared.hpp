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
	unsigned __int64 dst_address;
	unsigned __int64 src_address;
};