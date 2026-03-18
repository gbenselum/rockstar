# Rockstar GitFlow Branching Strategy

This project strictly adheres to the **GitFlow** branching model to manage releases, features, and fixes cleanly.

## 1. Main Branches
The core of the repository consists of two infinite lifetime branches:
* **`main` (or `master`)**: This branch contains the official release history. The code here is always considered production-ready. Every commit on `main` should be tagged with a version number (e.g., `v1.0.0`).
* **`develop`**: This is the integration branch for features. It serves as the daily working branch. When the source code in `develop` reaches a stable point and is ready to be released, all changes should be merged back into `main` and tagged.

## 2. Supporting Branches
To aid parallel development, we use various short-lived supporting branches. 

### Feature Branches (`feature/*`)
* **Branched from**: `develop`
* **Must merge back into**: `develop`
* **Naming convention**: `feature/your-new-feature`
* **Purpose**: Used for developing new features for upcoming releases. Features should only exist in developers' local repositories or pushed to origin when collaborative work is needed.

### Release Branches (`release/*`)
* **Branched from**: `develop`
* **Must merge back into**: `develop` AND `main`
* **Naming convention**: `release/vX.Y.Z` (e.g., `release/v1.2.0`)
* **Purpose**: Dedicated to preparing a new production release. They allow for minor bug fixes and metadata preparation (version numbers, changelogs) without blocking new features from being pushed to `develop`.

### Hotfix Branches (`hotfix/*`)
* **Branched from**: `main`
* **Must merge back into**: `develop` AND `main`
* **Naming convention**: `hotfix/vX.Y.Z` (e.g., `hotfix/v1.0.1`)
* **Purpose**: Used for quick patching of production releases. When a critical bug is found on `main`, a hotfix branch is spawned directly from the `main` tag containing the bug.

## Workflow Example
1. You want to build a new EQ type. Create a branch: `git checkout -b feature/new-eq develop`.
2. Commit your code: `git commit -m "Added graphical EQ"`.
3. Merge it back to develop via Pull Request.
4. When ready to launch v1.1.0, branch off `develop`: `git checkout -b release/v1.1.0 develop`.
5. Fix release bugs, then merge `release/v1.1.0` into `main` (and tag it), and also merge it back into `develop`.
