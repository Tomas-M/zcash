// Copyright (c) 2019 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php 

#include <gtest/gtest.h>
#include <iostream>

#include "arith_uint256.h"
#include "mempoollimit.h"
#include "utiltime.h"
#include "utiltest.h"
#include "transaction_builder.h"


const uint256 TX_ID1 = ArithToUint256(1);
const uint256 TX_ID2 = ArithToUint256(2);
const uint256 TX_ID3 = ArithToUint256(3);

TEST(MempoolLimitTests, RecentlyEvictedList_AddWrapsAfterMaxSize)
{
    RecentlyEvictedList recentlyEvicted(2, 100);
    SetMockTime(1);
    recentlyEvicted.add(TX_ID1);
    recentlyEvicted.add(TX_ID2);
    recentlyEvicted.add(TX_ID3);
    // tx 1 should be overwritten by tx 3 due to maxSize 2
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
}

TEST(MempoolLimitTests, RecentlyEvictedList_DoesNotContainAfterExpiry)
{
    SetMockTime(1);
    // maxSize=3, timeToKeep=1
    RecentlyEvictedList recentlyEvicted(3, 1);
    recentlyEvicted.add(TX_ID1);
    SetMockTime(2);
    recentlyEvicted.add(TX_ID2);
    recentlyEvicted.add(TX_ID3);
    // After 1 second the txId will still be there
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    SetMockTime(3);
    // After 2 second it is gone
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID1));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID2));
    EXPECT_TRUE(recentlyEvicted.contains(TX_ID3));
    SetMockTime(4);
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID2));
    EXPECT_FALSE(recentlyEvicted.contains(TX_ID3));
}

TEST(MempoolLimitTests, WeightedTxTree_CheckSizeAfterDropping)
{
    std::set<uint256> testedDropping;
    // Run the test until we have tested dropping each of the elements
    int trialNum = 0;
    while (testedDropping.size() < 3) {
        WeightedTxTree tree(MIN_TX_WEIGHT * 2);
        EXPECT_EQ(0, tree.getTotalWeight().weight);
        EXPECT_EQ(0, tree.getTotalWeight().lowFeePenaltyWeight);
        tree.add(WeightedTxInfo(TX_ID1, TxWeight(MIN_TX_WEIGHT, MIN_TX_WEIGHT)));
        EXPECT_EQ(4000, tree.getTotalWeight().weight);
        EXPECT_EQ(4000, tree.getTotalWeight().lowFeePenaltyWeight);
        tree.add(WeightedTxInfo(TX_ID2, TxWeight(MIN_TX_WEIGHT, MIN_TX_WEIGHT)));
        EXPECT_EQ(8000, tree.getTotalWeight().weight);
        EXPECT_EQ(8000, tree.getTotalWeight().lowFeePenaltyWeight);
        EXPECT_FALSE(tree.maybeDropRandom().is_initialized());
        tree.add(WeightedTxInfo(TX_ID3, TxWeight(MIN_TX_WEIGHT, MIN_TX_WEIGHT + LOW_FEE_PENALTY)));
        EXPECT_EQ(12000, tree.getTotalWeight().weight);
        EXPECT_EQ(12000 + LOW_FEE_PENALTY, tree.getTotalWeight().lowFeePenaltyWeight);
        boost::optional<uint256> drop = tree.maybeDropRandom();
        ASSERT_TRUE(drop.is_initialized());
        uint256 txid = drop.get();
        std::cerr << "Trial " << trialNum++ << ": dropped " << txid.ToString() << std::endl;
        testedDropping.insert(txid);
        // Do not continue to test if a particular trial fails
        ASSERT_EQ(8000, tree.getTotalWeight().weight);
        ASSERT_EQ(txid == TX_ID3 ? 8000 : 8000 + LOW_FEE_PENALTY, tree.getTotalWeight().lowFeePenaltyWeight);
    }
    std::cerr << "All 3 scenarios tested in " << trialNum << " trials" << std::endl;
}

TEST(MempoolLimitTests, WeightedTXInfo_FromTx)
{
    // The transaction creation is based on the test:
    // test_transaction_builder.cpp/TEST(TransactionBuilder, SetFee)
    auto consensusParams = RegtestActivateSapling();

    auto sk = libzcash::SaplingSpendingKey::random();
    auto testNote = GetTestSaplingNote(sk.default_address(), 50000);

    // Default fee
    {
        auto builder = TransactionBuilder(consensusParams, 1);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 25000, {});
        
        WeightedTxInfo info = WeightedTxInfo::from(builder.Build().GetTxOrThrow(), 10000);
        EXPECT_EQ(MIN_TX_WEIGHT, info.txWeight.weight);
        EXPECT_EQ(MIN_TX_WEIGHT, info.txWeight.lowFeePenaltyWeight);
    }
    
    // Lower than standard fee
    {
        auto builder = TransactionBuilder(consensusParams, 1);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 25000, {});
        builder.SetFee(9999);

        WeightedTxInfo info = WeightedTxInfo::from(builder.Build().GetTxOrThrow(), 9999);
        EXPECT_EQ(MIN_TX_WEIGHT, info.txWeight.weight);
        EXPECT_EQ(MIN_TX_WEIGHT + LOW_FEE_PENALTY, info.txWeight.lowFeePenaltyWeight);
    }

    // Larger Tx
    {
        auto builder = TransactionBuilder(consensusParams, 1);
        builder.AddSaplingSpend(sk.expanded_spending_key(), testNote.note, testNote.tree.root(), testNote.tree.witness());
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        builder.AddSaplingOutput(sk.full_viewing_key().ovk, sk.default_address(), 5000, {});
        
        auto result = builder.Build();
        if (result.IsError()) {
            std::cerr << result.GetError() << std::endl;
        }
        WeightedTxInfo info = WeightedTxInfo::from(result.GetTxOrThrow(), 10000);
        EXPECT_EQ(5124, info.txWeight.weight);
        EXPECT_EQ(5124, info.txWeight.lowFeePenaltyWeight);
    }
    
    RegtestDeactivateSapling();
}
