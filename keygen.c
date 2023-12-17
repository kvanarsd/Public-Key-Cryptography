#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "ss.h"
#include "randstate.h"
#include "numtheory.h"

#define OPTIONS "b:i:n:d:s:vh"

void synopsis(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Generates an SS public/private key pair.\n"
        "\n"
        "USAGE\n"
        "   %s [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -b bits         Minimum bits needed for public key n (default: 256).\n"
        "   -i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
        "   -n pbfile       Public key file (default: ss.pub).\n"
        "   -d pvfile       Private key file (default: ss.priv).\n"
        "   -s seed         Random seed for testing.\n",
        exec);
}

int main(int argc, char **argv) {
    // default values
    uint64_t iters = 50;
    uint64_t nbits = 256;
    FILE *pbfile = NULL;
    FILE *pvfile = NULL;
    int seed = time(NULL);
    bool verbose = false;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': nbits = atoi(optarg); break;
        case 'i': iters = atoi(optarg); break;
        case 'n':
            pbfile = fopen(optarg, "w");
            if (pbfile == NULL) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 'd':
            pvfile = fopen(optarg, "w");
            if (pvfile == NULL) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 's': seed = atoi(optarg); break;
        case 'v': verbose = true; break;
        case 'h': synopsis(argv[0]); return 0;
        default: synopsis(argv[0]); return 1;
        }
    }

    // open default files if not specified by user
    if (pbfile == NULL) {
        pbfile = fopen("ss.pub", "w");
        if (pbfile == NULL) {
            printf("Failed to open ss.pub.\n");
            return 1;
        }
    }
    if (pvfile == NULL) {
        pvfile = fopen("ss.priv", "w");
        if (pvfile == NULL) {
            printf("Failed to open ss.priv.\n");
            return 1;
        }
    }

    // set file permisions
    int pbd = fileno(pbfile);
    int pvd = fileno(pvfile);
    if (fchmod(pbd, S_IRUSR | S_IWUSR) != 0) {
        printf("Failed to set public key file write and read permisions to user.\n");
        return 1;
    }
    if (fchmod(pvd, S_IRUSR | S_IWUSR) != 0) {
        printf("Failed to set private key file write and read permisions to user.\n");
        return 1;
    }

    // initialize random state
    randstate_init(seed);

    // define mpz_t variables
    mpz_t p, q, n, d, pq, bits;
    mpz_inits(p, q, n, d, pq, bits, NULL);
    // make keys
    ss_make_pub(p, q, n, nbits, iters);
    ss_make_priv(d, pq, p, q);
    // Get the current users name
    // Write keys into respective files
    ss_write_pub(n, getenv("USER"), pbfile);
    ss_write_priv(pq, d, pvfile);

    // If verbose is enabled print information
    if (verbose) {
        printf("user = %s\n", getenv("USER"));
        mpz_set_ui(bits, mpz_sizeinbase(p, 2));
        gmp_printf("p (%Zd bits) = %Zd\n", bits, p);
        mpz_set_ui(bits, mpz_sizeinbase(q, 2));
        gmp_printf("q (%Zd bits) = %Zd\n", bits, q);
        mpz_set_ui(bits, mpz_sizeinbase(n, 2));
        gmp_printf("n (%Zd bits) = %Zd\n", bits, n);
        mpz_set_ui(bits, mpz_sizeinbase(d, 2));
        gmp_printf("d (%Zd bits) = %Zd\n", bits, d);
        mpz_set_ui(bits, mpz_sizeinbase(pq, 2));
        gmp_printf("pq (%Zd bits) = %Zd\n", bits, pq);
    }

    // Close files
    fclose(pbfile);
    fclose(pvfile);
    // Clear random state
    randstate_clear();
    // Clear all mpz_t variables
    mpz_clears(p, q, n, d, pq, bits, NULL);
    return 0;
}
