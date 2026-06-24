# Third-Party Notices

This repository contains both original code and third-party derived code.

## 1) bacnet-stack

Upstream project:
- https://github.com/bacnet-stack/bacnet-stack

Primary upstream authors include:
- Steve Karg <skarg@users.sourceforge.net>
- Peter McShane <petermcs@users.sourceforge.net>
- John Minack <minack@users.sourceforge.net>
- Nikola Jelic <nikola.jelic@euroicc.com>
- Additional contributors listed in the upstream repository

Scope in this repository:
- `src/bacnet/` (upstream protocol stack files)
- Derived/adapted files in `src/app/`
- Derived/adapted file `src/platform/dlmstp.c`

License model for these files:
- Per-file SPDX identifiers and header copyright blocks are authoritative.
- Common licenses in this scope are:
  - `GPL-2.0-or-later WITH GCC-exception-2.0`
  - `MIT`

Local full text copies:
- `LICENSES/GPL-2.0-or-later.txt`

## 2) Original wrapper and platform integration work in this repository

Copyright:
- Copyright (c) 2025-2026 George Arun

Primary scope (non-exhaustive):
- `src/BACnetMSTP.h`
- `src/BACnetStack.cpp`
- `src/wrapper/`
- Arduino integration and board configuration glue in selected `src/platform/` files

License:
- MIT (see `LICENSE`)

## Attribution policy

- Upstream notices are preserved in upstream/derived files.
- Repository-level MIT notice covers original work contributed in this repository.
- Where a file contains upstream code plus local modifications, both attributions must remain.