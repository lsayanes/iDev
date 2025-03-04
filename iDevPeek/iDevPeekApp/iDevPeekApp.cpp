#include <windows.h>
#include <iostream>
#include <iDevPeekIoctl.h>
#include "..\iDevPeekTypes.h"

int main()
{
    HANDLE hDevice = CreateFile(IDEVPEEK_DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE) 
    {
        std::cout << "Error al abrir el dispositivo: " << GetLastError() << std::endl;
        return 1;
    }

    PACKET_DATA packet;
    DWORD bytesReturned;

    if (DeviceIoControl(hDevice,
        IOCTL_IDEVPEEK_GET_PACKET,
        NULL, 0,  // No input buffer
        &packet, sizeof(PACKET_DATA),
        &bytesReturned,
        NULL))
    {
        std::cout << "Paquete recibido! Tamaño: " << bytesReturned << " bytes" << std::endl;
    }
    else 
    {
        std::cout << "Error en IOCTL: " << GetLastError() << std::endl;
    }

    CloseHandle(hDevice);
    return 0;
}
