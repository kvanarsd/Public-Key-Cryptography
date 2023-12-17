#Asignment 5: Public Key Cryptography

## Description:
This program contains an implementation of an SS cryptographic algorithm. It contains three different programs: keygen, encrypt, decrypt. Keygen creates a public and private key and stores them in different files. Encrypt uses the file containing the public key to encrypt a provided file. Decrypt takes in the encrypted file and outputs the decrypted file using the corresponding private key. 

## Build:
Make sure the supporting function files, ss.c, randstate.c, numtheory.c, and their headers, ss.h, randstate.h, numtheory.h, are in the directory. Along with the main files keygen.c, encrypt.c, decrypt.c, and the Makefile. Calling 'make' or 'make all' will create the executables: keygen, encrypt, and decrypt. If you only want to create one executable you can call 'make keygen', 'make encrypt', or 'make decrypt' to make the corresponding executables. 

## Cleaning:
Calling 'make clean' will remove all made executables and .o files from the directory. 

## Running:
Calling any of the executables with -h will print the usage, './keygen -h' for example will print the usage for keygen. 

## Running keygen:
Keygen's valid arguments are 'b:i:n:d:s:vh'. -b specifies the minimum bits need for modulus n; -b must be called with a number argument (default is 256). -i specifies the number of iterations used for testing primes, it must be called with a number argument(default is 50). -n specifies the file the public key will be saved in, it must be called with a file name (default is ss.pub). -d specifies the file the private key will be saved in, it must be called with a file name (default is ss.priv). -s called with any number specifies the random seed. -v enables verbose output. -h prints the usage.

## Running encrypt:
Encrypt's valid arguments are 'i:o:n:vh'. -n specifies the file containing the public key, it must be called with a file name (default is ss.pub). -i specifies the file to encrypt, it must be called with a file name (default is stdin). -o specifies the file to output encrypt, it must be called with a file name (default is stdout). -v enables verbose output. -h prints the usage.

## Running decrypt:
Decrypt's valid arguments are 'i:o:n:vh'. -n specifies the file containing the private key, it must be called with a file name (default is ss.priv). -i specifies the file to decrypt, it must be called with a file name (default is stdin). -o specifies the file to output decrypt, it must be called with a file name (default is stdout). -v enables verbose output. -h prints the usage.

## Known Errors;
Calling keygen with minimum bits < 4 will cause a 'Floating point exception (core dumped)' error.
If n has less than 50 bits encrypt will return nothing. Calling keygen with -b and a number >= 50 will resolve this problem. Sometimes calling -b 49 or 48 can produce a modulus with bits >= 50 which will not cause a problem.

