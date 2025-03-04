# iDev { Low-level developments for Windows }


### En esta solucion hay dos proyectos:

## 1) Filtro NDIS
Implementación de un filtro NDIS para interceptar y manipular paquetes de red en Wind``ows. 

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

## 2) Aplicación en C++
La aplicación en C++ que se comunicará con el filtro a través de IOCTL para:

Recibir paquetes capturados.
Configurar filtros de tráfico.
Controlar el filtro (enumeración de interfaces, reinicios, etc.).

<details>
<summary>Arquitectura NDIS</summary>


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
=============

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

### ¿Cómo se relaciona con el filtro NDIS?

* Un filtro NDIS se "engancha" a un miniport para inspeccionar o modificar los paquetes que entran y salen.
* FilterAttach se ejecuta cuando el filtro se conecta a un miniport.
* FilterDetach se ejecuta cuando el filtro se desconecta.

</details>

<details>

<summary>OID -> NDIS</summary>


Los OID (Object Identifier) requests son mensajes usados para leer o modificar configuraciones de un adaptador de red en Windows.

En un filtro NDIS, estos comandos permiten interceptar, modificar o bloquear solicitudes que afectan al adaptador de red.

### ¿Qué cosas se pueden hacer con OID?
1. Consultar información del adaptador de red

	Velocidad de conexión (OID_GEN_LINK_SPEED).
	Dirección MAC (OID_802_3_CURRENT_ADDRESS).
	Estadísticas de tráfico (OID_GEN_STATISTICS).

2. Modificar parámetros del adaptador

	Activar/desactivar modos (OID_GEN_CURRENT_PACKET_FILTER).
	Configurar VLANs (OID_GEN_VLAN_ID).
	Cambiar direcciones MAC (OID_802_3_MULTICAST_LIST).

3. Interceptar solicitudes de configuración

	Bloquear cambios no deseados.
	Alterar parámetros antes de enviarlos al adaptador.
	Registrar estadísticas o detectar anomalías.

Esto sucede en la función FilterOidRequest de filter.c 


* Por ejemplo se quiere bloquear el cambio de dirección MAC en una tarjeta de red:

	1. Un programa ejecuta:

	```
			NdisSetRequest(OID_802_3_CURRENT_ADDRESS, nueva_direccion_mac);
	```
	
	2. FilterOidRequest intercepta el OID.
	3. En el filtro NDIS, verificas el OID y rechazas la solicitud:

	```
			if (Request->Oid == OID_802_3_CURRENT_ADDRESS) 
			{
				return NDIS_STATUS_NOT_SUPPORTED;  // Bloquea el cambio de MAC
			}
	```

	4.  La solicitud es bloqueada y el sistema sigue usando la MAC original



### Es obligatorio clonar el OID en un filtro NDIS

En un filtro NDIS, el controlador de red espera una respuesta a cada solicitud OID que recibe. 
Si el filtro no responde, la solicitud se pierde y el sistema puede comportarse de manera inesperada.

Razón principal:
Cuando un filtro recibe un OID, no debe modificar ni retener el original porque no le pertenece. 
Windows lo envía al filtro solo para ser procesado, pero el propietario real del OID es la capa superior (por ejemplo, un protocolo o una aplicación).

Por eso, en lugar de modificar el Request original, se clona y se reenvía al siguiente nivel.

### Proceso

1. Llega un OID desde capas superiores

	Windows o una aplicación envía un comando, por ejemplo, para cambiar la dirección MAC.
	Se ejecuta FilterOidRequest().

2.  Se clona la solicitud
	```
	Status = NdisAllocateCloneOidRequest(
					pFilter->FilterHandle,
					Request,         // OID original
					FILTER_TAG,      // Etiqueta de memoria
					&ClonedRequest   // Nuevo OID clonado
				);

	```	
	Esto crea una copia exacta del OID original.


3️. Se envía el OID clonado al siguiente nivel (el miniport)
```
Status = NdisFOidRequest(pFilter->FilterHandle, ClonedRequest);
```
* Si el miniport lo procesa exitosamente, se llama a FilterOidRequestComplete().
* Si el miniport lo rechaza, se notifica el fallo y se limpia la memoria.

4️. Cuando el OID termina, se completa el original
```
FilterOidRequestComplete(pFilter, ClonedRequest, Status);
```
### ¿Qué pasaría si NO clonamos el OID?
* Si el filtro modifica el Request original → Puede causar problemas porque la capa superior sigue esperando el mismo OID intacto.
* Si el filtro retiene el Request original → Se rompe la cadena de comunicación y Windows podría bloquear el adaptador de red.
* Si el filtro no clona pero reenvía el OID → Windows podría reutilizar la estructura mientras aún está en uso, causando corrupción de memoria.

</details>

<details>

<summary>IRQL (Interrupt Request Level) </summary>
<br>

Es el nivel de prioridad con el que se ejecuta el código en el kernel de Windows. 
Mientras más alto sea el nivel, menos operaciones están permitidas.

Los niveles más comunes son:

PASSIVE_LEVEL (0) = Nivel más bajo, permite ejecutar código en contexto de usuario, usar paginación y llamar funciones bloqueantes.
DISPATCH_LEVEL (2) = No permite esperas bloqueantes ni acceso a memoria paginada.
DIRQL (Niveles altos) → Solo se usa en controladores de hardware, para manejar interrupciones.

Como se setea:
```
	_IRQL_requires_max_(PASSIVE_LEVEL)
```

Este es un annotation de análisis estático que indica que la función iDevPeekRegisterDevice solo puede ejecutarse en un nivel de interrupción (IRQL) menor o igual a PASSIVE_LEVEL.


¿Por qué elijo PASSIVE_LEVEL en esta función?
iDevPeekRegisterDevice crea un dispositivo y registra dispatch handlers, lo que implica:

* Llamar a funciones como NdisRegisterDeviceEx, que solo pueden ejecutarse en PASSIVE_LEVEL porque pueden bloquear la ejecución.
* Operaciones con nombres Unicode (NdisInitUnicodeString), que requieren memoria paginada.
* Manipulación de estructuras de dispositivo y memoria del sistema.
* Si se intentara ejecutar esta función en DISPATCH_LEVEL o superior, el sistema podría generar un bug check (la famosa... pantalla azul)


### ¿Qué hace _IRQL_requires_max_?

Es un annotation de Microsoft que:

* Ayuda al analizador de código estático a detectar posibles errores de IRQL.
* Evita llamadas inseguras desde contextos de ejecución inapropiados.

### Resumen:
* _IRQL_requires_max_(PASSIVE_LEVEL) indica que la función debe ejecutarse en PASSIVE_LEVEL o inferior.
* Protege contra llamadas en niveles de interrupción incorrectos.
* Es útil para evitar bug checks y facilitar el análisis estático del código.


</details>
