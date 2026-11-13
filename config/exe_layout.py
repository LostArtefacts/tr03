"""Tomb3's standalone link, through the shared original-address layout."""

import orig_layout

STATIC_INITIALIZERS = [
    (0x4752C0, 10, ["specific/di.cpp:_$E4"]),
    (0x475300, 104, ["specific/di.cpp:_$E2"]),
    (0x475370, 10, ["specific/di.cpp:_$E9"]),
    (0x4753D0, 47, ["specific/di.cpp:_$E7"]),
    (0x4804C0, 10, ["specific/ds.cpp:_$E4"]),
    (0x480500, 104, ["specific/ds.cpp:_$E2"]),
    (0x480570, 10, ["specific/ds.cpp:_$E9"]),
    (0x4805D0, 47, ["specific/ds.cpp:_$E7"]),
]

STATIC_DATA = [
    (0x633C08, 128, ["_?path@?1??GetFullPath@@9@4PADA"]),
]


def link_exe(objects: list[str], exe_objects: list[str], feats: list[str]) -> None:
    orig_layout.link(exe_objects, extra_rows=STATIC_INITIALIZERS + STATIC_DATA, runtime=["libc.lib"])
