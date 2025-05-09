#include <stdio.h>
#include <gmp.h>
#include <string.h>

typedef struct {
    mpz_t n;
    mpz_t e;
} PublicKey;

int is_prime(const mpz_t number) {
    return mpz_probab_prime_p(number, 50);
}

int save_public_key(const PublicKey *chave, const char *file_name) {
    FILE *arquivo = fopen(file_name, "w");
    if (!arquivo) return 0;

    gmp_fprintf(arquivo, "%Zd\n%Zd\n", chave->n, chave->e);
    fclose(arquivo);
    return 1;
}

int generate_public_key(const mpz_t p, const mpz_t q, const mpz_t e, PublicKey *chave) {
    if (!is_prime(p) || !is_prime(q)) {
        return 0;
    }

    mpz_t phi, temp;
    mpz_inits(phi, temp, NULL);

    mpz_sub_ui(phi, p, 1);
    mpz_sub_ui(temp, q, 1);
    mpz_mul(phi, phi, temp);

    mpz_gcd(temp, e, phi);
    if (mpz_cmp_ui(temp, 1) != 0) {
        mpz_clears(phi, temp, NULL);
        return 0;
    }

    mpz_init(chave->n);
    mpz_mul(chave->n, p, q);
    mpz_init(chave->e);
    mpz_set(chave->e, e);

    if (!save_public_key(chave, "chave_publica.txt")) {
        mpz_clears(phi, temp, NULL);
        return 0;
    }

    mpz_clears(phi, temp, NULL);
    return 1;
}

int encrypt_message(const char *mensagem, const PublicKey *chave, const char *file_name) {
    FILE *arquivo = fopen(file_name, "w");
    if (!arquivo) {
        return 0;
    }

    mpz_t m, c;
    mpz_inits(m, c, NULL);

    for (size_t i = 0; i < strlen(mensagem); ++i) {
        int ascii = (int)mensagem[i];
        if (ascii < 32 || ascii > 126) {
            fclose(arquivo);
            mpz_clears(m, c, NULL);
            return 0;
        }

        mpz_set_ui(m, ascii);
        mpz_powm(c, m, chave->e, chave->n);
        gmp_fprintf(arquivo, "%Zd\n", c);
    }

    fclose(arquivo);
    mpz_clears(m, c, NULL);
    return 1;
}

int main() {
    mpz_t p, q, e;
    mpz_inits(p, q, e, NULL);

    mpz_set_ui(p, 61);
    mpz_set_ui(q, 67);
    mpz_set_ui(e, 17);

    PublicKey chave;
    if (generate_public_key(p, q, e, &chave)) {

        const char *mensagem = "crypto";
        if (encrypt_message(mensagem, &chave, "mensagem_encriptada.txt")) {
        }
    }

    mpz_clears(p, q, e, chave.n, chave.e, NULL);
    return 0;
}