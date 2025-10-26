# Contributing to httplib

Thank you for your interest in contributing to httplib! This document provides guidelines and instructions for contributing to the project.

## Development Setup

### Prerequisites

- C++11 compatible compiler (g++ 4.8+, clang 3.4+, MSVC 2015+)
- Boost libraries (1.70 or later)
- CMake (3.10 or later) or Make
- Git

### Getting Started

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/your-username/httplib.git
   cd httplib
   ```

3. Create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Building and Testing

### Using Make

```bash
make           # Build all examples and tests
make examples  # Build only examples
make tests     # Build only tests
make clean     # Clean build artifacts
```

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
```

## Code Style Guidelines

### General Guidelines

- Follow existing code style and conventions
- Use C++11 standard features
- Keep the library header-only
- Maintain compatibility with cpp-httplib API where possible
- Write clear, self-documenting code
- Add comments only for complex logic

### Naming Conventions

- Classes: `PascalCase` (e.g., `Server`, `WebSocketClient`)
- Functions/Methods: `snake_case` (e.g., `listen()`, `set_content()`)
- Variables: `snake_case` with trailing underscore for members (e.g., `is_running_`)
- Constants: `UPPER_SNAKE_CASE`
- Namespaces: `lowercase`

### Code Organization

- Keep all implementation in `include/httplib.h`
- Use namespace `httplib` for public API
- Use namespace `httplib::detail` for internal implementation
- Group related functionality together

## Adding New Features

### HTTP Features

When adding HTTP features:
1. Check cpp-httplib API for compatibility
2. Implement using Boost.Beast primitives
3. Add appropriate examples
4. Update documentation

### WebSocket Features

When adding WebSocket features:
1. Maintain event-driven API pattern
2. Support both text and binary messages
3. Handle errors gracefully
4. Add appropriate examples

### Documentation

For any new feature:
1. Update README.md with usage examples
2. Update API.md with complete API reference
3. Add inline documentation for complex code
4. Update CHANGELOG.md

## Testing

### Writing Tests

- Add tests for new features in `tests/` directory
- Test both success and failure cases
- Test edge cases
- Ensure tests are deterministic

### Running Tests

```bash
cd tests
make          # Builds and runs the cpp-httplib test suite
```

## Submitting Changes

### Before Submitting

1. Ensure code compiles without warnings
2. Run all tests
3. Update documentation
4. Format code consistently
5. Write clear commit messages

### Commit Message Format

```
<type>: <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `test`: Test additions or modifications
- `chore`: Build process or auxiliary tool changes

Example:
```
feat: add support for chunked transfer encoding

Implement chunked transfer encoding for both client and server.
This allows streaming large responses without buffering.

Closes #123
```

### Pull Request Process

1. Update the README.md with details of changes if applicable
2. Update the CHANGELOG.md with your changes
3. Ensure all tests pass
4. Create a pull request with a clear title and description
5. Link any relevant issues
6. Wait for review and address feedback

## Code Review Process

- All submissions require review before merging
- Reviewers may request changes
- Address review comments promptly
- Maintain professional and respectful communication

## Reporting Issues

### Bug Reports

When reporting bugs, include:
- Clear description of the issue
- Steps to reproduce
- Expected vs actual behavior
- Environment details (OS, compiler, Boost version)
- Minimal code example demonstrating the issue

### Feature Requests

When requesting features:
- Describe the use case
- Explain why it's useful
- Provide examples of desired API
- Check if it exists in cpp-httplib

## Questions and Support

- Check existing documentation first
- Search existing issues
- Create a new issue with the "question" label
- Be specific and provide context

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be acknowledged in the project. Thank you for making httplib better!
