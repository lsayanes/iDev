# iDev
Low-level developments for Windows


En esta solucion hay dos proyectos:

# 1) Filtro NDIS
Implementación de un filtro NDIS para interceptar y manipular paquetes de red en Windows. 

Función:

```
FilterReceiveNetBufferLists
```

Esta función se encarga de recibir paquetes de red desde el adaptador de red y pasarlos a capas superiores.
Hemos analizado cómo manejar los paquetes, modificarlos si es necesario y cómo devolverlos a la pila NDIS.
Función iDevPeekRegisterDevice

Se encarga de registrar un dispositivo en el sistema para permitir la comunicación con una aplicación de usuario mediante IOCTL.
Se definió una tabla de funciones IRP (Dispatch Table) para manejar las solicitudes desde la aplicación.
Función iDevPeekDeviceIoControl

* Se encarga de procesar las solicitudes IOCTL de la aplicación de usuario.
* Se implementaron casos específicos para:
* Obtener un paquete capturado (IOCTL_IDEVPEEK_GET_PACKET).
* Establecer un filtro de captura (IOCTL_IDEVPEEK_SET_FILTER).
* Enumerar interfaces activas (IOCTL_FILTER_ENUERATE_ALL_INSTANCES).
* Reiniciar una interfaz filtrada (IOCTL_FILTER_RESTART_ONE_INSTANCE).

# 2) Aplicación en C++
La aplicación en C++ que se comunicará con el filtro a través de IOCTL para:

Recibir paquetes capturados.
Configurar filtros de tráfico.
Controlar el filtro (enumeración de interfaces, reinicios, etc.).


# Arquitectura NDIS


```
-------------------------------------------------
|   Aplicaciones de Red (Chrome, Steam, etc.)   |
-------------------------------------------------
|   Protocolos de Red (TCP/IP, UDP, etc.)       |
-------------------------------------------------
|   Filtro NDIS (iDevPeek)                      | <-- el driver
-------------------------------------------------
|   Controlador Miniport (Driver de la NIC)     |
-------------------------------------------------
|   Hardware de Red (Tarjeta de red física)     |
-------------------------------------------------
```

# ¿Cómo funciona un filtro NDIS?
Cuando un paquete de red pasa por el filtro, se llaman funciones específicas:

Tipo de tráfico	Función en el filtro NDIS
* Paquete enviado por el sistema	FilterSendNetBufferLists()
* Confirmación de envío de un paquete	FilterSendNetBufferListsComplete()
* Paquete recibido desde la red	FilterReceiveNetBufferLists()
* Devolución del paquete recibido	FilterReturnNetBufferLists()

Ejemplo simplificado de FilterReceiveNetBufferLists:

```
VOID
FilterReceiveNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG NumberOfNetBufferLists,
    ULONG ReceiveFlags
    )
{
    // Aquí es posible inspeccionar o modificar los paquetes recibidos
    ProcessNetworkTraffic(NetBufferLists);
    
    // Luego pasamos los paquetes al protocolo de red
    NdisFIndicateReceiveNetBufferLists(
        FilterModuleContext, 
        NetBufferLists, 
        PortNumber, 
        NumberOfNetBufferLists, 
        ReceiveFlags);
}
```

En esta función, podemos leer los paquetes, guardarlos en un buffer, enviarlos a una app en user-mode, modificarlos o bloquearlos antes de que el sistema los procese.


FLUJO DE DATOS
==============


Un paquete sale de una aplicación en Windows --->

1. La aplicación envía datos a través del stack TCP/IP.
2. El driver de protocolo los pasa a NDIS.
3. El filtro NDIS puede interceptar o modificar los datos.
4. El miniport los envía a la tarjeta de red.
5. La tarjeta transmite el paquete.

Un paquete llega desde la red <----

1. La tarjeta de red recibe el paquete.
2. El miniport entrega los datos a NDIS.
3. El filtro NDIS puede interceptarlo y modificarlo.
4. El driver de protocolo entrega los datos a la aplicación.

# ¿Cómo se relaciona con el filtro NDIS?

* Un filtro NDIS se "engancha" a un miniport para inspeccionar o modificar los paquetes que entran y salen.
* FilterAttach se ejecuta cuando el filtro se conecta a un miniport.
* FilterDetach se ejecuta cuando el filtro se desconecta.





