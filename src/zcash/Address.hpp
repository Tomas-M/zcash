#ifndef ZC_ADDRESS_H_
#define ZC_ADDRESS_H_

#include "key_constants.h"
#include "pubkey.h"
#include "key_constants.h"
#include "script/script.h"
#include "uint256.h"
#include "zcash/address/orchard.hpp"
#include "zcash/address/sapling.hpp"
#include "zcash/address/sprout.hpp"
#include "zcash/address/unified.h"
#include "zcash/address/zip32.h"

#include <variant>
#include <rust/unified_keys.h>

const unsigned char ZCASH_UFVK_ID_PERSONAL[BLAKE2bPersonalBytes] =
    {'Z', 'c', 'a', 's', 'h', '_', 'U', 'F', 'V', 'K', '_', 'I', 'd', '_', 'F', 'P'};

namespace libzcash {

/** Protocol addresses that can receive funds in a transaction. */
typedef std::variant<SproutPaymentAddress, SaplingPaymentAddress> RawAddress;

class InvalidEncoding {
public:
    friend bool operator==(const InvalidEncoding &a, const InvalidEncoding &b) { return true; }
    friend bool operator<(const InvalidEncoding &a, const InvalidEncoding &b) { return true; }
};

class UnknownReceiver {
public:
    uint32_t typecode;
    std::vector<uint8_t> data;

    UnknownReceiver(uint32_t typecode, std::vector<uint8_t> data) :
        typecode(typecode), data(data) {}

    friend inline bool operator==(const UnknownReceiver& a, const UnknownReceiver& b) {
        return a.typecode == b.typecode && a.data == b.data;
    }
    friend inline bool operator<(const UnknownReceiver& a, const UnknownReceiver& b) {
        // We don't know for certain the preference order of unknown receivers, but it is
        // _likely_ that the higher typecode has higher preference. The exact sort order
        // doesn't really matter, as unknown receivers have lower preference than known
        // receivers.
        return (a.typecode > b.typecode ||
                (a.typecode == b.typecode && a.data < b.data));
    }
};

/**
 * Receivers that can appear in a Unified Address.
 *
 * These types are given in order of preference (as defined in ZIP 316), so that sorting
 * variants by `operator<` is equivalent to sorting by preference.
 */
typedef std::variant<
    SaplingPaymentAddress,
    CScriptID,
    CKeyID,
    UnknownReceiver> Receiver;

struct ReceiverIterator {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const Receiver;
    using pointer           = const Receiver*;
    using reference         = const Receiver&;

    ReceiverIterator(std::vector<const Receiver*> sorted, size_t cur) :
        sortedReceivers(sorted), cur(cur) {}

    reference operator*() const { return *sortedReceivers[cur]; }
    pointer operator->() { return sortedReceivers[cur]; }

    ReceiverIterator& operator++() { cur++; return *this; }
    ReceiverIterator operator++(int) {
        ReceiverIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const ReceiverIterator& a, const ReceiverIterator& b) {
        return a.sortedReceivers == b.sortedReceivers && a.cur == b.cur;
    }
    friend bool operator!=(const ReceiverIterator& a, const ReceiverIterator& b) {
        return !(a == b);
    }

private:
    std::vector<const Receiver*> sortedReceivers;
    size_t cur;
};

class UnifiedAddress {
    std::vector<Receiver> receivers;

    std::vector<const Receiver*> GetSorted() const;

public:
    UnifiedAddress() {}

    /**
     * Adds the given receiver to this unified address.
     *
     * Receivers are stored in the unified address (and its encoding) in the order they
     * are added. When generating a new UA, call this method in preference order.
     *
     * Returns false if adding this receiver would result in an invalid unified address:
     * - The UA already contains this receiver type.
     * - The UA would contain both P2PKH and P2SH receivers.
     */
    bool AddReceiver(Receiver receiver);

    const std::vector<Receiver>& GetReceiversAsParsed() const { return receivers; }

