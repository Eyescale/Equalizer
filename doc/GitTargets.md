# Git Targets

This module adds the following git release management targets:

## branch

Creates a new branch for developing the current version. The branch is
name MAJOR.MINOR, where the minor version is rounded up to the next even
version. Odd minor numbers are considered development versions, and
might still be used when releasing a pre-release version (e.g., 1.3.9
used for 1.4-beta).

The branch is pushed to the origin.

## cut

Delete the current version branch locall and remote.

## tag

Creates the version branch if needed, and creates a tag release-VERSION
on the version branch HEAD. Pushes tag to the origin repository.

## erase

Removes the release-VERSION locally and remotely.
