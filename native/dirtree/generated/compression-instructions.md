To get a build with compressed dirtrees, do the following:
- Build SenTools via CMake.
- Invoke `sentools DirTree.Create` on every .json file in this folder to generate several `dirtree-*.bin` files.
- Run `sentools FileCompressor` on each `dirtree-*.bin` file.
- Rename the output files to `compressed-dirtree-*.bin`, eg. `compressed-dirtree-cs1.bin`, and put them into the `native/dirtree/generated` folder.
- Re-run CMake. In Visual Studio that's under Project -> Configure SenPatcher.
- Rebuild SenTools.
