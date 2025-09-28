# Gloin Programming Language - v0.0.1

Gloin is a modern, statically-typed, compiled systems programming language designed for performance, control, and developer ergonomics. It features manual memory management, explicit error handling, and powerful concurrency primitives. The compiler is implemented in C and generates LLVM IR for optimal performance.

## ✅ Core Language Features

### Variables & Types

- **Variables**: `def variable_name: type = value;` syntax. Immutable by default.
- **Mutability**: `def mut` for mutable variables and `def const` for compile-time constants.
- **Primitive Types**: `i32`, `f32`, `bool`, `string`, `void`.
- **Compound Types**:
  - **Tuples**: Fixed-size, ordered lists of heterogeneous types. `def point: (i32, f32) = (10, 20.5);`
  - **Structs**: User-defined composite types.
  - **Enums**: C-style enumerations for a set of named integer constants.
- **Null Value**: `null` can be assigned to any type to indicate the absence of a value.
- **Comments**: Single-line comments are supported using `//`.

### Functions & Control Flow

- **Functions**: First-class functions declared with the `def` keyword.
- **Conditionals**: `if`, `else`, and `unless` statements for branching. `unless` is syntactic sugar for `if !...`.
- **Switch Statements**: `switch`, `case`, `default` for value-based branching.
- **Loops**: A single `for` loop construct is used for all iteration.
- **Return Statements**: Explicit return with type checking.
- **Deferred Execution**: `defer` statement schedules a function call to be executed just before the current function returns.

### Structs & Objects

- **Struct Declarations**: Syntax for bundling data and methods.
- **Field Access**: Dot notation (`object.field`).
- **Instance Methods**: Use the `self` keyword to access instance data.
- **Static Methods**: Use the `::` operator for methods that belong to the struct type itself (e.g., constructors).
- **Visibility Modifiers**: `pub` (public) and `priv` (private) for methods.

### Memory Management

- **Manual Management**: Memory is managed manually.
- **new and free Convention**: By convention, structs requiring heap allocation provide a static `new()` constructor and a `free()` instance method for deallocation.
- **defer for Cleanup**: The `defer` statement is the primary tool for ensuring resources are cleaned up correctly at scope exit.

## ✅ Concurrency

### Asynchronous Operations (I/O-bound)
- **deferred functions**: Return a `Deferred<T>` handle for non-blocking I/O tasks.
- **join() method**: Blocks until the operation completes and returns a `Result<T, E>`.
- **force_join() method**: Returns the value `T` on success or panics on error.

### Multi-threading (CPU-bound)
- **spawnable functions**: Can be executed on a separate system thread.
- **run keyword**: Launches a spawnable function, returning a `Spawn<T>` handle.
- **join() method**: Blocks until the thread completes, returning a `Result<T, E>` to handle success or a thread panic.

## ✅ Error Handling

- **Errors as Values**: Functions that can fail return a `Result<T, E>` enum, making error handling explicit.
- **Result<T, E> Type**: An enum with two variants:
  - `Ok(T)`: Contains the successful value.
  - `Err(E)`: Contains an error value.
- **Result Helpers**: Standard library methods like `is_ok()`, `is_err()`, `ok().value()`, and `err().value()` are provided for interacting with Result types.

## ✅ Import System & Package Management

- **Imports**: `import "module_name";` syntax to include code from other files.
- **Static Access**: The `::` operator is used to access static methods on structs or variants on enums (e.g., `Vector2::new(...)`, `Color::Red`).
- **Package Management**: The `Armory.toml` file and `includes/` directory structure for managing external packages remain from the previous specification.

## Quick Start

### Example Program```gloin
import "@std" // Assuming std library for println

// A struct with a static constructor and an instance method for cleanup
def struct Vector2 {
    x: f32,
    y: f32,

    def static new(x: f32, y: f32) -> Vector2 {
        // In a real scenario, this would allocate on the heap
        return Vector2{ x: x, y: y };
    }

    // Instance method for cleanup
    def free() {
        // Deallocate memory for `self`
        std.println("Vector freed!");
    }
}

