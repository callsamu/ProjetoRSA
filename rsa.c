#include <stdio.h>
#include <gmp.h>
#include <string.h>

#include "rsa.h"

int is_prime(const mpz_t number) {
    return mpz_probab_prime_p(number, 100);
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
		return RSA_E_NOT_COPRIME;
    }

    mpz_init(chave->n);
    mpz_mul(chave->n, p, q);
    mpz_init(chave->e);
    mpz_set(chave->e, e);

    if (!save_public_key(chave, "./texts/public_key.txt")) {
        mpz_clears(phi, temp, NULL);
        return RSA_FAILED_TO_SAVE_PUBLIC_KEY;
    }

    mpz_clears(phi, temp, NULL);
    return RSA_OK;
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
        gmp_fprintf(arquivo, "%Zd ", c);
    }

    fclose(arquivo);
    mpz_clears(m, c, NULL);
    return 1;
}

int decrypt_message(const char *mensagem_encriptada, const mpz_t p, const mpz_t q, const mpz_t e) {
    if (!is_prime(p) || !is_prime(q)) {
        return 0;
    }

    mpz_t phi, d, n, c, m;
    mpz_inits(phi, d, n, c, m, NULL);
    mpz_mul(n, p, q);

    mpz_sub_ui(phi, p, 1);
    mpz_t q_minus_1;
    mpz_init(q_minus_1);
    mpz_sub_ui(q_minus_1, q, 1);
    mpz_mul(phi, phi, q_minus_1);
    mpz_clear(q_minus_1);

    if (!mpz_invert(d, e, phi)) {
        mpz_clears(phi, d, n, c, m, NULL);
        return 0;
    }

    FILE *output = fopen("texts/decrypted.txt", "w");
    if (!output) {
        mpz_clears(phi, d, n, c, m, NULL);
        return 0;
    }

	FILE *stream = fmemopen((void *) mensagem_encriptada, strlen(mensagem_encriptada), "r");

    char token[1024];
    while (fscanf(stream, "%1023s", token) == 1) {
        if (mpz_set_str(c, token, 10) != 0) {
            fclose(output);
			fclose(stream);
            mpz_clears(phi, d, n, c, m, NULL);
            return 0;
        }

        mpz_powm(m, c, d, n);

        unsigned long ascii = mpz_get_ui(m);
        if (ascii < 32 || ascii > 126) {
            fclose(output);
			fclose(stream);
            mpz_clears(phi, d, n, c, m, NULL);
            return 0;
        }
        fputc((char)ascii, output);
    }

    fclose(output);
	fclose(stream);
    mpz_clears(phi, d, n, c, m, NULL);

	return RSA_OK;
}