    ReceiverIterator begin() const {
        return ReceiverIterator(GetSorted(), 0);
    }
    ReceiverIterator end() const {
        return ReceiverIterator(GetSorted(), receivers.size());
    }
    size_t size() const {
        return receivers.size();
    }

    std::optional<CKeyID> GetP2PKHReceiver() const;

    std::optional<CScriptID> GetP2SHReceiver() const;

    std::optional<SaplingPaymentAddress> GetSaplingReceiver() const;

    friend inline bool operator==(const UnifiedAddress& a, const UnifiedAddress& b) {
        return a.receivers == b.receivers;
    }
    friend inline bool operator!=(const UnifiedAddress& a, const UnifiedAddress& b) {
        return a.receivers != b.receivers;
    }
    friend inline bool operator<(const UnifiedAddress& a, const UnifiedAddress& b) {
        return a.receivers < b.receivers;
    }
};

class UnifiedFullViewingKeyBuilder;

class UFVKId: public uint256 {
public:
    UFVKId() : uint256() {}
    UFVKId(const uint256& in) : uint256(in) {}
};

/**
 * Wrapper for a zcash_address::unified::Ufvk.
 */
class UnifiedFullViewingKey {
private:
    std::unique_ptr<UnifiedFullViewingKeyPtr, decltype(&unified_full_viewing_key_free)> inner;

    UnifiedFullViewingKey() :
        inner(nullptr, unified_full_viewing_key_free) {}

    UnifiedFullViewingKey(UnifiedFullViewingKeyPtr* ptr) :
        inner(ptr, unified_full_viewing_key_free) {}

    friend class UnifiedFullViewingKeyBuilder;
public:
    UnifiedFullViewingKey(UnifiedFullViewingKey&& key) : inner(std::move(key.inner)) {}

    UnifiedFullViewingKey(const UnifiedFullViewingKey& key) :
        inner(unified_full_viewing_key_clone(key.inner.get()), unified_full_viewing_key_free) {}

    /**
     * This method should only be used for serialization of unified full
     * viewing keys that have been generated internally from unified spending
     * keys by Zcashd.  It is not suitable for use in any case where the
     * ZcashdUnifiedFullViewingKey value may have been produced by a
     * potentially-lossy conversion from a UnifiedFullViewingKey value that
     * originated outside of zcashd.
     */
    static UnifiedFullViewingKey FromZcashdUFVK(const ZcashdUnifiedFullViewingKey&);

    static std::optional<UnifiedFullViewingKey> Decode(
            const std::string& str,
            const KeyConstants& keyConstants);

    std::string Encode(const KeyConstants& keyConstants) const;

    std::optional<SaplingDiversifiableFullViewingKey> GetSaplingKey() const;

    std::optional<CChainablePubKey> GetTransparentKey() const;

    UFVKId GetKeyID(const KeyConstants& keyConstants) const;

    UnifiedFullViewingKey& operator=(UnifiedFullViewingKey&& key)
    {
        if (this != &key) {
            inner = std::move(key.inner);
        }
        return *this;
    }

    UnifiedFullViewingKey& operator=(const UnifiedFullViewingKey& key)
    {
        if (this != &key) {
            inner.reset(unified_full_viewing_key_clone(key.inner.get()));
        }
        return *this;
    }
};

class UnifiedFullViewingKeyBuilder {
private:
    std::optional<std::vector<uint8_t>> t_bytes;
    std::optional<std::vector<uint8_t>> sapling_bytes;
public:
    UnifiedFullViewingKeyBuilder(): t_bytes(std::nullopt), sapling_bytes(std::nullopt) {}

    bool AddTransparentKey(const CChainablePubKey&);
    bool AddSaplingKey(const SaplingDiversifiableFullViewingKey&);

