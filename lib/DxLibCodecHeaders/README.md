# Codec interface headers for the custom DxLib build

Only public headers are vendored here. Existing codec binaries remain in
`lib/DxLib_unk`; no additional codec/runtime is installed. These headers let us
compile the supplied `DxUseCLibOgg` adapters against the custom core's actual
internal graphics types instead of mixing incompatible precompiled adapters.

Sources (unmodified public headers from the indicated upstream tags):

- [libogg v1.3.5](https://github.com/xiph/ogg/tree/v1.3.5/include/ogg): `ogg/*.h`
- [libvorbis v1.3.7](https://github.com/xiph/vorbis/tree/v1.3.7/include/vorbis): `vorbis/*.h`
- [libtheora v1.1.1](https://github.com/xiph/theora/tree/v1.1.1/include/theora): `theora/theora.h`
- [libopus v1.3.1](https://github.com/xiph/opus/tree/v1.3.1/include): `opus*.h` except `opusfile.h`
- [libopusfile v0.12](https://github.com/xiph/opusfile/tree/v0.12/include): `opusfile.h`

Copyright notices remain in each header. Corresponding licenses are the five
`COPYING-*.txt` files in this directory (BSD-style terms).
