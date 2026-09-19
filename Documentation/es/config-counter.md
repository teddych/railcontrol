# Configuración de los contadores
En la pantalla principal se puede ir a la configuración de los contadores con el icono ![](../menu_counter.png).

Los contadores se usan en el modo automático para limitar el número de itinerarios reservados en una red de vías, por si los itinerarios se pudieran bloquear entre sí.

Durante la reserva de un itinerario, los contadores pueden ser
* aumentados en 1 y reducidos de nuevo al liberar el itinerario o
* reducidos en 1 y aumentados de nuevo al liberar el itinerario.

Un itinerario puede influir en ningún contador o en cualquier número de contadores. Para cada contador el usuario configura un valor máximo y un valor mínimo.
Cuando un contador ha alcanzado su valor máximo o mínimo, la reserva del itinerario queda bloqueada hasta que sea posible aumentar o reducir de nuevo el valor del contador.

## Datos básicos
![](counter_basics_es.png)

### Nombre
El nombre del contador es una identificación inequívoca. Si no se indica un nombre inequívoco, RailControl crea uno o añade un número al nombre para hacerlo inequívoco.

### Máximo
El valor más alto que puede alcanzar el contador. Si se alcanza este valor máximo, se bloquea la reserva del itinerario que quiera aumentar más el contador. El rango de valores es positivo, incluyendo cero.

### Mínimo
El valor más bajo que puede alcanzar el contador. Si se alcanza este valor mínimo, se bloquea la reserva del itinerario que quiera reducir más el contador. El rango de valores es negativo, incluyendo cero.

## Posición
![](counter_position_es.png)

Los contadores se muestran en la vista general como una vía recta con el valor actual del contador al lado.

### Posición X
La posición del contador en cuadros desde la izquierda del diagrama de vías. Se empieza a contar con cero.

### Posición Y
La posición del contador en cuadros desde arriba del diagrama de vías. Se empieza a contar con cero.

### Capa
La capa en la que el contador debe estar visible.

### Rotación
La representación del contador con vía se puede rotar en pasos de 90 grados.
