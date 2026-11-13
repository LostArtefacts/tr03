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

# Nonzero game data no source here defines; nothing the exe runs reads
# any of it.
ORIGINAL_DATA = [
    (0x4C5050, 0x4C5084, "Core's 3dglobal poly_draw_routines and constants; no such file here"),
    (0x4C5088, 0x4C508C, "unnamed, after one"),
    (0x4C70B4, 0x4C70B8, "Core's DoInventoryBackground static bz; no such function"),
    (0x4C7788, 0x4C77F8, "unnamed index table"),
    (0x4C7BD0, 0x4C7BE0, "Core's shark.c shark_bite; no such file here"),
    (0x4C7C00, 0x4C7C04, "unnamed, after shiva_right"),
    (0x4C8090, 0x4C80C0, "unnamed table, after BatLinks"),
    (0x4C85E8, 0x4C8628, "Core's LoadSamples static apStr, for a log call"),
    (0x4CD8F8, 0x4CD904, "packed shorts with DumpX, DumpY and DumpWidth; separate globals cannot pack"),
    (0x4CE35C, 0x4CE360, "unnamed, after selection"),
    (0x4CE6A8, 0x4CE708, "Core's utils.cpp static month names, for a log call"),
    (0x4CEB68, 0x4CEB6C, "unnamed float, after MagicFix24"),
    (0x4CEB78, 0x4CEB7C, "unnamed float, after SubSpanLength"),
    (0x4CEFE8, 0x4CF050, "unnamed table, after AWCS"),
]


def link_exe(objects: list[str], exe_objects: list[str], feats: list[str]) -> None:
    orig_layout.link(exe_objects, extra_rows=STATIC_INITIALIZERS + STATIC_DATA, runtime=["libc.lib"],
                     data_libraries=["dxguid.lib"], original_data=None if feats else ORIGINAL_DATA)
