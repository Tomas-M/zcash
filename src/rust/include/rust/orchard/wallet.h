// Copyright (c) 2021 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef ZCASH_RUST_INCLUDE_RUST_ORCHARD_WALLET_H
#define ZCASH_RUST_INCLUDE_RUST_ORCHARD_WALLET_H

#include "rust/orchard/keys.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A type-safe pointer type for an Orchard wallet.
 */
struct OrchardWalletPtr;
typedef struct OrchardWalletPtr OrchardWalletPtr;

/**
 * Constructs a new empty Orchard wallet and return a pointer to it.
 * Memory is allocated by Rust and must be manually freed using
 * `orchard_wallet_free`.
 */
OrchardWalletPtr* orchard_wallet_new();

/**
 * Frees the memory associated with an Orchard wallet that was allocated
 * by Rust.
 */
void orchard_wallet_free(OrchardWalletPtr* wallet);

/**
 * Adds a checkpoint to the wallet's note commitment tree to enable
 * a future rewind.
 */
void orchard_wallet_checkpoint(
        OrchardWalletPtr* wallet
        );

/**
 * Rewinds to the most recently added checkpoint.
 */
bool orchard_wallet_rewind(
        OrchardWalletPtr* wallet
        );

/**
 * Searches the provided bundle for notes that are visible to the specified
 * wallet's incoming viewing keys, and adds those notes to the wallet.
 */
void orchard_wallet_add_notes_from_bundle(
        OrchardWalletPtr* wallet,
        const unsigned char txid[32],
        const OrchardBundlePtr* bundle
        );

/**
 * Add the note commitment values for the specified bundle to the wallet's note
 * commitment tree, and mark any Orchard notes that belong to the wallet so
 * that we can construct authentication paths to these notes in the future.
 *
 * This requires the block height and the index of the block within the
 * transaction in order to guarantee that note commitments are appended in the
 * correct order. Returns `false` if the provided bundle is not in the correct
 * position to have its note commitments appended to the note commitment tree.
 */
bool orchard_wallet_append_bundle_commitments(
        OrchardWalletPtr* wallet,
        const uint32_t block_height,
        const size_t block_tx_idx,
        const unsigned char txid[32],
        const OrchardBundlePtr* bundle
        );

/**
 * Returns whether the specified transaction contains any Orchard notes that belong
 * to this wallet.
 */
bool orchard_wallet_tx_contains_my_notes(
        const OrchardWalletPtr* wallet,
        const unsigned char txid[32]);

/**
 * Add the specified spending key to the wallet's key store.
 * This will also compute and add the associated full and
 * incoming viewing keys.
 */
void orchard_wallet_add_spending_key(
        OrchardWalletPtr* wallet,
        const OrchardSpendingKeyPtr* sk);

/**
 * Add the specified full viewing key to the wallet's key store.
 * This will also compute and add the associated incoming viewing key.
 */
void orchard_wallet_add_full_viewing_key(
        OrchardWalletPtr* wallet,
        const OrchardFullViewingKeyPtr* fvk);

/**
 * Add the specified raw address to the wallet's key store,
 * associated with the incoming viewing key from which that
 * address was derived.
 */
bool orchard_wallet_add_raw_address(
        OrchardWalletPtr* wallet,
        const OrchardRawAddressPtr* addr,
        const OrchardIncomingViewingKeyPtr* ivk);

/**
 * Returns a pointer to the Orchard incoming viewing key
 * corresponding to the specified raw address, if it is
 * known to the wallet, or `nullptr` otherwise.
 *
 * Memory is allocated by Rust and must be manually freed using
 * `orchard_incoming_viewing_key_free`.
 */
OrchardIncomingViewingKeyPtr* orchard_wallet_get_ivk_for_address(
        const OrchardWalletPtr* wallet,
        const OrchardRawAddressPtr* addr);

/**
 * A C struct used to transfer note metadata information across the Rust FFI
 * boundary. This must have the same in-memory representation as the
 * `NoteMetadata` type in orchard_ffi/wallet.rs.
 */
struct RawOrchardNoteMetadata {
    unsigned char txid[32];
    uint32_t actionIdx;
    OrchardRawAddressPtr* addr;
    CAmount noteValue;
    unsigned char memo[512];
    int confirmations;
};

typedef int (*confs_callback_t)(void* pwallet, const unsigned char* ptxid);
typedef void (*push_callback_t)(void* resultVector, const RawOrchardNoteMetadata noteMeta);

/**
 * Finds notes that belong to the wallet that were sent to addresses derived
 * from the specified incoming viewing key, subject to the specified flags, and
 * uses the provided callback to push RawOrchardNoteMetadata values
 * corresponding to those notes on to the provided result vector.  Note that
 * the push_cb callback can perform any necessary conversion from a
 * RawOrchardNoteMetadata value prior in addition to modifying the provided
 * result vector.
 *
 * If `ivk` is null, all notes belonging to the wallet will be returned.
 */
void orchard_wallet_get_filtered_notes(
        const OrchardWalletPtr* wallet,
        const OrchardIncomingViewingKeyPtr* ivk,
        void* pCppWallet,
        confs_callback_t confs_cb,
        bool ignoreSpent,
        bool ignoreLocked,
        bool requireSpendingKey,
        void* resultVector,
        push_callback_t push_cb
        );

#ifdef __cplusplus
}
#endif

#endif // ZCASH_RUST_INCLUDE_RUST_ORCHARD_WALLET_H


