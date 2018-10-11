#ifndef ADDRESSES_H
#define ADDRESSES_H

#include "iota_types.h"

#ifdef __cplusplus
extern "C" {
#endif


void get_public_addr(const unsigned char *seed_bytes, uint32_t idx,
                     unsigned int security, unsigned char *address_bytes);

void get_address(const unsigned char *seed_bytes, uint32_t idx,
                        unsigned int security, char *address);

/** @brief Computes the full address string in base-27 encoding.
 *  The full address consists of the actual address (81 chars) plus 9 chars of
 *  checksum.
 */
void get_address_with_checksum(const unsigned char *address_bytes,
                               char *full_address);

/*
 * From mnemonic phrase seed to IOTA seed bytes.
*/
void iota_get_seed(uint8_t derived_seed[64], char seed_bytes[81]);

#ifdef __cplusplus
}
#endif


#endif // ADDRESSES_H
