# GEMINI.md

## Project Overview

This project is a high-performance web server written in C++. It utilizes an event-driven, non-blocking I/O model based on the `epoll` system call, implementing the Reactor design pattern. The server is designed to handle a large number of concurrent connections efficiently.

### Key Technologies and Features:

*   **C++:** The core language of the project.
*   **Epoll:** For scalable I/O event notification.
*   **Reactor Pattern:** The server's architecture is based on the Reactor pattern, with `EventLoop` and `Channel` classes managing events and their handlers.
*   **Thread Pool:** A thread pool (`EventLoopThreadPool`) is used to distribute the handling of client connections across multiple threads, leveraging multi-core processors.
*   **HTTP Parsing:** The server includes components for parsing HTTP requests (`HttpRequest`) and generating HTTP responses (`HttpResponse`).
*   **MySQL Integration:** The server uses a connection pool (`SqlConnPool`) to manage connections to a MySQL database for functionalities like user authentication.
*   **Authentication:** An `AuthService` class provides user verification against the database.

## Building and Running

*TODO: The build and run commands are not explicitly available in the provided files. Information on how to compile and run the server should be added here. A `Makefile` or a `CMakeLists.txt` file would typically contain this information.*

### Example (hypothetical):

```bash
# Build the server
make

# Run the server
./web-server
```

## Development Conventions

*   **Coding Style:** The code follows a consistent style, with clear naming conventions for classes and methods. The use of `.clang-format` suggests an automated formatting process is in place.
*   **Concurrency:** The server is heavily multi-threaded. Concurrency is managed through mechanisms like mutexes, condition variables, and a thread pool.
*   **Resource Management:** The code uses `std::unique_ptr` and `std::shared_ptr` for automatic memory management. The `SqlConnRAII` class is used for RAII-style management of SQL connections.
*   **Modularity:** The code is organized into modules with clear responsibilities (e.g., `http`, `sql`, `event_loop`).
