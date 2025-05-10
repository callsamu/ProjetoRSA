#include <stdio.h>
#include <gmp.h>
#include <string.h>

typedef struct {
    mpz_t n;
    mpz_t e;
} PublicKey;

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

int decrypt_message(const char *mensagem_encriptada, const mpz_t p, const mpz_t q, const mpz_t e) {
    if (!is_prime(p) || !is_prime(q)) {
        return 0;
    }

    mpz_t phi, d, n, c, m;
    mpz_inits(phi, d, n, c, m, NULL);
    mpz_mul(n, p, q);

    mpz_sub_ui(phi, p, 1);
    mpz_t q_menos_1;
    mpz_init(q_menos_1);
    mpz_sub_ui(q_menos_1, q, 1);
    mpz_mul(phi, phi, q_menos_1);
    mpz_clear(q_menos_1);

    if (!mpz_invert(d, e, phi)) {
        mpz_clears(phi, d, n, c, m, NULL);
        return 0;
    }

    FILE *input = fopen(mensagem_encriptada, "r");
    if (!input) {
        mpz_clears(phi, d, n, c, m, NULL);
        return 0;
    }

    FILE *output = fopen("mensagem_desencriptada.txt", "w");
    if (!output) {
        fclose(input);
        mpz_clears(phi, d, n, c, m, NULL);
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), input)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (mpz_set_str(c, line, 10) != 0) {
            return 0;
        }

        mpz_powm(m, c, d, n);

        unsigned long ascii = mpz_get_ui(m);
        if (ascii < 32 || ascii > 126) {
            return 0;
        }
        fputc((char)ascii, output);
    }

    if (ferror(input)) {
        return 0;
    }

    fclose(input);
    fclose(output);
    mpz_clears(phi, d, n, c, m, NULL);

    return 1;
}

int main() {
    mpz_t p, q, e;
    mpz_inits(p, q, e, NULL);
    mpz_set_ui(p, 100019);
    mpz_set_ui(q, 100043);
    mpz_set_ui(e, 65537);

    PublicKey chave;
    if (generate_public_key(p, q, e, &chave)) {

        const char *mensagem = "matematica discreta";
        if (encrypt_message(mensagem, &chave, "mensagem_encriptada.txt")) {
        }
    }

    if (!decrypt_message("mensagem_encriptada.txt", p, q, e)) {
        return 1;
    }

    mpz_clears(p, q, e, chave.n, chave.e, NULL);
    return 0;
}