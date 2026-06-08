# Contributing to OpenRV-annotation

Thank you for your interest in contributing to OpenRV-annotation!

## Developer Certificate of Origin (DCO)

All contributions to this project must be accompanied by a `Signed-off-by:` line
in the commit message. This certifies that you wrote the contribution or have the
right to submit it under the project's open-source license.

To sign off on a commit, add `-s` to your `git commit` command:

```sh
git commit -s -m "Your commit message"
```

This adds a line like:

```
Signed-off-by: Your Name <your.email@example.com>
```

By making a contribution to this project, you certify that:

```
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.


Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

## Pull Request Process

1. Fork the repository and create a branch from `main`.
2. Make your changes, ensuring all existing tests pass and new tests are added where appropriate.
3. Sign off on all commits with `git commit -s`.
4. Open a pull request against the `main` branch with a clear description of what the change does and why.
5. A maintainer will review your PR. Please be responsive to feedback.

## Code Style

This project uses C++17. Source files are formatted with `clang-format` using the
configuration in [`.clang-format`](.clang-format) at the repository root.

Before submitting:

```sh
make format-check  # check without modifying
make format        # fix in place
```

The pre-commit hook in `hooks/pre-commit` will automatically check formatting on
staged files. Install it with:

```sh
make install-hooks
```

## Bug Reports and Feature Requests

Please use [GitHub Issues](https://github.com/AcademySoftwareFoundation/OpenRV-annotation/issues)
to report bugs or request features. When filing a bug, include:

- A clear description of the issue
- Steps to reproduce
- Expected and actual behavior
- Platform, compiler, and CMake version
