# Configuración de los semáforos
En la pantalla principal se puede ir a la configuración de los semáforos con el icono ![](../menu_signal.png).

## Datos básicos
![](signals_basics_es.png)

### Nombre
Cada semáforo necesita un nombre inequívoco. Si el nombre no está indicado, RailControl crea un nombre. Si el nombre ya existe, RailControl añade un número al nombre para hacerlo inequívoco.

### Orientación
Obsoleto. Solamente se muestra si ya hay semáforos configurados como inicio o destino en una ruta. Se eliminará en versiones futuras.

Por favor, reemplaza este semáforo con una vía en la ruta y asocia este semáforo con la vía.

### Control
Si hay más de un control configurado en RailControl, se tiene que seleccionar el control que controla el semáforo. Si solamente hay un control configurado en RailControl, el campo de selección no está visible.

### Tipo
RailControl conoce los siguientes tipos de semáforos (otros tipos siguen):

Simple izquierda: Este semáforo conoce solamente los dos aspectos "parada" y "vía libre". El semáforo está posicionado a la izquierda de la vía.

Simple derecha: Este semáforo conoce solamente los dos aspectos "parada" y "vía libre". El semáforo está posicionado a la derecha de la vía.

Suiza enana: El semáforo enano de Suiza conoce los aspectos "parada", "vía libre" y "cuidado". Vea también: Swiss railway signaling.

Suiza L principal: El semáforo principal tipo L de Suiza conoce los aspectos "parada", "vía libre", "40 km/h", "60 km/h", "90 km/h" y "vía libre corta". Vea también: Swiss railway signaling.

Alemania Ks: El semáforo Ks conoce los aspectos "parada", "vía libre", "maniobrar", "cuidado", "oscuro" y "parada esperada".

### Longitud
Obsoleto. Solamente se muestra si ya hay semáforos configurados como inicio o destino en una ruta. Se eliminará en versiones futuras.

Por favor, reemplaza este semáforo con una vía en la ruta y asocia este semáforo con la vía.

### Control
Si hay más de un control configurado en RailControl, se tiene que seleccionar el control que controla el semáforo. Si solamente hay un control configurado en RailControl, el campo de selección no está visible.

### Protocolo
Si un control soporta más de un protocolo, se tiene que seleccionar el protocolo que controla el semáforo. Si el control solamente soporta un protocolo, el campo de selección no está visible.

### Dirección básica
La dirección básica que controla el semáforo. En "Direcciones" se puede introducir la dirección de cada aspecto del semáforo. La dirección básica es la dirección más baja que se puede usar.

### Duración de conmutación (ms)
Después del procedimiento de conmutación se tienen que apagar los accesorios. Para accesorios nuevos suelen bastar 100ms. A veces accesorios más viejos o lentos necesitan 250ms. Algunos controles apagan los accesorios automáticamente con un valor que se puede configurar directamente en el control. En este caso se puede configurar 0ms aquí.

### Invertido
Cuando las conexiones del accesorio están conectadas al revés, RailControl puede invertirlas virtualmente.

## Direcciones
![](signals_addresses_es.png)

Según el tipo de semáforo elegido en "Datos básicos", los aspectos posibles se muestran aquí. Para cada aspecto se puede elegir una dirección. La dirección más baja es la dirección básica elegida en "Datos básicos". Por defecto, las direcciones se eligen en orden ascendente empezando con la dirección básica.

## Posición
![](signals_position_es.png)
 
### Posición X
La posición del elemento en cuadros desde la izquierda en el diagrama de vías. Se empieza a contar con cero. Si un elemento es más grande que un cuadrado, el cuadrado superior izquierdo es importante para contar.

### Posición Y
La posición del elemento en cuadros desde arriba en el diagrama de vías. Se empieza a contar con cero. Si un elemento es más grande que un cuadrado, el cuadrado superior izquierdo es importante para contar.

### Capa
La capa en que el elemento está visible.

### Rotación
Se puede rotar los elementos en pasos de 90 grados.

## Retroseñales
Obsoleto. Solamente se muestra si ya hay retroseñales configuradas. Se eliminará en versiones futuras.

Por favor, elimina las retroseñales configurado.

### Modo automático
Obsoleto. Solamente se muestra si ya hay semáforos configurados como inicio o destino en una ruta. Se eliminará en versiones futuras.

Por favor, reemplaza este semáforo con una vía en la ruta y asocia este semáforo con la vía.

