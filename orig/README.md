Supply your own pristine Steam Tomb Raider III game directory at
`orig/tomb3`, either as a directory or a local symlink. The executable is
verified against `tomb3.sha1` before it is copied to `build/orig/tomb3.exe`.
Game files are ignored by Git.

See the root README for the local compiler, SDK and reference layout.

## MASM

3DSYSTEM's hand-written assembly is assembled with MASM 6.11, as Core
did (`ml /c /coff`). From the MASM 6.11 disks, `DISK1/BIN/ML.EX$` and
`ML.ER$` are KWAJ-compressed, which libmspack expands. Put them in the
prefix as `C:\masm611\bin\ML.EXE` and `ML.ERR`.
