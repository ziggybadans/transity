# Branch Protection Rules

This document outlines the branch protection rules that should be applied to the Transity repository to ensure code quality and maintain a stable codebase.

## Protected Branches

The following branches should be protected:

1. **`main`** - Production-ready code
2. **`develop`** - Integration branch for features and bug fixes

## Protection Settings

### Main Branch Protection

For the `main` branch, apply the following settings in GitHub:

- **Require pull request reviews before merging**: Enabled
  - **Required approving reviews**: 2
  - **Dismiss stale pull request approvals when new commits are pushed**: Enabled
  - **Require review from Code Owners**: Enabled

- **Require status checks to pass before merging**: Enabled
  - **Require branches to be up to date before merging**: Enabled
  - **Required status checks**:
    - Transity CI / Windows MSVC
    - Transity CI / Ubuntu GCC
    - Transity CI / macOS Clang
    - code-coverage

- **Require signed commits**: Enabled

- **Require linear history**: Enabled

- **Include administrators**: Enabled

- **Restrict who can push to matching branches**: Enabled
  - Only allow specific people or teams to push directly to `main`

### Develop Branch Protection

For the `develop` branch, apply the following settings:

- **Require pull request reviews before merging**: Enabled
  - **Required approving reviews**: 1
  - **Dismiss stale pull request approvals when new commits are pushed**: Enabled
  - **Require review from Code Owners**: Enabled

- **Require status checks to pass before merging**: Enabled
  - **Require branches to be up to date before merging**: Enabled
  - **Required status checks**:
    - Transity CI / Windows MSVC
    - Transity CI / Ubuntu GCC

- **Require signed commits**: Enabled

- **Require linear history**: Disabled

- **Include administrators**: Enabled

## Setting Up Branch Protection Rules

1. Go to the repository settings on GitHub
2. Navigate to "Branches"
3. Under "Branch protection rules", click "Add rule"
4. Enter the branch name pattern (e.g., `main` or `develop`)
5. Configure the protection settings as outlined above
6. Click "Create" or "Save changes"

## Code Owners Configuration

Create a `CODEOWNERS` file in the `.github` directory with the following content:

```
# These owners will be the default owners for everything in the repo
* @repository-owner

# Core engine files
/src/core/ @engine-team

# Simulation systems
/src/simulation/ @simulation-team

# Rendering systems
/src/rendering/ @graphics-team

# Build system
/cmake/ @build-team
```

Replace the placeholder team names with actual GitHub teams or usernames. 