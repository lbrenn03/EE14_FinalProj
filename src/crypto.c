#include "ee14lib.h"
#include "crypto.h"

static const uint8_t key[32] = { /* 32 bytes */ };

// compute MAC over received challenge
int compute_mac(const uint8_t *challenge, size_t challenge_len,
                uint8_t *mac_out) {
    return mbedtls_md_hmac(
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        key, sizeof(key),
        challenge, challenge_len,
        mac_out
    );
}

// verify response
int verify_mac(const uint8_t *challenge, size_t challenge_len,
               const uint8_t *received_mac) {
    uint8_t expected_mac[32];
    compute_mac(challenge, challenge_len, expected_mac);
    
    return mbedtls_ct_memcmp(expected_mac, received_mac, 32);
}

void generate_challenge(uint8_t *buf, size_t len) {
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
    
    for (size_t i = 0; i < len; i += 4) {
        while (!(RNG->SR & RNG_SR_DRDY));
        uint32_t val = RNG->DR;
        memcpy(buf + i, &val, 4);
    }
}