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
#include "filesync.h"

#include <cerrno>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#elif defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

#include "../ioretcodes.h"
#include "log.h"

muse::Ret flushToDisk(int handle)
{
    if (handle < 0) {
        muse::Ret ret = muse::io::make_ret(muse::io::Err::FSWriteError);
        ret.setText("Invalid file handle");
        return ret;
    }

#if defined(_WIN32)
    if (_commit(handle) < 0) {
        // EINVAL can occur on network shares or virtual devices where flushing is not supported
        if (errno == EINVAL) {
            LOGW() << "File system does not support commit to disk (EINVAL)";
            return muse::make_ok();
        }

        muse::Ret ret = muse::io::make_ret(muse::io::Err::FSWriteError);
        ret.setText(std::string("Failed to commit file to disk: ") + std::strerror(errno));
        return ret;
    }
#elif defined(__APPLE__)
    int res = -1;
    // On macOS, F_FULLFSYNC instructs the controller to flush dirty pages to physical media
    while ((res = fcntl(handle, F_FULLFSYNC)) < 0 && errno == EINTR) {}

    // Fallback to fsync if F_FULLFSYNC is not supported by the underlying filesystem (e.g. SMB/NFS)
    if (res < 0 && (errno == ENOTSUP || errno == EINVAL)) {
        while ((res = fsync(handle)) < 0 && errno == EINTR) {}
    }

    if (res < 0) {
        // Both F_FULLFSYNC and fsync fallback failed with an unsupported error
        if (errno == ENOTSUP || errno == EINVAL) {
            LOGW() << "File system does not support sync to disk: " << std::strerror(errno);
            return muse::make_ok();
        }

        muse::Ret ret = muse::io::make_ret(muse::io::Err::FSWriteError);
        ret.setText(std::string("Failed to sync file to disk: ") + std::strerror(errno));
        return ret;
    }
#else
    int res = -1;
    while ((res = fsync(handle)) < 0 && errno == EINTR) {}

    if (res < 0) {
        if (errno == ENOTSUP || errno == EINVAL) {
            LOGW() << "File system does not support sync to disk: " << std::strerror(errno);
            return muse::make_ok();
        }

        muse::Ret ret = muse::io::make_ret(muse::io::Err::FSWriteError);
        ret.setText(std::string("Failed to sync file to disk: ") + std::strerror(errno));
        return ret;
    }
#endif

    return muse::make_ok();
}
