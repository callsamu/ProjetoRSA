#include <gmp.h>

typedef struct {
    mpz_t n;
    mpz_t e;
} PublicKey;


int is_prime(const mpz_t number);
int generate_public_key(const mpz_t p, const mpz_t q, const mpz_t e, PublicKey *chave);
int save_public_key(const PublicKey *chave, const char *file_name);
int encrypt_message(const char *mensagem, const PublicKey *chave, const char *file_name);
int decrypt_message(const char *mensagem_encriptada, const mpz_t p, const mpz_t q, const mpz_t e);
