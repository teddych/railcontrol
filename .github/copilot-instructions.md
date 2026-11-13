# RailControl - Copilot Instructions

## Compilation
- On Windows compile RailControl within cygwin environment. Do not try to compile if cygwin is not installed.

## Coding style
- Never use `auto` in C++, always write full type.
- If there is more than one condition then use extra parentheses in if statements to clarify what is checked first.

## Design
- The class `Manager` handles the central management of all data that has to be passed around in the project.
- The classes in the directory `Hardware` each represent a type of control hardware that connects RailControl to a model railway layout.
- The classes in the directory `Server` each represent a type of server through which RailControl can be controlled.
- Hardware and servers are always and only accessed via `ControlInterface`.
- Do not modify the `Manager` unless I explicitly request it.
