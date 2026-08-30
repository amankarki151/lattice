"""Readers for the .fvecs / .ivecs format SIFT ships in.

Layout is the same for both: each record is a 4-byte little-endian
int32 dimension count, followed by that many values - float32 for
fvecs, int32 for ivecs. Records sit back to back with no header and
no separator.

The dimension is repeated in every single record, which is wasteful
but does mean you can read the file without knowing anything about it
up front.
"""

import numpy as np


def read_fvecs(path: str) -> np.ndarray:
    """Read a .fvecs file into an (n, dim) float32 array."""
    raw = np.fromfile(path, dtype=np.int32)
    if raw.size == 0:
        return np.zeros((0, 0), dtype=np.float32)

    dim = int(raw[0])
    # Each record is 1 int32 for the dim + dim values.
    record_size = dim + 1
    if raw.size % record_size != 0:
        raise ValueError(
            f"{path}: size {raw.size} is not a multiple of record size "
            f"{record_size} - file may be truncated or the wrong format"
        )

    count = raw.size // record_size
    # Reshape, then drop the leading dim field from every row.
    return raw.reshape(count, record_size)[:, 1:].view(np.float32).copy()


def read_ivecs(path: str) -> np.ndarray:
    """Read an .ivecs file into an (n, dim) int32 array."""
    raw = np.fromfile(path, dtype=np.int32)
    if raw.size == 0:
        return np.zeros((0, 0), dtype=np.int32)

    dim = int(raw[0])
    record_size = dim + 1
    if raw.size % record_size != 0:
        raise ValueError(
            f"{path}: size {raw.size} is not a multiple of record size "
            f"{record_size}"
        )

    count = raw.size // record_size
    return raw.reshape(count, record_size)[:, 1:].copy()