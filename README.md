# ChismeGPT

El sistema `ChismeGPT` es un simulador de procesamiento de mensajes para usuarios pre-pago y pos-pago. Implementa un núcleo transaccional que prioriza mensajes pos-pago y permite hasta 10 mensajes gratuitos para usuarios pre-pago antes de forzar su cambio de plan. Además, el servidor limita la cantidad de mensajes concurrentes que procesa, simulando pérdida de mensajes si se supera este límite.

## Funcionalidad

El sistema consiste en dos programas principales (`server` y `client`) y un script de bash (`simular_trafico.sh`) para simular múltiples clientes conectándose al servidor.

### Ejecución del servidor
El servidor acepta conexiones de múltiples clientes y procesa sus mensajes según las siguientes reglas:
- Los mensajes **pos-pago** tienen prioridad.
- Los usuarios **pre-pago** pueden enviar hasta 10 mensajes antes de cambiar de plan.
- Los mensajes se procesan de manera concurrente hasta un límite configurado en el programa.

Para ejecutar el servidor:
```bash
$ ./server
[INFO] Servidor iniciado y esperando conexiones...
```
Ejecución del cliente:
Cada cliente pos-pago envía hasta 15 mensajes para simular tráfico de mensajes, respetando las reglas de su plan. Los usuarios pre-pago cambian a pos-pago de forma aleatoria si superan el límite de 10 mensajes gratuitos, notificando al servidor.

Para ejecutar un cliente individual:
```
$ ./client
[CLIENTE 123] Tipo de usuario: Pre-pago
[CLIENTE 123] Mensaje 1 enviado...
[CLIENTE 123] Límite alcanzado para pre-pago. Intentando cambiar a pos-pago...
[CLIENTE 123] Ahora es un cliente Pos-pago.
```
Simulación de tráfico con múltiples clientes
El script simular_trafico.sh permite lanzar múltiples clientes con un intervalo de 2.5 segundos entre cada uno, simulando un flujo de mensajes.

Para simular tráfico:
```
$ ./simular_trafico.sh
Iniciando cliente 1...
Iniciando cliente 2...
Iniciando cliente 3...
...
Todos los clientes han terminado.
```

 ## Compilación
Para compilar el programa:
```
$ make
```
Para compilar facilitando la depuración con gdb:

```
$ make debug
```
Para compilar habilitando la herramienta AddressSanitizer, facilita la depuración en tiempo de ejecución:

```
$ make sanitize
```
