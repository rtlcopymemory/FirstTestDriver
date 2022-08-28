#include <iostream>
#include <Windows.h>

#define IOCTL_SKULLCODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0X2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BUFFER_SIZE 128

using namespace std;

int main()
{
    HANDLE device = NULL;
    DWORD bytesReturned = 0;
    
    device = CreateFile(L"\\\\.\\SkullEmoji", GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    if (device == NULL) {
        cout << "Couldnt open symlink: " << GetLastError() << endl;
        return EXIT_FAILURE;
    }

    CHAR sending[BUFFER_SIZE] = "Testing message from userspace";
    CHAR received[BUFFER_SIZE] = { 0 };
    BOOL sent = DeviceIoControl(device, IOCTL_SKULLCODE, sending, sizeof(sending), received, sizeof(received), &bytesReturned, NULL);

    cout << "Received from Kernel: `" << received << "` with length: " << bytesReturned << endl;
    cin.get();
}
