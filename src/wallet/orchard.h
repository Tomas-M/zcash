// Copyright (c) 2021 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef ZCASH_WALLET_ORCHARD_H
#define ZCASH_WALLET_ORCHARD_H

#include <array>

#include "primitives/transaction.h"
#include "rust/orchard/keys.h"
#include "rust/orchard/wallet.h"
#include "zcash/address/orchard.hpp"

class OrchardNoteMetadata
{
private:
    OrchardOutPoint op;
    libzcash::OrchardRawAddress address;
    CAmount noteValue;
    std::array<uint8_t, ZC_MEMO_SIZE> memo;
    int confirmations;
public:
    OrchardNoteMetadata(
        OrchardOutPoint op,
        const libzcash::OrchardRawAddress& address,
        CAmount noteValue,
        const std::array<unsigned char, ZC_MEMO_SIZE>& memo,
        int confirmations):
        op(op), address(address), noteValue(noteValue), memo(memo), confirmations(confirmations) {}

    const OrchardOutPoint& GetOutPoint() const {
        return op;
    }

    void SetConfirmations(int c) {
        confirmations = c;
    }

    int GetConfirmations() const {
        return confirmations;
    }

    CAmount GetNoteValue() const {
        return noteValue;
    }
};

class OrchardWallet
{
private:
    std::unique_ptr<OrchardWalletPtr, decltype(&orchard_wallet_free)> inner;

public:
    OrchardWallet() : inner(orchard_wallet_new(), orchard_wallet_free) {}
    OrchardWallet(OrchardWallet&& wallet_data) : inner(std::move(wallet_data.inner)) {}
    OrchardWallet& operator=(OrchardWallet&& wallet)
    {
        if (this != &wallet) {
            inner = std::move(wallet.inner);
        }
        return *this;
    }

    // OrchardWallet should never be copied
    OrchardWallet(const OrchardWallet&) = delete;
    OrchardWallet& operator=(const OrchardWallet&) = delete;

    const OrchardWalletPtr* GetPointer() const {
        return inner.get();
    }

    void CheckpointNoteCommitmentTree() {
        orchard_wallet_checkpoint(inner.get());
    }

    bool RewindToLastCheckpoint() {
        return orchard_wallet_rewind(inner.get());
    }

    /**
     * Add notes that are decryptable with IVKs for which the wallet
     * contains the full viewing key to the wallet.
     */
    void AddNotesIfInvolvingMe(const CTransaction& tx) {
        orchard_wallet_add_notes_from_bundle(
                inner.get(),
                tx.GetHash().begin(),
                tx.GetOrchardBundle().inner.get());
    }

    /**
     * Append each Orchard note commitment from the specified block to the
     * wallet's note commitment tree.
     *
     * Returns `false` if the caller attempts to insert a block out-of-order.
     */
    bool AppendNoteCommitments(const int nBlockHeight, const CBlock& block) {
        for (int txidx = 0; txidx < block.vtx.size(); txidx++) {
            const CTransaction& tx = block.vtx[txidx];
            if (!orchard_wallet_append_bundle_commitments(
                    inner.get(),
                    (uint32_t) nBlockHeight,
                    txidx,
                    tx.GetHash().begin(),
                    tx.GetOrchardBundle().inner.get()
                    )) {
                return false;
            }
        }

        return true;
    }

    bool TxContainsMyNotes(const uint256& txid) {
        return orchard_wallet_tx_contains_my_notes(
                inner.get(),
                txid.begin());
    }

    void AddSpendingKey(const libzcash::OrchardSpendingKey& sk) {
        orchard_wallet_add_spending_key(inner.get(), sk.inner.get());
    }

    void AddFullViewingKey(const libzcash::OrchardFullViewingKey& fvk) {
        orchard_wallet_add_full_viewing_key(inner.get(), fvk.inner.get());
    }

    std::optional<libzcash::OrchardIncomingViewingKey> GetIncomingViewingKeyForAddress(
            const libzcash::OrchardRawAddress& addr) const {
        auto ivkPtr = orchard_wallet_get_ivk_for_address(inner.get(), addr.inner.get());
        if (ivkPtr == nullptr) return std::nullopt;
        return libzcash::OrchardIncomingViewingKey(ivkPtr);
    }

    /**
     * Adds an address/IVK pair to the wallet, and returns `true` if the
     * IVK corresponds to a full viewing key known to the wallet, `false`
     * otherwise.
     */
    bool AddRawAddress(
            const libzcash::OrchardRawAddress& addr,
            const libzcash::OrchardIncomingViewingKey& ivk) {
        return orchard_wallet_add_raw_address(inner.get(), addr.inner.get(), ivk.inner.get());
    }

    static void PushOrchardNoteMeta(void* orchardNotesRet, RawOrchardNoteMetadata rawNoteMeta) {
        uint256 txid;
        std::move(std::begin(rawNoteMeta.txid), std::end(rawNoteMeta.txid), txid.begin());
        OrchardOutPoint op(txid, rawNoteMeta.actionIdx);
        std::array<uint8_t, ZC_MEMO_SIZE> memo;
        std::move(std::begin(rawNoteMeta.memo), std::end(rawNoteMeta.memo), memo.begin());
        OrchardNoteMetadata noteMeta(
                op,
                libzcash::OrchardRawAddress(rawNoteMeta.addr),
                rawNoteMeta.noteValue,
                memo,
                rawNoteMeta.confirmations);

        reinterpret_cast<std::vector<OrchardNoteMetadata>*>(orchardNotesRet)->push_back(noteMeta);
    }
};

#endif // ZCASH_ORCHARD_WALLET_H
