#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include "ss.h"
#include "randstate.h"
#include "numtheory.h"

#define OPTIONS "i:o:n:vh"

void synopsis(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Encrypts data using SS encryption.\n"
        "   Encrypted data is encrypted by the encrypt program.\n"
        "\n"
        "USAGE\n"
        "   %s [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to encrypt (default: stdin).\n"
        "   -o outfile      Output file for encrypted data (default: stdout).\n"
        "   -n pbfile       Public key file (default: ss.priv).\n",
        exec);
}

int main(int argc, char **argv) {
    // default values
    FILE *input = NULL;
    FILE *output = NULL;
    FILE *pvfile = NULL;
    bool verbose = false;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            input = fopen(optarg, "r");
            if (input == NULL) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 'o':
            output = fopen(optarg, "w");
            if (output == NULL) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 'n':
            pvfile = fopen(optarg, "r");
            if (pvfile == NULL) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 'v': verbose = true; break;
        case 'h': synopsis(argv[0]); return 0;
        default: synopsis(argv[0]); return 1;
        }
    }

    // open default files if not specified
    if (pvfile == NULL) {
        pvfile = fopen("ss.priv", "r");
        if (pvfile == NULL) {
            printf("Failed to open ss.priv.\n");
            return 1;
        }
    }
    if (input == NULL) {
        input = stdin;
    }
    if (output == NULL) {
        output = stdout;
    }

    // initialize mpz_t variables
    mpz_t pq, d, bits;
    mpz_inits(pq, d, bits, NULL);

    ss_read_priv(pq, d, pvfile);

    if (verbose) {
        mpz_set_ui(bits, mpz_sizeinbase(pq, 2));
        gmp_printf("pq (%Zd bits) = %Zd\n", bits, pq);
        mpz_set_ui(bits, mpz_sizeinbase(d, 2));
        gmp_printf("d (%Zd bits) = %Zd\n", bits, d);
    }

    // encrypt input file
    ss_decrypt_file(input, output, d, pq);

    //close files and clear variables
    mpz_clears(pq, d, bits, NULL);
    fclose(input);
    fclose(output);
    fclose(pvfile);
}
