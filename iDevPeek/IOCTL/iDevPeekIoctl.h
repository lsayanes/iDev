
#ifndef IDEVPEEK_IOCTL_H
#define IDEVPEEK_IOCTL_H

//#include <winioctl.h>

#define IDEVPEEK_DEVICE_NAME  L"\\\\.\\iDevPeek"
#define IDEVPEEK_IOCTL_BASE   0x800

//La app lo usara para recibir paquetes de red desde el driver.
#define IOCTL_IDEVPEEK_GET_PACKET CTL_CODE(IDEVPEEK_IOCTL_BASE, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS)

//Permite que la app configure un filtro (opcional, para filtrar tráfico desde el user-mode)
#define IOCTL_IDEVPEEK_SET_FILTER CTL_CODE(IDEVPEEK_IOCTL_BASE, 0x802, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#endif // IDEVPEEK_IOCTL_H
