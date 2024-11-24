#!/bin/bash

# Número de clientes a simular
NUM_CLIENTES=6

# Ejecutar múltiples instancias del programa client con un retraso de 2.5 segundos entre cada uno
for i in $(seq 1 $NUM_CLIENTES); do
    echo "Iniciando cliente $i..."
    ./client &  # El & permite ejecutar el programa en segundo plano
    sleep 2.5   # Pausa de 2.5 segundos antes de iniciar el siguiente cliente
done

# Esperar a que todos los procesos terminen
wait
echo "Todos los clientes han terminado."