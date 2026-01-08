# Código fuente

El código fuente está disponible en [GitHub RailControl](https://github.com/teddych/railcontrol).

El código fuente está publicado bajo la licencia Open-Source [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

Se puede comilar RailControl sobre todos sistemas posix que ofrecen un compilador GCC o clang. Se probó RailControl sobre los siguientes sistemas:

  * Windows con Cygwin
  * Ubuntu Linux
  * Mac OS X

# Compilar sobre Windows

RailControl necesita un entorno posix. Windows no ofrece un entorno posix, pero [cygwin](https://www.cygwin.com/) ofrece los funciones de posix para Windows. Para instalar las heremientas necesitades se tiene que seleccionar los paquetes siquientes:

```
gcc-g++ 
make 
git
```

Despues de instalar cygwin se puede consegir el codigo fuente con:

```
git clone https://github.com/teddych/railcontrol.git
```

Ahora el codigo fuente está en el directorio railcontrol. Se puede cambiar in eso directorio con

```
cd railcontrol
```

Entonces se puede compilar RailControl con

```
make
```

Despues el archivo railcontrol.exe puede ser iniciado.

## Actualizar

Se puede actualizar RailVontrol con:

```
git pull
make
```

## 32-bit Cygwin

Hay una version de cygwin con 32 bit, pero esta version no está más soportado. Basicamente RailControl tiene que funcionar en un entorno 32 bit, per no soportamos eso.

# Compilar sobre Linux o entono POSIX

Se puede instalar las heremientas requerido con:

```
sudo apt-get install g++ binutils make git
```

Despues de instalar cygwin se puede consegir el codigo fuente con:

```
git clone https://github.com/teddych/railcontrol.git
```

Ahora el codigo fuente está en el directorio railcontrol. Se puede cambiar in eso directorio con

```
cd railcontrol
```

Entonces se puede compilar RailControl con

```
make
```

Despues el archivo railcontrol puede ser iniciado.

```
./railcontrol
```

## Actualizar

Se puede actualizar railcontrol con:

```
git pull
make
```

# Compilar sobre Mac OS X

Se puede instalar las heremientas requerido con:

```
sudo xcode-select --install
```

Despues de instalar cygwin se puede consegir el codigo fuente con:

```
git clone https://github.com/teddych/railcontrol.git
```

Ahora el codigo fuente está en el directorio railcontrol. Se puede cambiar in eso directorio con

```
cd railcontrol
```

Entonces se puede compilar RailControl con

```
make
```

Despues el archivo railcontrol puede ser iniciado.

```
./railcontrol
```

## Actualizar

Se puede actualizar railcontrol con:

```
git pull
make
```

