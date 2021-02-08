# Minefield
## Compiler
The folder `compiler` contains the patch for LLVM to build the Clang compiler with the Minefield extension.
To build the compiler use the `clone_and_build.sh` shell script.

## The SGX-SDK
The folder `sdk` contains the patch to build the SGX-SDK.
Run the `checkout-and-patch.sh` to clone the SDK and apply the patch.
Then follow the build instructions from the SGX SDK version 2.10 [here](https://github.com/intel/linux-sgx/tree/sgx_2.10_reproducible).

## Proof of Concepts
The folder `pocs` contains the example of the `imul` instruction in the `imul` folder and a template to build sgx enclaves in the `sgx-template` folder.