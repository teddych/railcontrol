## Convertidor de RS-232 a USB

Algunos controles ofrecen un conector RS-232, también conocido como puerto serial o puerto COM. Las computadoras recientes solamente ofrecen conectores USB. Hay convertidores de fabricantes diferentes. Recomendamos utilizar convertidores con un chip de FTDI. Esos chips normalmente también son soportados por sistemas operativos futuros. Otros chips normalmente no tienen soporte en el futuro y se tiene que comprar un nuevo convertidor.

También hay controles con un conector RS-232 y un convertidor interno incluido. Externamente ofrecen solamente un conector USB. El comportamiento es el mismo que el de los convertidores.

### Puertos seriales en Windows (puertos COM)

En Windows cada convertidor tiene su propio número de puerto COM. Estos números nunca cambian. Esto facilita la configuración de los controles en RailControl.

### Puertos seriales en Linux

En Linux cada convertidor de RS-232 a USB recibe un nombre durante el inicio de Linux o al conectar el dispositivo. Normalmente el primer dispositivo se llama /dev/ttyUSB0 y cada dispositivo adicional incrementa el número en uno. Si hay uno o más dispositivos conectados con un convertidor a una computadora con Linux, el orden de los nombres puede cambiar cada vez que se reinicia la computadora. Si el nombre cambia, RailControl no puede encontrar los dispositivos o se comunica con los dispositivos incorrectos. En un terminal, con el comando.

```
ls /dev/ttyUSB*
```

recibimos los nombres de los dispositivos conectados:

```
/dev/ttyUSB0   /dev/ttyUSB1
```

Los nombres se intercambian regularmente o se reemplazan por /dev/ttyUSB2.

Es posible fijar el nombre o crear al menos un enlace fijo. Para ello tenemos que averiguar cómo se identifica el convertidor desde el punto de vista del sistema. Con

```
lsusb
```

recibimos una lista con los dispositivos USB. Vemos los dispositivos estándar y también los dos convertidores (ESU Lokprogrammer y CC-Schnitte):

```
Bus 002 Device 003: ID 0408:03ba Quanta Computer, Inc. 
Bus 002 Device 021: ID 0403:6001 Future Technology Devices International, Ltd FT232 USB-Serial (UART) IC
Bus 002 Device 002: ID 8087:0020 Intel Corp. Integrated Rate Matching Hub
Bus 002 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
Bus 001 Device 008: ID 0461:4d17 Primax Electronics, Ltd Optical Mouse
Bus 001 Device 007: ID 0403:6001 Future Technology Devices International, Ltd FT232 USB-Serial (UART) IC
Bus 001 Device 002: ID 8087:0020 Intel Corp. Integrated Rate Matching Hub
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

Para nosotros los identificadores son importantes. A la izquierda de los dos puntos está el identificador del fabricante y a la derecha está el identificador del producto. Además necesitamos el número de serie. Lo obtenemos con:

```
udevadm info -a -n /dev/ttyUSB0 | grep '{serial}' | head -n1
```

El resultado es:

```
ATTRS{serial}=="FTF3ZDL5"
```

Tenemos que repetir esto para el segundo dispositivo. Por esto tenemos que remplazar /dev/ttyUSB0 con /devttyUSB1.

Con los identificadores del fabricante y del productor y del número de serie podemos configurar udev. Normalmente la configuración de udev está en la carpeta /etc/udev/rules.d y a veces en /etc/udev/rules (nota .d que falta). Creamos un archivo con el nombre 99-usb-serial.rules (si no existe).

En el archivo añadimos las siguientes líneas (reemplace los identificadores por los de sus dispositivos):

```
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="FTF3ZDL5", SYMLINK+="LokProgrammer"
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A907FXY2", SYMLINK+="CC-Schnitte"
```

Después de reiniciar Linux tenemos los dos archivos (enlaces):

```
$ ls /dev/LokProgrammer /dev/CC-Schnitte
/dev/CC-Schnitte  /dev/LokProgrammer
```

Ahora RailControl puede utilizar estos nombres para comunicar con los dispositivos.

