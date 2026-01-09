# Source Code

Der Source-Code ist unter [GitHub RailControl](https://github.com/teddych/railcontrol) downloadbar.

Der Source-Code ist unter der [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) Open-Source-Lizenz freigegeben.

RailControl sollte auf allen Posix-Systemen kompilierbar sein, die einen GCC- oder clang-Compiler installiert haben. Getestet wurde RailControl auf folgenden Systemen:

  * Windows mit Cygwin
  * Ubuntu Linux
  * Mac OS X

# Kompilieren unter Windows

RailControl ist angwiesen auf eine Posix-Umgebung. Windows bietet dies von Haus aus nicht an, jedoch kann eine Posix-Umgebung in Form von [Cygwin](https://www.cygwin.com/) nachgerüstet werden. Um die benötigten Entwickler-Werkzeuge mit zu installieren müssen folgende Packete zusätzlich ausgewählt werden:

```
gcc-g++
make
git
```

Nach der Installation von Cygwin können in einem Cygwin-Terminal der Quellcode geholt werden:

```
git clone https://github.com/teddych/railcontrol.git
```

Der Quellcode ist nun im Verzeichnis railcontrol. In dieses Verzeichnis muss nun hineingewechselt werden:

```
cd railcontrol
```

Nun kann RailControl kompiliert werden:

```
make
```

Anschliessend ist die Datei railcontrol.exe zu starten.

## Aktualisieren

Ein Update kann folgendermassen durchgeführt werden:

```
git pull
make
```

## Cygwin in 32-Bit

Cygwin gibt es zwar noch in einer 32-Bit Variante, diese wird jedoch nur noch mit tiefer Priorität betreut. RailControl sollte auch auf der 32-Bit-Version noch voll funktionsfähig sein. Jedoch wird dies unsererseits nicht generell unterstützt.

# Kompilieren unter Linux bzw. unter einer Posix-Umgebung

Auf debian-basierten System  können die nötigen Entwickler-Werkzeuge in einem Terminal folgendermassen installiert werden (je nach Distribution kann dies etwas abweichen):

```
sudo apt-get install g++ binutils make git
```

Nach der Installation der Entwickler-Werkzeuge kann der Quellcode geholt werden:

```
git clone https://github.com/teddych/railcontrol.git
```

Der Quellcode ist nun im Verzeichnis railcontrol. In dieses Verzeichnis muss nun hineingewechselt werden:

```
cd railcontrol
```

Nun kann RailControl kompiliert werden:

```
make
```

Anschliessend ist die Datei railcontrol zu starten:

```
./railcontrol
```

## Aktualisieren

Ein Update kann folgendermassen durchgeführt werden:

```
git pull
make
```

# Kompilieren unter Mac OS X

Die nötigen Entwickler-Werkzeuge können in einem Terminal folgendermassen installiert werden:

```
sudo xcode-select --install
```

Nach der Installation der Entwickler-Werkzeuge kann der Quellcode geholt werden:

```
git clone https://github.com/teddych/railcontrol.git
```

Der Quellcode ist nun im Verzeichnis railcontrol. In dieses Verzeichnis muss nun hineingewechselt werden:

```
cd railcontrol
```

Nun kann RailControl kompiliert werden:

```
make
```

Anschliessend ist die Datei railcontrol zu starten:

```
./railcontrol
```

## Aktualisieren

Ein Update kann folgendermassen durchgeführt werden:

```
git pull
make
```

