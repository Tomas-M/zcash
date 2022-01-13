// Copyright (c) 2021 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef ZCASH_ZCASH_ADDRESS_UNIFIED_H
#define ZCASH_ZCASH_ADDRESS_UNIFIED_H

#include "bip44.h"
#include "key_constants.h"
#include "zip32.h"

namespace libzcash {

enum class ReceiverType: uint32_t {
    P2PKH = 0x00,
    P2SH = 0x01,
    Sapling = 0x02,
    //Orchard = 0x03
};

/**
 * Test whether the specified list of receiver types contains a
 * shielded receiver type
 */
bool HasShielded(const std::set<ReceiverType>& receiverTypes);

/**
 * Test whether the specified list of receiver types contains a
 * shielded receiver type
 */
bool HasTransparent(const std::set<ReceiverType>& receiverTypes);

class ZcashdUnifiedSpendingKey;

// prototypes for the classes handling ZIP-316 encoding (in Address.hpp)
class UnifiedAddress;
class UnifiedFullViewingKey;

/**
 * An internal identifier for a unified full viewing key, derived as a
 * blake2b hash of the serialized form of the UFVK.
 */
class UFVKId: public uint256 {
public:
    UFVKId() : uint256() {}
    UFVKId(const uint256& in) : uint256(in) {}
};

/**
 * An internal-only type for unified full viewing keys that represents only the
 * set of receiver types that are supported by zcashd. This type does not
 * support round-trip serialization to and from the UnifiedFullViewingKey type,
 * which should be used in most cases.
 */
class ZcashdUnifiedFullViewingKey {
private:
    UFVKId keyId;
    std::optional<CChainablePubKey> transparentKey;
    std::optional<SaplingDiversifiableFullViewingKey> saplingKey;

    ZcashdUnifiedFullViewingKey() {}

    friend class ZcashdUnifiedSpendingKey;
public:
    /**
     * This constructor is lossy; it ignores unknown receiver types
     * and therefore does not support round-trip transformations.
     */
    static ZcashdUnifiedFullViewingKey FromUnifiedFullViewingKey(
            const KeyConstants& keyConstants,
            const UnifiedFullViewingKey& ufvk);

    const UFVKId& GetKeyID() const {
        return keyId;
    }

    const std::optional<CChainablePubKey>& GetTransparentKey() const {
        return transparentKey;
    }

    const std::optional<SaplingDiversifiableFullViewingKey>& GetSaplingKey() const {
        return saplingKey;
    }

    /**
     * Creates a new unified address having the specified receiver types, at the specified
     * diversifier index, unless the diversifer index would generate an invalid receiver.
     * Returns `std::nullopt` if the diversifier index does not produce a valid receiver
     * for one or more of the specified receiver types; under this circumstance, the caller
     * should usually try successive diversifier indices until the operation returns a
     * non-null value.
     *
     * This method will throw if `receiverTypes` does not include a shielded receiver type.
     */
    std::optional<UnifiedAddress> Address(
            const diversifier_index_t& j,
            const std::set<ReceiverType>& receiverTypes) const;

    /**
     * Find the smallest diversifier index >= `j` such that it generates a valid
     * unified address according to the conditions specified in the documentation
     * for the `Address` method above, and returns the newly created address along
     * with the diversifier index used to produce it. Returns `std::nullopt` if the
     * diversifier space is exhausted, or if the set of receiver types contains a
     * transparent receiver and the diversifier exceeds the maximum transparent
     * child index.
     *
     * This method will throw if `receiverTypes` does not include a shielded receiver type.
     */
    std::optional<std::pair<UnifiedAddress, diversifier_index_t>> FindAddress(
            const diversifier_index_t& j,
            const std::set<ReceiverType>& receiverTypes) const;

    /**
     * Find the next available address that contains all supported receiver types.
     */
    std::optional<std::pair<UnifiedAddress, diversifier_index_t>> FindAddress(const diversifier_index_t& j) const;

    friend bool operator==(const ZcashdUnifiedFullViewingKey& a, const ZcashdUnifiedFullViewingKey& b)
    {
        return a.transparentKey == b.transparentKey && a.saplingKey == b.saplingKey;
    }
};

/**
 * The type of unified spending keys supported by zcashd.
 */
class ZcashdUnifiedSpendingKey {
private:
    CExtKey transparentKey;
    SaplingExtendedSpendingKey saplingKey;

    ZcashdUnifiedSpendingKey() {}
public:
    static std::optional<ZcashdUnifiedSpendingKey> ForAccount(
            const HDSeed& seed,
            uint32_t bip44CoinType,
            libzcash::AccountId accountId);

    const CExtKey& GetTransparentKey() const {
        return transparentKey;
    }

    const SaplingExtendedSpendingKey& GetSaplingExtendedSpendingKey() const {
        return saplingKey;
    }

    UnifiedFullViewingKey ToFullViewingKey() const;
};

} //namespace libzcash

#endif // ZCASH_ZCASH_ADDRESS_UNIFIED_H
