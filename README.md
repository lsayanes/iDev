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
Próximos Pasos
Probar el filtro NDIS en Windows y verificar que los paquete
