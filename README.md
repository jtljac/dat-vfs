# Dat VFS
A library for creating a virtual filesystem intended to abstract multiple storage locations into a single filesystem for
easy access.

## Repo Structure
### Library
The code for this library can be found in the [source](./source/) and [include](./include/) directories,
which contain the cpp implementation and header files respectively.

The library can be easily added to a cmake project by using `add_subdiretory` with the root directory of this repo and
linking with: `target_link_libraries(your-project dat-vfs)`.

### Tests
Testing can be enabled by setting `DATVFS_ENABLE_TESTS` to `ON`, this will then build the tests and make them available

When tests are enabled, the project will depend on [catch2](https://github.com/catchorg/Catch2) and will not build 
without it.