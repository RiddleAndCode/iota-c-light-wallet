#include "addresses.h"
#include "common.h"
#include "conversion.h"
#include "kerl.h"
#include <string.h>

#define CHECKSUM_CHARS 9

static void digest_single_chunk(unsigned char *key_fragment,
                                cx_sha3_t *digest_sha3, cx_sha3_t *round_sha3)
{
    for (int k = 0; k < 26; k++) {
        kerl_initialize(round_sha3);
        kerl_absorb_chunk(round_sha3, key_fragment);
        kerl_squeeze_final_chunk(round_sha3, key_fragment);
    }

    // absorb buffer directly to avoid storing the digest fragment
    kerl_absorb_chunk(digest_sha3, key_fragment);
}

// initialize the sha3 instance for generating private key
static void init_shas(const unsigned char *seed_bytes, uint32_t idx,
                      cx_sha3_t *key_sha, cx_sha3_t *digest_sha)
{
    // use temp bigint so seed not destroyed
    unsigned char bytes[NUM_HASH_BYTES];
    os_memcpy(bytes, seed_bytes, sizeof(bytes));

    bytes_add_u32_mem(bytes, idx);

    kerl_initialize(key_sha);
    kerl_absorb_chunk(key_sha, bytes);
    kerl_squeeze_final_chunk(key_sha, bytes);

    kerl_initialize(key_sha);
    kerl_absorb_chunk(key_sha, bytes);

    kerl_initialize(digest_sha);
}

// generate public address in byte format
void get_public_addr(const unsigned char *seed_bytes, uint32_t idx,
                     unsigned int security, unsigned char *address_bytes)
{
    if (!IN_RANGE(security, MIN_SECURITY_LEVEL, MAX_SECURITY_LEVEL)) {
        THROW(INVALID_PARAMETER);
    }

    // sha size is 424 bytes
    cx_sha3_t key_sha, digest_sha;

    // init private key sha, digest sha
    init_shas(seed_bytes, idx, &key_sha, &digest_sha);

    // buffer for the digests of each security level
    unsigned char digest[NUM_HASH_BYTES * security];

    // only store a single fragment of the private key at a time
    // use last chunk of buffer, as this is only used after the key is generated
    unsigned char *key_f = digest + NUM_HASH_BYTES * (security - 1);

    for (uint8_t i = 0; i < security; i++) {
        for (uint8_t j = 0; j < 27; j++) {
            // use address output array as a temp Kerl state storage
            unsigned char *state = address_bytes;

            // the state takes only 48bytes and allows us to reuse key_sha
            kerl_state_squeeze_chunk(&key_sha, state, key_f);
            // re-use key_sha as round_sha
            digest_single_chunk(key_f, &digest_sha, &key_sha);

            // as key_sha has been tainted, reinitialize with the saved state
            kerl_reinitialize(&key_sha, state);
        }
        kerl_squeeze_final_chunk(&digest_sha, digest + NUM_HASH_BYTES * i);

        // reset digest sha for next digest
        kerl_initialize(&digest_sha);
    }

    // absorb the digest for each security
    kerl_absorb_bytes(&digest_sha, digest, NUM_HASH_BYTES * security);

    // one final squeeze for address
    kerl_squeeze_final_chunk(&digest_sha, address_bytes);
}

// get 9 character checksum of NUM_HASH_TRYTES character address
void get_address_with_checksum(const unsigned char *address_bytes,
                               char *full_address)
{
    cx_sha3_t sha;
    kerl_initialize(&sha);

    unsigned char checksum_bytes[NUM_HASH_BYTES];
    kerl_absorb_chunk(&sha, address_bytes);
    kerl_squeeze_final_chunk(&sha, checksum_bytes);

    char full_checksum[NUM_HASH_TRYTES];
    bytes_to_chars(checksum_bytes, full_checksum, NUM_HASH_BYTES);

    bytes_to_chars(address_bytes, full_address, NUM_HASH_BYTES);

    os_memcpy(full_address + NUM_HASH_TRYTES,
              full_checksum + NUM_HASH_TRYTES - CHECKSUM_CHARS, CHECKSUM_CHARS);
}

void iota_get_seed(uint8_t derived_seed[64], char seed[81]) {
    // generate seed from mnemonic
    SHA3_CTX ctx;

    kerl_initialize(&ctx);

    // Absorb 4 times using sliding window:
    // Divide 64 byte trezor-seed in 4 sections of 16 bytes.
    // 1: [123.] first 48 bytes
    // 2: [.123] last 48 bytes
    // 3: [3.12] last 32 bytes + first 16 bytes
    // 4: [23.1] last 16 bytes + first 32 bytes
    unsigned char bytes_in[48], seed_bytes[48];

    // Step 1.
    memcpy(&bytes_in[0], derived_seed, 48);
    kerl_absorb_bytes(&ctx, bytes_in, 48);

    // Step 2.
    memcpy(&bytes_in[0], derived_seed+16, 48);
    kerl_absorb_bytes(&ctx, bytes_in, 48);

    // Step 3.
    memcpy(&bytes_in[0], derived_seed+32, 32);
    memcpy(&bytes_in[32], derived_seed, 16);
    kerl_absorb_bytes(&ctx, bytes_in, 48);

    // Step 4.
    memcpy(&bytes_in[0], derived_seed+48, 16);
    memcpy(&bytes_in[16], derived_seed, 32);
    kerl_absorb_bytes(&ctx, bytes_in, 48);

    // Squeeze out the seed
    kerl_squeeze_final_chunk(&ctx, seed_bytes);
    bytes_to_chars(seed_bytes, seed, 48);
}

void get_address(const unsigned char *seed_bytes, uint32_t idx,
                        unsigned int security, char *address) {
  unsigned char bytes[48];
  get_public_addr(seed_bytes, idx, security, bytes);
  bytes_to_chars(bytes, address, 48);
}