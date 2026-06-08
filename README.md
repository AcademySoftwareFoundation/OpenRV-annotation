# OpenRV-annotation

OpenRV-annotation is a C++ annotation rendering library. It provides triangle-ribbon path geometry (`TwkPaint::Path`), physics-based input smoothing (`TwkPaint::Smoother`), and stamp-based brush placement (`TwkPaint::StampPath`) for review annotation tools. Used natively by [OpenRV](https://github.com/AcademySoftwareFoundation/OpenRV) and compiled to WebAssembly for Creative Review via [OpenRV-annotation-wasm](https://github.com/AcademySoftwareFoundation/OpenRV-annotation-wasm).

## Requirements

- C++17 compiler (Clang, GCC, or MSVC)
- CMake 3.19+

`TwkMath` is included in this repository. [Imath](https://github.com/AcademySoftwareFoundation/Imath) is fetched via CMake FetchContent at configure time. There are no other dependencies.

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

## Tests

```sh
cd build
ctest -C Release --output-on-failure
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). All contributions require a Developer Certificate of Origin (DCO) `Signed-off-by:` line in commit messages.

## License

Apache-2.0. See [LICENSE](LICENSE) for details.