// A function that uses the defer statement for guaranteed cleanup
def process_vector() {
    def v: Vector2 = Vector2::new(10.0, 20.0);
    defer v.free(); // Guarantees v.free() is called before process_vector returns

    std.println("Processing vector...");
    // ... function logic using v
}

def main() -> i32 {
    process_vector();
    return 0;
}
```

## Language Syntax

### Variable Declaration
```gloin
// Immutable by default
def count: i32 = 10;

// Mutable variable
def mut name: string = "Gloin";

// Compile-time constant
def const PI: f32 = 3.14159;

// Tuple
def point: (i32, f32) = (100, -55.5);
```

### Struct with Static and Instance Methods
```gloin```gloin
import "@std"

def struct Rectangle {
    width: f32,
    height: f32,

    // Static method (constructor)
    def static new(w: f32, h: f32) -> Rectangle {
        return Rectangle{ width: w, height: h };
    }

    // Instance method using `self`
    def area() -> f32 {
        return self.width * self.height;
    }
}

def main() -> i32 {
    def rect: Rectangle = Rectangle::new(10.0, 5.0);
    def area: f32 = rect.area();
    std.println(std.to_string(area)); // Assuming f32 to string conversion
    return 0;
}
```

### Concurrency Example
```gloin```gloin
import "@std"

// An I/O-bound asynchronous function
def deferred fetch_data() -> string {
    // Simulate a network request
    return "Data from web";
}

// A CPU-bound multi-threaded function
def spawnable compute_heavy_task() -> i32 {
    // Simulate heavy computation
    def mut result: i32 = 0;
    for def mut i: i32 = 0; i < 100000000; i = i + 1 {
        result = result + 1;
    }
    return result;
}

def main() -> i32 {
    // Run async task
    def handle: Deferred<string> = fetch_data();

    // Run threaded task
    def computation: Spawn<i32> = run compute_heavy_task();

    // Do other work...
    std.println("Tasks are running in the background.");

    // Wait for the results
    def data_result: Result<string, err> = handle.join();
    if data_result.is_ok() {
        std.println(data_result.ok().value());
    }

    def compute_result: Result<i32, err> = computation.join();
    if compute_result.is_ok() {
        std.println(std.to_string(compute_result.ok().value()));
    }

    return 0;
}
```

### Error Handling with Result
```gloin```gloin
import "@std"

// A function that can fail
def might_fail(should_fail: bool) -> Result<string, string> {
    if should_fail {
        return Err("Something went wrong!");
    }
    return Ok("Success!");
}

def main() -> i32 {
    def result: Result<string, string> = might_fail(true);

    if result.is_err() {
        std.print("Error: ");
        std.println(result.err().value());
        return 1;
    }

    std.println(result.ok().value());
    return 0;
}
```

## Summary of Changes from Pre-v0.0.1

### New Types
- `f32`: 32-bit floating-point numbers
- `void`: Function return type for functions that don't return a value
- `(T, U)`: Tuple types for fixed-size, heterogeneous collections
- `enum`: C-style enumerations for named integer constants

### New Keywords
- `const`: Compile-time constants
- `f32`: Floating-point type
- `void`: Void return type  
- `null`: Null value literal
- `self`: Reference to current instance in methods
- `else`: Conditional branching
- `switch`, `case`, `default`: Switch statements
- `enum`: Enumeration declarations
- `defer`: Deferred execution
- `deferred`: Asynchronous function modifier
- `spawnable`: Multi-threaded function modifier
- `run`: Launch spawnable functions
- `static`: Static method declaration

### New Language Features
- **Comments**: `//` for single-line comments is now officially specified
- **Control Flow**: `else` is added. `switch` statements are introduced. The `while` loop has been removed in favor of a single `for` loop construct
- **Memory Management**: The `defer` statement is introduced as a key feature for resource cleanup
- **Concurrency**: A major addition, providing `deferred` functions for async I/O and `spawnable` functions for multi-threading
- **Error Handling**: A formal `Result<T, E>` type has been introduced for explicit, value-based error handling
- **Structs**: Can now have static methods accessed via `::`
