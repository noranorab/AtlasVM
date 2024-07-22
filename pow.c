#include "pow.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>



int compute_pow(uint8_t* block, size_t block_size, uint32_t difficulty) {
    uint32_t nonce = 0;
    uint8_t hash[SHA256_DIGEST_LENGTH];
    uint8_t target[SHA256_DIGEST_LENGTH];
    memset(target, 0, SHA256_DIGEST_LENGTH);

    // Set the difficulty target
    for (uint32_t i = 0; i < difficulty; i++) {
        target[i / 8] |= (1 << (7 - (i % 8)));
    }

    while (1) {
        memcpy(block + block_size - sizeof(nonce), &nonce, sizeof(nonce));
        SHA256(block, block_size, hash);

        if (memcmp(hash, target, SHA256_DIGEST_LENGTH) < 0) {
            printf("Nonce found: %u\n", nonce);
            return nonce;
        }

        nonce++;
    }
}

