#include "numtheory.h"
#include "randstate.h"

void gcd(mpz_t d, const mpz_t a, const mpz_t b) {
    mpz_t b2, temp;
    mpz_inits(b2, temp, NULL);
    mpz_set(d, a);
    mpz_set(b2, b);

    while (mpz_cmp_ui(b2, 0) != 0) {
        // a,b = b, a mod b
        mpz_set(temp, b2);
        mpz_mod(b2, d, b2);
        mpz_set(d, temp);
    }
    mpz_clears(b2, temp, NULL);
    return;
}

void mod_inverse(mpz_t i, const mpz_t a, const mpz_t n) {
    mpz_t r, r1, t, t1, q, temp;
    mpz_inits(r, r1, t, t1, q, temp, NULL);
    mpz_set(r, n);
    mpz_set(r1, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t1, 1);

    while (mpz_cmp_ui(r1, 0) != 0) {
        mpz_fdiv_q(q, r, r1); //q = r//r1

        //r1 = temp(r) - q * r1;
        mpz_set(temp, r);
        mpz_set(r, r1);
        mpz_mul(r1, q, r1);
        mpz_sub(r1, temp, r1);

        //t1 = temp(r) - q * t1;
        mpz_set(temp, t);
        mpz_set(t, t1);
        mpz_mul(t1, q, t1);
        mpz_sub(t1, temp, t1);
    }

    mpz_set(i, t);

    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(i, 0);
        mpz_clears(r, r1, t, t1, q, temp, NULL);
        return;
    }

    if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(i, t, n);
    }

    mpz_clears(r, r1, t, t1, q, temp, NULL);
    return;
}

void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    mpz_t base, exp;
    mpz_inits(base, exp, NULL);
    mpz_set(base, a);
    mpz_set(exp, d);
    mpz_set_ui(o, 1);

    while (mpz_cmp_ui(exp, 0) > 0) {
        if (mpz_odd_p(exp)) {
            mpz_mul(o, o, base);
            mpz_mod(o, o, n);
            // o = (o * base) mod n
        }
        // base = (base * base) mod n
        mpz_mul(base, base, base);
        mpz_mod(base, base, n);
        // exp = d // 2
        mpz_fdiv_q_ui(exp, exp, 2);
    }
    mpz_clears(base, exp, NULL);
    return;
}

bool is_prime(const mpz_t n, uint64_t iters) {
    mpz_t y, s, s1, a, j, two, r, temp;
    mpz_inits(y, s, s1, a, j, two, r, temp, NULL);
    mpz_set_ui(two, 2);

    // if n is less than 2 or if its even but not 2
    // then it isn't a prime
    if (mpz_cmp_ui(n, 2) < 0 || (mpz_cmp_ui(n, 2) != 0 && mpz_even_p(n))) {
        mpz_clears(y, s, s1, a, j, two, r, temp, NULL);
        return false;
    }

    // if n is 2 or 3 it is prime
    if (mpz_cmp_ui(n, 2) == 0 || mpz_cmp_ui(n, 3) == 0) {
        mpz_clears(y, s, s1, a, j, two, r, temp, NULL);
        return true;
    }

    // n - 1 = (2^s)r such that r is odd
    mpz_sub_ui(r, n, 1);
    int si = 0;

    while (mpz_even_p(r) != 0) {
        mpz_fdiv_q_ui(r, r, 2);
        si++;
    }
    mpz_set_ui(s, si);

    for (uint64_t i = 1; i <= iters; i++) {
        // find random number 'a'
        mpz_sub_ui(temp, n, 3);
        mpz_urandomm(a, state, temp);
        mpz_add_ui(a, a, 2);

        pow_mod(y, a, r, n);

        mpz_sub_ui(temp, n, 1);
        // if y != 1 and y != n -1
        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, temp) != 0) {
            mpz_set_ui(j, 1);
            mpz_sub_ui(s1, s, 1);
            //while j is <= s-1 and y != n-1
            while (mpz_cmp(j, s1) <= 0 && mpz_cmp(y, temp) != 0) {
                pow_mod(y, y, two, n);

                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(y, s, s1, a, j, two, r, temp, NULL);
                    return false;
                }
                mpz_add_ui(j, j, 1);
            }
            if (mpz_cmp(y, temp) != 0) {
                mpz_clears(y, s, s1, a, j, two, r, temp, NULL);
                return false;
            }
        }
    }
    mpz_clears(y, s, s1, a, j, two, r, temp, NULL);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t low, up, mod, one;
    mpz_inits(low, up, mod, one, NULL);

    // variable to use 1 as an mpz
    mpz_set_ui(one, 1);

    // set up the bounds for random
    uint64_t bits1 = bits - 1;
    mpz_mul_2exp(low, one, bits1);
    mpz_mul_2exp(up, one, bits);
    mpz_sub_ui(up, up, 1);
    mpz_sub(up, up, low);

    while (true) {
        // random starts from 0 so add the lower bound afterwards
        mpz_urandomm(p, state, up);
        mpz_add(p, p, low);

        // if p is even add 1
        mpz_mod_ui(mod, p, 2);
        if (mpz_cmp_ui(mod, 0) == 0) {
            mpz_add_ui(p, p, 1);
        }

        // if p is prime return
        if (is_prime(p, iters)) {
            mpz_clears(low, up, mod, one, NULL);
            return;
        }
    }
}
