# Código fuente

El código fuente está disponible en [GitHub RailControl](https://github.com/teddych/railcontrol).

El código fuente está publicado bajo la licencia Open Source [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

RailControl debería poder compilarse en todos los sistemas POSIX que tengan instalado un compilador GCC o clang. RailControl se ha probado en los siguientes sistemas:

  * Windows con Cygwin
  * Ubuntu Linux
  * Mac OS X

# Compilar en Windows

RailControl necesita un entorno POSIX. Windows no ofrece un entorno POSIX de forma nativa, pero [Cygwin](https://www.cygwin.com/) proporciona las funciones POSIX para Windows. Para instalar las herramientas de desarrollo necesarias se deben seleccionar los siguientes paquetes adicionales:

```
gcc-g++
make
git
libsqlite3-devel
zlib-devel
```

Después de instalar Cygwin se puede obtener el código fuente en una terminal de Cygwin con:

```
git clone https://github.com/teddych/railcontrol.git
```

El código fuente está ahora en el directorio railcontrol. Se puede cambiar a este directorio con:

```
cd railcontrol
```

Ahora se puede compilar RailControl con:

```
make
```

Después se puede iniciar el archivo railcontrol.exe.

## Actualizar

Se puede actualizar RailControl con:

```
git pull
make
```

## Cygwin de 32 bits

Existe una versión de Cygwin de 32 bits, pero ya no recibe soporte. Básicamente RailControl debería funcionar en un entorno de 32 bits, pero no damos soporte general para ello.

# Compilar en Linux o en un entorno POSIX

En sistemas basados en Debian se pueden instalar las herramientas de desarrollo necesarias con el siguiente comando (puede variar en distribuciones Linux que no estén basadas en Debian):

```
sudo apt-get install g++ binutils make git libsqlite3-dev zlib1g-dev
```

Después de instalar las herramientas de desarrollo se puede obtener el código fuente con:

```
git clone https://github.com/teddych/railcontrol.git
```

El código fuente está ahora en el directorio railcontrol. Se puede cambiar a este directorio con:

```
cd railcontrol
```

Ahora se puede compilar RailControl con:

```
make
```

Después se puede iniciar el archivo railcontrol:

```
./railcontrol
```

## Actualizar

Se puede actualizar RailControl con:

```
git pull
make
```

## Compilar con cmake

El soporte para cmake todavía está en desarrollo. Si la compilación con cmake falla en tu sistema,
usa GNU Make.

Configura el directorio de compilación fuera del árbol de fuentes:

```
cmake -B build
```

Compila RailControl con:

```
cmake --build build
```

Instala RailControl (como root) con:

```
cmake --install build
```

### Modo de desarrollo con cmake

Para desarrollar RailControl, "instala" RailControl en la copia de trabajo de Git.
El resultado es similar a compilar RailControl con GNU Make.

```
cmake -B build -DINSTALL_IN_PLACE=1
cmake --build build
cmake --install build
```

# Compilar en Mac OS X

Se pueden instalar las herramientas de desarrollo necesarias en una terminal con:

```
sudo xcode-select --install
```

SQLite y zlib se pueden instalar con [Homebrew](https://brew.sh/):

```
brew install sqlite zlib
```

Después de instalar las herramientas de desarrollo se puede obtener el código fuente con:

```
git clone https://github.com/teddych/railcontrol.git
```

El código fuente está ahora en el directorio railcontrol. Se puede cambiar a este directorio con:

```
cd railcontrol
```

Ahora se puede compilar RailControl con:

```
make
```

Después se puede iniciar el archivo railcontrol:

```
./railcontrol
```

## Actualizar

Se puede actualizar RailControl con:

```
git pull
make
```

