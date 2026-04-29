# Directory Opus 5 — All Amigas

A maintained continuation of **Directory Opus 5 Magellan II** for AmigaOS,
MorphOS, and AROS.

The `dopus5allamigas` fork builds on the 2012 APL source release with
GCC/cross-toolchain support, platform fixes, and ongoing module work.

## Supported platforms

- **AmigaOS 3** (m68k, including OS3.9/OS3.2)
- **AmigaOS 4** (PPC)
- **MorphOS** (PPC)
- **AROS** (i386 and ARM)

## Downloads

Pre-built binaries are published under [Releases](../../releases).

## Building

The project builds with GCC-based cross-compilation toolchains for each target
platform. CI uses the
[`sacredbanana/amiga-compiler`](https://hub.docker.com/r/sacredbanana/amiga-compiler)
Docker images for AmigaOS 3, AmigaOS 4, and MorphOS.

General flow (inside the toolchain environment):

```sh
cd source
make os3        # AmigaOS 3
make os4        # AmigaOS 4
make mos        # MorphOS
make i386-aros  # AROS/i386
make arm-aros   # AROS/ARM
```

Release archives are built from the same makefile:

```sh
cd source
make os3 release debug=no
```

The FTP module can be built with SFTP support by passing `sftp=yes`. For
AmigaOS 3, AmigaOS 4, and MorphOS this uses the bundled libssh2 build helper by
default.

```sh
cd source
make os3 sftp=yes
make os3 release debug=no sftp=yes
```

Consult `source/makefile` and the module makefiles for the full list of
targets and options.

## Testing

The FTP parser/protocol tests run on a normal Unix-like host:

```sh
make -C source/Modules/ftp -f makefile.tests test
```

If libssh2 development headers are installed, the SFTP backend tests can also
be run:

```sh
make -C source/Modules/ftp -f makefile.tests test-libssh2
```

## Project Status

Active development happens in this repository. See [`ChangeLog`](ChangeLog)
for the change history, [Releases](../../releases) for packaged builds, and
[GitHub Issues](../../issues) for open work and bug reports.

## License

Directory Opus 5 is released under the **AROS Public License (APL) version
1.1** — see [`COPYING`](COPYING).

Optional builds may include or link third-party components. In particular, the
FTP module's SFTP support built with `sftp=yes` uses libssh2 and zlib; see
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

"Directory Opus" is a registered trademark of GP Software. The APL trademark
licence limits use of the mark to Amiga-like platforms (AmigaOS, MorphOS,
AROS, and compatibles); use on Windows, macOS, Linux, etc. is **not**
permitted.

The release under the APL does **not** affect the commercial status of
*Directory Opus for Windows*. See <https://www.gpsoft.com.au> for that
product.

## Credits

- **Jonathan Potter / GP Software** — original Directory Opus 5 / Magellan II
  (1993-2012)
- **DOPUS5 Open Source Team** — initial post-APL GCC ports and platform work
  for OS3 / OS4 / MorphOS / AROS (2012-2013)
- **Dimitris Panokostas** ([@midwan](https://github.com/midwan)) —
  `dopus5allamigas` fork maintainer (2023-present)
- All other contributors listed in [`AUTHORS`](AUTHORS)

Special thanks to Olaf Schönweiß and the
[power2people.org](https://www.power2people.org) contributors who funded the
original open-source release.

## Legacy

The original 2012 APL release README is preserved at
[`README.legacy`](README.legacy).

## Links

- **Issue tracker:** [GitHub Issues](../../issues)
- **Historical project site:** <https://sourceforge.net/p/dopus5allamigas/>
- **Directory Opus for Windows (commercial):** <https://www.gpsoft.com.au>
