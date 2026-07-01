# Third-Party Notices

## 1) bacnet-stack

Upstream project: https://github.com/bacnet-stack/bacnet-stack

Primary upstream authors:
- Steve Karg <skarg@users.sourceforge.net>
- Peter McShane <petermcs@users.sourceforge.net>
- John Minack <minack@users.sourceforge.net>
- Nikola Jelic <nikola.jelic@euroicc.com>
- Additional contributors listed in the upstream repository

Scope in this repository:
- `src/bacnet/` (upstream protocol stack files, some unused files disabled)
- Derived/adapted files in `src/app/`
- Derived/adapted file `src/platform/dlmstp.c`

Licenses: GPL-2.0-or-later WITH GCC-exception-2.0 and MIT (per-file SPDX)
Local full text: `LICENSES/GPL-2.0-or-later.txt`

## 2) Original wrapper and platform integration work

Copyright (c) 2025-2026 George Arun
License: MIT (see `LICENSE`)

Scope: `src/BACnetMSTP.h`, `src/compile_config.h`, `src/app/`, and
Arduino integration and board configuration in selected `src/platform/` files.
