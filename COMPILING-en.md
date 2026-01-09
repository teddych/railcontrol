# Source Code

The sourcecode is available under [GitHub RailControl](https://github.com/teddych/railcontrol).

The sourcecode is published under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) Open-Source-Licence.

RailControl should be compilable on all Posix-systems that have a GCC- or clang-compiler installed. RailControl has been tested on the following systems:

  * Windows with Cygwin
  * Ubuntu Linux
  * Mac OS X

# Compile under Windows

RailControl requires a posix-environement. Windows is not posix compiliant, but [cygwin](https://www.cygwin.com/) offers the posix functions for Windows. To install the needed development tools one have to select the following additional packets:

```
gcc-g++ 
make 
git
```

After the cygwin installation one can get the sources in a cygwin terminal with:

```
git clone https://github.com/teddych/railcontrol.git
```

The sources are in the newly created directory railcontrol. One can change into this directory with

```
cd railcontrol
```

Now RailControl can be compiled with

```
make
```

Then RailControl can be started with

```
./railcontrol.exe
```

## Update

An update can be performed as follows:

```
git pull
make
```

## 32-bit Cygwin

There is a 32 bit build of cygwin. But there is no more support for the 32 bit version. Basically RailControl should run in the 32 bit environment, but we do not really support it.

# Compile under Linux or a Posix-Environment

On debian based systems the required developer tools can be installed with (this can be different on non debian Linux distributions):

```
sudo apt-get install g++ binutils make git
```

After installing the developer tools one can get the sources with:

```
git clone https://github.com/teddych/railcontrol.git
```

The sources are in the newly created directory railcontrol. One can change into this directory with

```
cd railcontrol
```

Now RailControl can be compiled with

```
make
```

Then RailControl can be started with

```
./railcontrol
```

## Update

An update can be performed as follows:

```
git pull
make
```

# Compile under Mac OS X

The required developer tools can be installed in a terminal with:

```
sudo xcode-select --install
```

After installing the developer tools one can get the sources with:

```
git clone https://github.com/teddych/railcontrol.git
```

The sources are in the newly created directory railcontrol. One can change into this directory with

```
cd railcontrol
```

Now RailControl can be compiled with

```
make
```

Then RailControl can be started with

```
./railcontrol
```

## Update

An update can be performed as follows:

```
git pull
make
```

