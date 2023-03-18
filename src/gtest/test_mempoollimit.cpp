// Copyright (c) 2019-2023 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include <gtest/gtest.h>
#include <iostream>

#include "arith_uint256.h"
#include "mempool_limit.h"
#include "gtest/utils.h"
#include "util/time.h"
#include "util/test.h"
#include "transaction_builder.h"


const uint256 TX_ID1 = ArithToUint256(1);
const uint256 TX_ID2 = ArithToUint256(2);
const uint256 TX_ID3 = ArithToUint256(3);

TEST(MempoolLimitTests, RecentlyEvictedListAddWrapsAfterMaxSize)
{
    FixedClock clock(std::chrono::seconds(1));
    RecentlyEvictedList recentlyEvicted(&clock, 2, 100);
    recentlyEvicted.add(TX_ID1);
    recentlyEvicted.add(TX_ID2);
    recentlyEvicted.add(TX_ID3);
    // tx 1 should be overwritten by tx 3 due to maxSize 2
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
}

TEST(MempoolLimitTests, RecentlyEvictedListDoesNotContainAfterExpiry)
{
    FixedClock clock(std::chrono::seconds(1));
    // maxSize=3, timeToKeep=1
    RecentlyEvictedList recentlyEvicted(&clock, 3, 1);
    recentlyEvicted.add(TX_ID1);
    clock.Set(std::chrono::seconds(2));
    recentlyEvicted.add(TX_ID2);
    recentlyEvicted.add(TX_ID3);
    // After 1 second the txId will still be there
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    clock.Set(std::chrono::seconds(3));
    // After 2 seconds it is gone
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    clock.Set(std::chrono::seconds(4));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID2));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID3));
}

TEST(MempoolLimitTests, RecentlyEvictedDropOneAtATime)
{
    FixedClock clock(std::chrono::seconds(1));
    RecentlyEvictedList recentlyEvicted(&clock, 3, 2);
    recentlyEvicted.add(TX_ID1);
    clock.Set(std::chrono::seconds(2));
    recentlyEvicted.add(TX_ID2);
    clock.Set(std::chrono::seconds(3));
    recentlyEvicted.add(TX_ID3);
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    clock.Set(std::chrono::seconds(4));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    clock.Set(std::chrono::seconds(5));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    clock.Set(std::chrono::seconds(6));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID2));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID3));
}

TEST(MempoolLimitTests, MempoolLimitTxSetCheckSizeAfterDropping)
{
    std::set<uint256> testedDropping;
    // Run the test until we have tested dropping each of the elements
    int trialNum = 0;
    while (testedDropping.size() < 3) {
        MempoolLimitTxSet limitSet(MIN_TX_COST * 2);
        EXPECT_EQ(0, limitSet.getTotalWeight());
        limitSet.add(TX_ID1, MIN_TX_COST, MIN_TX_COST);
        EXPECT_EQ(4000, limitSet.getTotalWeight());
        limitSet.add(TX_ID2, MIN_TX_COST, MIN_TX_COST);
        EXPECT_EQ(8000, limitSet.getTotalWeight());
        EXPECT_FALSE(limitSet.maybeDropRandom().has_value());
        limitSet.add(TX_ID3, MIN_TX_COST, MIN_TX_COST + LOW_FEE_PENALTY);
        EXPECT_EQ(12000 + LOW_FEE_PENALTY, limitSet.getTotalWeight());
        std::optional<uint256> drop = limitSet.maybeDropRandom();
        ASSERT_TRUE(drop.has_value());
        uint256 txid = drop.value();
        testedDropping.insert(txid);
        // Do not continue to test if a particular trial fails
        ASSERT_EQ(txid == TX_ID3 ? 8000 : 8000 + LOW_FEE_PENALTY, limitSet.getTotalWeight());
    }
}

TEST(MempoolLimitTests, MempoolCostAndEvictionWeight)
{
    LoadProofParameters();

    // The transaction creation is based on the test:
    // test_transaction_builder.cpp/TEST(TransactionBuilder, SetFee)
    auto consensusParams = RegtestActivateSapling();

    auto sk = libzcash::SaplingSpendingKey::random();
    auto testNote = GetTestSaplingNote(sk.default_address(), 50000);

    // Default fee
    {
        auto builder = TransactionBuilder(consensusParams, 1, std::nullopt);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 25000, {});

        auto [cost, evictionWeight] = MempoolCostAndEvictionWeight(builder.Build().GetTxOrThrow(), DEFAULT_FEE);
        EXPECT_EQ(MIN_TX_COST, cost);
        EXPECT_EQ(MIN_TX_COST, evictionWeight);
    }

    // Lower than standard fee
    {
        auto builder = TransactionBuilder(consensusParams, 1, std::nullopt);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 25000, {});
        static_assert(DEFAULT_FEE == 1000);
        builder.SetFee(DEFAULT_FEE-1);

        auto [cost, evictionWeight] = MempoolCostAndEvictionWeight(builder.Build().GetTxOrThrow(), DEFAULT_FEE-1);
        EXPECT_EQ(MIN_TX_COST, cost);
        EXPECT_EQ(MIN_TX_COST + LOW_FEE_PENALTY, evictionWeight);
    }

    // Larger Tx
    {
        auto builder = TransactionBuilder(consensusParams, 1, std::nullopt);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});

        auto result = builder.Build();
        if (result.IsError()) {
            std::cerr << result.FormatError() << std::endl;
        }
        auto [cost, evictionWeight] = MempoolCostAndEvictionWeight(result.GetTxOrThrow(), DEFAULT_FEE);
        EXPECT_EQ(5168, cost);
        EXPECT_EQ(5168, evictionWeight);
    }

    RegtestDeactivateSapling();
}
