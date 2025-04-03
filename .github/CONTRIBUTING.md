# Contributing to Transity

Thank you for your interest in contributing to Transity! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

By participating in this project, you are expected to uphold our Code of Conduct, which promotes a respectful and inclusive community.

## Getting Started

1. **Fork the repository**
2. **Clone your fork locally**
   ```bash
   git clone https://github.com/your-username/transity.git
   cd transity
   ```
3. **Set up the development environment**
   - Follow the instructions in the README.md to set up the build environment
   - Install all required dependencies
   - Verify your setup by building the project and running tests

## Branch Strategy

- `main` - Stable, production-ready code
- `develop` - Integration branch for features and fixes
- `feature/*` - Feature development branches
- `bugfix/*` - Bug fix branches
- `release/*` - Release preparation branches
- `hotfix/*` - Urgent fixes for production

## Development Workflow

1. **Create a new branch from `develop`**
   ```bash
   git checkout develop
   git pull origin develop
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Follow the coding standards outlined below
   - Write tests for new functionality
   - Keep commits small and focused

3. **Commit your changes**
   - Write clear, descriptive commit messages
   - Format: `[type]: Brief description of changes`
   - Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`
   - Example: `feat: Add passenger routing algorithm`

4. **Push your branch to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

5. **Submit a pull request**
   - Target the `develop` branch
   - Fill out the pull request template
   - Link any related issues
   - Request reviews from maintainers

## Commit Message Guidelines

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification for commit messages. This leads to more readable messages that are easy to follow when looking through the project history.

### Commit Message Format
```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types
- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation only changes
- `style`: Changes that do not affect the meaning of the code (white-space, formatting, etc)
- `refactor`: A code change that neither fixes a bug nor adds a feature
- `perf`: A code change that improves performance
- `test`: Adding missing tests or correcting existing tests
- `chore`: Changes to the build process or auxiliary tools and libraries
- `ci`: Changes to CI configuration files and scripts

### Scope
The scope should be the name of the component affected (e.g., `core`, `simulation`, `gui`, `docs`, etc.)

### Subject
- Use the imperative, present tense: "change" not "changed" nor "changes"
- Don't capitalize first letter
- No dot (.) at the end
- Maximum 72 characters

### Body
- Use the imperative, present tense
- Include motivation for the change and contrasts with previous behavior
- Wrap at 72 characters

### Footer
- Reference issues and pull requests that this commit closes
- Breaking changes should start with `BREAKING CHANGE:`

### Examples
```
feat(simulation): add passenger routing algorithm

Implement A* pathfinding for passenger route calculation.
- Add priority queue implementation
- Add path reconstruction
- Add distance heuristic function

Closes #123
```

```
fix(core): prevent crash when loading invalid map file

Add proper error handling and validation for map file format.
Previously the application would crash when encountering
malformed JSON.

Fixes #456
```

```
BREAKING CHANGE: change map file format to support new features

The map file format has been updated to version 2.0 to support
multi-level transportation networks. Old map files need to be
migrated using the provided conversion tool.

Migration guide: docs/migration-guide-v2.md
Closes #789
```

## Coding Standards

### C++ Coding Standards

- **Naming Conventions**
  - Classes: PascalCase (e.g., `TransportManager`)
  - Methods/Functions: camelCase (e.g., `calculateRoute()`)
  - Variables: camelCase (e.g., `busCapacity`)
  - Constants: UPPER_SNAKE_CASE (e.g., `MAX_PASSENGER_COUNT`)
  - Namespaces: lowercase (e.g., `transity::simulation`)
  - Files: snake_case.hpp/cpp (e.g., `transport_manager.hpp`)
  - Template parameters: PascalCase with 'T' prefix (e.g., `template <typename TValue>`)

- **Code Organization**
  - One class per file where appropriate
  - Group related functionality in namespaces
  - Use forward declarations to minimize header dependencies
  - Implement PIMPL idiom for complex classes to reduce compile times
  - Maximum line length of 100 characters

- **Comments and Documentation**
  - All public APIs must have doxygen-style documentation
  - Code comments should explain "why" not "what"
  - Each file should have a brief description at the top
  - Complex algorithms should include explanations and references

### Error Handling

- Use exceptions for exceptional conditions
- Use return values for expected failure states
- Implement logging for all error conditions
- Validate inputs at API boundaries

## Testing

- Write unit tests for all new functionality
- Ensure all tests pass before submitting a pull request
- Follow the Arrange-Act-Assert pattern
- One assertion per test
- Target 80%+ code coverage

## Pull Request Process

1. Ensure your code follows the project's coding standards
2. Update documentation as necessary
3. Add or update tests as necessary
4. Make sure all CI checks pass
5. Get at least one approval from a maintainer
6. Once approved, a maintainer will merge your PR

## Additional Resources

- [Project Technical Plan](/docs/transity_development_plan.md)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [SFML Documentation](https://www.sfml-dev.org/documentation.php)

Thank you for contributing to Transity! 