# RailControl - Copilot Instructions

## Compilation

- On Windows, compile RailControl within the **Cygwin** environment.
- Do **not** try to compile if Cygwin is not installed.

## Running RailControl

- Start RailControl with `./railcontrol`.
- On Windows, run it within the **Cygwin** environment.
- The web server is then reachable at `http://[::1]:8082/` (or at the address/port configured in `railcontrol.conf`).
- Besides the main page, various commands are available via `/?cmd=<command>`, where `<command>` is replaced by the desired command.
- Stop RailControl by calling the web server:

  ```
  http://[::1]:8082/?cmd=shutdown
  ```

## Coding Style

- Never use `auto` in C++; always write the full type.
- If an `if` statement contains more than one condition, use extra parentheses to clarify the evaluation order:

  ```cpp
  if ((a == b) && (c != d))
  ```

## Design

| Component          | Responsibility                                                                                     |
|--------------------|----------------------------------------------------------------------------------------------------|
| `Manager`          | Central management of all data that has to be passed around in the project.                        |
| `Hardware/`        | Each class represents a type of control hardware connecting RailControl to a model railway layout. |
| `Server/`          | Each class represents a type of server through which RailControl can be controlled.                |
| `ControlInterface` | The only way hardware and servers are accessed.                                                    |

### Rules

- Hardware and servers are **always and only** accessed via `ControlInterface`.
- Do **not** modify the `Manager` unless explicitly requested.