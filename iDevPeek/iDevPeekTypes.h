#ifndef IDEVPEEK_TYPES_H
#define IDEVPEEK_TYPES_H


#define MAX_PACKET_SIZE 1514  // Tamaño máximo de un paquete Ethernet

// Estructura para enviar paquetes de red a la aplicación
typedef struct _PACKET_DATA {
    ULONG PacketLength;
    UCHAR Packet[MAX_PACKET_SIZE];
} PACKET_DATA, * PPACKET_DATA;

// Estructura para definir reglas de filtrado (ejemplo: bloquear un puerto)
typedef struct _FILTER_RULE {
    ULONG Protocol;  // 1 = ICMP, 6 = TCP, 17 = UDP
    USHORT Port;
    BOOLEAN Allow;
} FILTER_RULE, * PFILTER_RULE;

#endif // IDEVPEEK_TYPES_H
