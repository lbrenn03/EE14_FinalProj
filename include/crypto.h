#include "mbedtls/md.h"

int compute_mac(const uint8_t *challenge, size_t challenge_len,
                uint8_t *mac_out);

int verify_mac(const uint8_t *challenge, size_t challenge_len,
               const uint8_t *received_mac);

void generate_challenge(uint8_t *buf, size_t len);