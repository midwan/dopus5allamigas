# Directory Opus 5 — All Amigas

A modern, actively-maintained fork of **Directory Opus 5 Magellan II**, the legendary Amiga file manager, ported and updated for all Amiga-like platforms.

This fork (`dopus5allamigas`) picks up where the 2012 APL open-source release left off, adding bug fixes, new features, and continued platform support.

## Supported platforms

- **AmigaOS 3** (m68k, including OS3.9/OS3.2)
- **AmigaOS 4** (PPC)
- **MorphOS** (PPC)
- **AROS** (i386, ARM, and other architectures)

## Downloads

Pre-built binaries are published under [Releases](../../releases). The `releases/` folder in this repository also contains historical archives (`Dopus5_94_*.lha`, `Dopus5_95_*.lha`, ...).

## Building

The project builds with cross-compilation toolchains for each target platform. If you don't have a full toolchain installed locally, the easiest option is the [`sacredbanana/amigaos-cross-compiler`](https://hub.docker.com/r/sacredbanana/amigaos-cross-compiler) Docker image which bundles all four toolchains.

General flow (inside the toolchain environment):

```sh
cd source
make -f makefile OS=os3    # AmigaOS 3
make -f makefile OS=os4    # AmigaOS 4
make -f makefile OS=mos    # MorphOS
make -f makefile OS=aros   # AROS (add ARCH=i386 / ARCH=arm as appropriate)
```

Consult `source/makefile` for the full list of targets and options.

## Status

Active development. See [`ChangeLog`](ChangeLog) for the running history of fixes and features. Recent work includes:

- MorphOS PPC startup / module dispatch fixes
- Live filter overlay (View menu → Filter...)
- Pipelined icon population on backdrop / listers
- Memory and resource leak closures ported from the Galileo fork
- Ongoing OS4 / AROS compatibility work

## License

Directory Opus 5 is released under the **AROS Public License (APL) version 1.1** — see [`COPYING`](COPYING).

"Directory Opus" is a registered trademark of GP Software. The APL trademark licence limits use of the mark to Amiga-like platforms (AmigaOS, MorphOS, AROS, and compatibles); use on Windows, macOS, Linux, etc. is **not** permitted.

The release under the APL does **not** affect the commercial status of *Directory Opus for Windows*. See <https://www.gpsoft.com.au> for that product.

## Credits

- **Jonathan Potter / GP Software** — original Directory Opus 5 / Magellan II (1993-2012)
- **DOPUS5 Open Source Team** — initial GPL port to OS3 / OS4 / MorphOS / AROS (2012-2013)
- **Dimitris Panokostas** ([@midwan](https://github.com/midwan)) — `dopus5allamigas` fork maintainer (2023-present)
- All other contributors listed in [`AUTHORS`](AUTHORS)

Special thanks to Olaf Schönweiß and the [power2people.org](https://www.power2people.org) contributors who funded the original open-source release.

## Legacy

The original 2012 APL release README is preserved at [`README.legacy`](README.legacy).

## Links

- **Issue tracker:** [GitHub Issues](../../issues)
- **Historical project site:** <https://sourceforge.net/p/dopus5allamigas/>
- **Directory Opus for Windows (commercial):** <https://www.gpsoft.com.au>
