# Third-party notices

CS2AC uses the following third-party software. Each project remains under its own license and copyright.

- [Metamod:Source](https://github.com/alliedmodders/metamod-source), pinned as a Git submodule. License text is included in `metamod-source/LICENSE.txt`.
- [Source 2 SDK](https://github.com/alliedmodders/hl2sdk), CS2 branch, pinned as a Git submodule. Individual SDK files retain their Valve and contributor notices.
- [AMBuild](https://github.com/alliedmodders/ambuild), downloaded at the commit pinned in the bootstrap scripts. It is licensed under the BSD 3-Clause License.
- [Funchook](https://github.com/kubo/funchook/tree/7cb8819594f0d586454011ab691fab4edb625068), vendored as headers and prebuilt x64 libraries. Funchook is licensed under GNU GPL version 2 or later with its documented linking exception. Its exact license text is included in `licenses/FUNCHOOK.txt`.
- [diStorm 3.5.2b](https://github.com/gdabah/distorm/tree/3.5.2b), included in the vendored Funchook libraries. It is licensed under the BSD 3-Clause License. Its exact license text is included in `licenses/DISTORM.txt`.
- [tinyformat](https://github.com/c42f/tinyformat), vendored as a header. It is licensed under the Boost Software License 1.0.
- [Protocol Buffers](https://github.com/protocolbuffers/protobuf), supplied by the pinned Source 2 SDK and generated during the build. It is licensed under the BSD 3-Clause License.

The upstream links above identify the corresponding source used by the vendored components. Release archives include the applicable license texts under `addons/cs2ac/licenses`.
