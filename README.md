# Thesis Pipeline

Synthetic processing pipeline for the thesis experiment, cross-compiled for the i.MX 8M Plus target using the toolchain file in `cmake/imx8mp-toolchain.cmake`.

## Build

```bash
cmake -B .build -S .
cmake --build .build
```

The resulting binary will be at `.build/thesis-pipeline`.

## Clean rebuild

```bash
rm -rf .build
cmake -B .build -S .
cmake --build .build
```
