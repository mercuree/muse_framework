/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MUSE_IO_FILESYNC_H
#define MUSE_IO_FILESYNC_H

#include "global/types/ret.h"

/**
 * @brief Synchronizes buffered file descriptor data and metadata to physical storage.
 *
 * Platform implementation details:
 * - Windows: Uses _commit(handle).
 * - macOS/Apple: Uses fcntl(handle, F_FULLFSYNC) with fallback to fsync(handle).
 * - Linux/POSIX: Uses fsync(handle) with retry loop on EINTR.
 *
 * Error handling:
 * - Unsupported sync operations on virtual or network filesystems (EINVAL, ENOTSUP)
 *   are logged as warnings and treated as non-fatal.
 * - Hardware or I/O storage failures (ENOSPC, EIO, etc.) return Err::FSWriteError
 *   with the OS error description.
 *
 * @param handle Valid OS file descriptor / CRT handle.
 * @return Ret indicating success or failure.
 */
muse::Ret flushToDisk(int handle);

#endif // MUSE_IO_FILESYNC_H
