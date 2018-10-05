#ifndef TRANSFERS_H
#define TRANSFERS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct TX_OUTPUT {
        char address[81];
        int64_t value;
        char message[2187];
        char tag[27];
} TX_OUTPUT;

typedef struct TX_INPUT {
        int64_t balance;
        uint32_t key_index;
} TX_INPUT;

typedef struct TX_DETAILS{
  char receiving_address[81];
  uint32_t input_address_index;
  uint32_t remainder_address_index;
  uint32_t timestamp;
  uint64_t transfer_amount;
  uint64_t balance;
  char tag[27];
} TX_DETAILS;

void prepare_transfers(char *seed, uint8_t security, TX_OUTPUT *outputs,
                       int num_outputs, TX_INPUT *inputs, int num_inputs,
                       uint32_t timestamp,
                       char bundle_hash[81],
                       char transaction_chars[][2673]);

bool iota_sign_transaction(char seed[81], TX_DETAILS *tx, char bundle_hash[],  char serialized_tx[]);

/*
 * Builds a 0 valued bundle (with input tx) to send a ASCII message (found on the output, tx index 0) from itself to itself.
 *
 * Make sure char[] are base27encoded.
 * Outputs:
 *   - bundle_hash
 *   - serialized_tx
 */
void build_signed_message(char seed[81], uint8_t index, char tag[27], uint32_t timestamp, char* message, uint16_t message_size, char bundle_hash[],  char serialized_tx[]);

#endif //TRANSFERS_H
