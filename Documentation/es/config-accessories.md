# Configuración de los accesorios
En la pantalla principal se puede ir a la configuración de los accesorios con el icono ![](../menu_accessory.png).

## Datos básicos
TODO FALTA IMAGEN CORRECTA (la imagen parece ser de la configuración de desvíos)
![](switches_basics_es.png)

### Nombre
Cada accesorio necesita un nombre inequívoco. Si el nombre no está indicado, RailControl crea un nombre. Si el nombre ya existe, RailControl añade un número al nombre para hacerlo inequívoco.

### Control
Si hay más de un control configurado en RailControl, se tiene que seleccionar el control que controla el accesorio. Si solamente hay un control configurado en RailControl, el campo de selección no está visible.

### Protocolo
Si un control soporta más de un protocolo, se tiene que seleccionar el protocolo que controla el accesorio. Si el control solamente soporta un protocolo, el campo de selección no está visible.

### Dirección
La dirección digital con la que se controla el accesorio.

### Duración de conmutación (ms)
Después del procedimiento de conmutación se tienen que apagar los accesorios electromagnéticos. En accesorios nuevos suelen bastar 100ms; accesorios más viejos o lentos a veces necesitan 250ms. Algunos controles apagan los accesorios automáticamente con un valor que se puede configurar directamente en el control. En este caso se puede configurar 0ms aquí. Según el tipo de accionamiento o decodificador utilizado, se puede prescindir del apagado. Esto afecta especialmente a muchos accionamientos de desvíos con servomotor o motor y a sus decodificadores correspondientes.

### Invertido
Cuando las conexiones del accesorio están conectadas al revés en el decodificador, RailControl puede invertirlas virtualmente.

## Posición
TODO FALTA IMAGEN CORRECTA (la imagen parece ser de la posición de desvíos)
![](switches_position_es.png)

### Posición X
La posición del elemento en cuadros desde la izquierda en el diagrama de vías. Se empieza a contar con cero. Si un elemento es más grande que un cuadrado, el cuadrado superior izquierdo es importante para contar.

### Posición Y
La posición del elemento en cuadros desde arriba en el diagrama de vías. Se empieza a contar con cero. Si un elemento es más grande que un cuadrado, el cuadrado superior izquierdo es importante para contar.

### Capa
La capa en que el elemento está visible.

### Rotación
Se puede rotar los elementos en pasos de 90 grados.

