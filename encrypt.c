#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "ss.h"
#include "randstate.h"
#include "numtheory.h"

#define OPTIONS "i:o:n:vh"

void synopsis(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Encrypts data using SS encryption.\n"
        "   Encrypted data is decrypted by the decrypt program.\n"
        "\n"
        "USAGE\n"
        "   %s [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to encrypt (default: stdin).\n"
        "   -o outfile      Output file for encrypted data (default: stdout).\n"
        "   -n pbfile       Public key file (default: ss.pub).\n",
        exec);
}

int main(int argc, char **argv) {
    // default values
    FILE *input = NULL;
    FILE *output = NULL;
    FILE *pbfile = NULL;
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
            pbfile = fopen(optarg, "r");
            if (pbfile == NULL) {
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
    if (pbfile == NULL) {
        pbfile = fopen("ss.pub", "r");
        if (pbfile == NULL) {
            printf("Failed to open ss.pub.\n");
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
    mpz_t n, bits;
    mpz_inits(n, bits, NULL);

    char *username = malloc((LOGIN_NAME_MAX + 1) * sizeof(char));
    ss_read_pub(n, username, pbfile);

    if (verbose) {
        printf("user = %s\n", username);
        mpz_set_ui(bits, mpz_sizeinbase(n, 2));
        gmp_printf("n (%Zd bits) = %Zd\n", bits, n);
    }

    // encrypt input file
    ss_encrypt_file(input, output, n);

    //close files and clear variables
    free(username);
    mpz_clears(n, bits, NULL);
    fclose(input);
    fclose(output);
    fclose(pbfile);
}
