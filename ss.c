#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

// Generates the components for a new SS key.
//
// Provides:
//  p:  first prime
//  q: second prime
//  n: public modulus/exponent
//
// Requires:
//  nbits: minimum # of bits in n
//  iters: iterations of Miller-Rabin to use for primality check
//  all mpz_t arguments to be initialized
//
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    mpz_t d1, d2, temp;
    mpz_inits(d1, d2, temp, NULL);

    // choose number of bits for p and q
    uint64_t low = nbits / 5;
    uint64_t up = ((2 * nbits) / 5) - low;
    uint64_t pbits = (rand() % up) + low;
    uint64_t qbits = nbits - (2 * pbits);

    //add one to make n at least nbits
    pbits += 1;
    qbits += 1;

    //generate p
    make_prime(p, pbits, iters);
    //generate q
    make_prime(q, qbits, iters);

    while (true) {
        mpz_sub_ui(temp, q, 1);
        gcd(d1, p, temp);
        mpz_sub_ui(temp, p, 1);
        gcd(d2, q, temp);
        // if p | q -1 or q | p - 1 generate new primes
        if (mpz_cmp(d1, p) == 0 || mpz_cmp(d2, q) == 0) {
            //regenerate the primes
            make_prime(p, pbits, iters);
            make_prime(q, qbits, iters);
        } else {
            break;
        }
    }

    //make public key
    mpz_mul(n, p, p);
    mpz_mul(n, n, q);

    mpz_clears(d1, d2, temp, NULL);
    return;
}

// Generates components for a new SS private key.
//
// Provides:
//  d:  private exponent
//  pq: private modulus
//
// Requires:
//  p:  first prime number
//  q: second prime number
//  all mpz_t arguments to be initialized
//
void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {
    mpz_t p1, q1, lcm, gcd_lam, n;
    mpz_inits(p1, q1, lcm, gcd_lam, n, NULL);
    // setting variables p - 1 and q - 1 and pq
    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);
    mpz_mul(pq, p, q);

    // lcm  =  (p - 1)(q - 1) / gcd(p - 1)(q - 1)
    mpz_mul(d, p1, q1);
    gcd(gcd_lam, p1, q1);
    mpz_fdiv_q(lcm, d, gcd_lam);
    // d = mod_inverse(n, lcm)
    mpz_mul(n, pq, p);
    mod_inverse(d, n, lcm);

    mpz_clears(p1, q1, lcm, gcd_lam, n, NULL);
    return;
}

// Export SS public key to output stream
//
// Requires:
//  n: public modulus/exponent
//  username: login name of keyholder ($USER)
//  pbfile: open and writable file stream
//
void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    // print n into file
    gmp_fprintf(pbfile, "%Zx\n", n);

    // Allocate space for username and print it
    size_t len = LOGIN_NAME_MAX + 1;
    char *user = malloc(len * sizeof(char));
    strncpy(user, username, len);
    fprintf(pbfile, "%s\n", user);
    free(user);
    return;
}

//
// Export SS private key to output stream
//
// Requires:
//  pq: private modulus
//  d:  private exponent
//  pvfile: open and writable file stream
//
void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", pq);
    gmp_fprintf(pvfile, "%Zx\n", d);
    return;
}

//
// Import SS public key from input stream
//
// Provides:
//  n: public modulus
//  username: $USER of the pubkey creator
//
// Requires:
//  pbfile: open and readable file stream
//  username: requires sufficient space
//  all mpz_t arguments to be initialized
//
void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n", n);
    fscanf(pbfile, "%s\n", username);
    return;
}

//
// Import SS private key from input stream
//
// Provides:
//  pq: private modulus
//  d:  private exponent
//
// Requires:
//  pvfile: open and readable file stream
//  all mpz_t arguments to be initialized
//
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n", pq);
    gmp_fscanf(pvfile, "%Zx\n", d);
    return;
}

//
// Encrypt number m into number c
//
// Provides:
//  c: encrypted integer
//
// Requires:
//  m: original integer
//  n: public exponent/modulus
//  all mpz_t arguments to be initialized
//
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
    return;
}
//
// Encrypt an arbitrary file
//
// Provides:
//  fills outfile with the encrypted contents of infile
//
// Requires:
//  infile: open and readable file stream
//  outfile: open and writable file stream
//  n: public exponent and modulus
//
void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {
    mpz_t m, c;
    mpz_inits(m, c, NULL);

    //calculate block size k
    uint64_t k = ((mpz_sizeinbase(n, 2) / 2) - 1) / 8;
    uint8_t *kbytes = malloc(k * sizeof(uint8_t));
    memset(kbytes, 0, k);
    kbytes[0] = 0xFF;

    while (fread(kbytes + 1, sizeof *kbytes, k - 2, infile) > 0) {
        // j is number of read bytes
        uint64_t j = (uint64_t) strlen((const char *) kbytes);
        mpz_import(m, j, 1, sizeof(kbytes[0]), 1, 0, kbytes);
        // encrypt block of text
        ss_encrypt(c, m, n);
        // print it into outfile
        gmp_fprintf(outfile, "%Zx\n", c);
        // clear array
        memset(kbytes, 0, j);
        kbytes[0] = 0xFF;
    }

    free(kbytes);
    mpz_clears(m, c, NULL);
}
//
// Decrypt number c into number m
//
// Provides:
//  m: decrypted/original integer
//
// Requires:
//  c: encrypted integer
//  d: private exponent
//  pq: private modulus
//  all mpz_t arguments to be initialized
//
void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
    return;
}

//
// Decrypt a file back into its original form.
//
// Provides:
//  fills outfile with the unencrypted data from infile
//
// Requires:
//  infile: open and readable file stream to encrypted data
//  outfile: open and writable file stream
//  d: private exponent
//  pq: private modulus
//
void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {
    mpz_t m, c, i;
    mpz_inits(m, c, i, NULL);
    size_t j;

    //calculate block size k
    uint64_t k = ((mpz_sizeinbase(pq, 2) - 1) / 8);

    uint8_t *kbytes = malloc(k * sizeof(uint8_t));

    while (gmp_fscanf(infile, "%Zx", c) == 1) {
        // decrypt scanned line
        ss_decrypt(m, c, d, pq);

        memset(kbytes, 0, k);
        // j = number of read bytes
        mpz_export(kbytes, &j, 1, sizeof(unsigned char), 1, 0, m);

        for (uint64_t i = 1; i < j; i++) {
            // print out read bytes from m
            fprintf(outfile, "%c", kbytes[i]);
        }
    }

    free(kbytes);
    mpz_clears(m, c, i, NULL);
}