    std::optional<UnifiedFullViewingKey> build() const;
};

/** Addresses that can appear in a string encoding. */
typedef std::variant<
    InvalidEncoding,
    SproutPaymentAddress,
    SaplingPaymentAddress,
    UnifiedAddress> PaymentAddress;
/** Viewing keys that can have a string encoding. */
typedef std::variant<
    InvalidEncoding,
    SproutViewingKey,
    SaplingExtendedFullViewingKey,
    UnifiedFullViewingKey> ViewingKey;
/** Spending keys that can have a string encoding. */
typedef std::variant<
    InvalidEncoding,
    SproutSpendingKey,
    SaplingExtendedSpendingKey> SpendingKey;

class AddressInfoFromSpendingKey {
public:
    std::pair<std::string, PaymentAddress> operator()(const SproutSpendingKey&) const;
    std::pair<std::string, PaymentAddress> operator()(const struct SaplingExtendedSpendingKey&) const;
    std::pair<std::string, PaymentAddress> operator()(const InvalidEncoding&) const;
};

class AddressInfoFromViewingKey {
public:
    std::pair<std::string, PaymentAddress> operator()(const SproutViewingKey&) const;
    std::pair<std::string, PaymentAddress> operator()(const struct SaplingExtendedFullViewingKey&) const;
    std::pair<std::string, PaymentAddress> operator()(const UnifiedFullViewingKey&) const;
    std::pair<std::string, PaymentAddress> operator()(const InvalidEncoding&) const;
};

} //namespace libzcash

/** Check whether a PaymentAddress is not an InvalidEncoding. */
bool IsValidPaymentAddress(const libzcash::PaymentAddress& zaddr);

/** Check whether a ViewingKey is not an InvalidEncoding. */
bool IsValidViewingKey(const libzcash::ViewingKey& vk);

/** Check whether a SpendingKey is not an InvalidEncoding. */
bool IsValidSpendingKey(const libzcash::SpendingKey& zkey);

/**
 * Gets the typecode for the given UA receiver.
 */
class TypecodeForReceiver {
public:
    TypecodeForReceiver() {}

    uint32_t operator()(const libzcash::SaplingPaymentAddress &zaddr) const;
    uint32_t operator()(const CScriptID &p2sh) const;
    uint32_t operator()(const CKeyID &p2pkh) const;
    uint32_t operator()(const libzcash::UnknownReceiver &p2pkh) const;
};

/**
 * Converts the given UA receiver to a protocol address, if it is a shielded receiver.
 */
class ReceiverToRawAddress {
public:
    ReceiverToRawAddress() {}

    std::optional<libzcash::RawAddress> operator()(const libzcash::SaplingPaymentAddress &zaddr) const;
    std::optional<libzcash::RawAddress> operator()(const CScriptID &p2sh) const;
    std::optional<libzcash::RawAddress> operator()(const CKeyID &p2pkh) const;
    std::optional<libzcash::RawAddress> operator()(const libzcash::UnknownReceiver &p2pkh) const;
};

/**
 * Returns the protocol address that should be used in transaction outputs.
 */
class RecipientForPaymentAddress {
public:
    RecipientForPaymentAddress() {}

    std::optional<libzcash::RawAddress> operator()(const libzcash::InvalidEncoding& no) const;
    std::optional<libzcash::RawAddress> operator()(const libzcash::SproutPaymentAddress &zaddr) const;
    std::optional<libzcash::RawAddress> operator()(const libzcash::SaplingPaymentAddress &zaddr) const;
    std::optional<libzcash::RawAddress> operator()(const libzcash::UnifiedAddress &uaddr) const;
};

/**
 * Returns all protocol addresses contained within the given payment address.
 */
class GetRawAddresses {
public:
    GetRawAddresses() {}

    std::set<libzcash::RawAddress> operator()(const libzcash::InvalidEncoding& no) const;
    std::set<libzcash::RawAddress> operator()(const libzcash::SproutPaymentAddress &zaddr) const;
    std::set<libzcash::RawAddress> operator()(const libzcash::SaplingPaymentAddress &zaddr) const;
    std::set<libzcash::RawAddress> operator()(const libzcash::UnifiedAddress &uaddr) const;
};

#endif // ZC_ADDRESS_H_
