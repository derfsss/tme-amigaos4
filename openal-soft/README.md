# openal-soft — OpenAL Soft 1.25.1 with AHI Backend

**Source:** axmol's bundled `3rdparty/openal/` (OpenAL Soft 1.25.1)
**Reference:** https://github.com/salass00/openal_lib (1.18.2 with AHI backend, newlib — source reference only)
**Runtime:** clib4 (`-mcrt=clib4`)

## What

Port axmol's bundled OpenAL Soft 1.25.1 to AmigaOS4 by adding a native AHI
audio output backend. This keeps axmol's AudioEngine code completely unchanged.

## Work required

1. Add AHI backend implementing OpenAL Soft's backend interface:
   - Double-buffered AHI requests via IExec->OpenDevice(AHINAME, ...)
   - Mixer runs as separate AmigaOS process
   - Mono/stereo, 8/16/32-bit PCM output
2. Fix TLS for AmigaOS4 (thread_local disabled in GCC 11.5):
   - Custom TLS shim (reference salass00's common/threads.c)
3. Stub audio capture (not needed for games)
4. Build with clib4 — salass00's port is newlib, cannot link against it

## Known issues

salass00's 1.18.2 had a threading freeze bug with concurrent SFX + music
(issue #1). Address in 1.25.1 with proper mutex/condition variable handling.
clib4's pthreads are more complete than newlib's.
