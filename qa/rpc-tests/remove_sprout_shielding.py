#!/usr/bin/env python3
# Copyright (c) 2020 The Zcash developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://www.opensource.org/licenses/mit-license.php .

from decimal import Decimal
from test_framework.authproxy import JSONRPCException
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_raises_message,
    start_nodes, get_coinbase_address,
    wait_and_assert_operationid_status,
    nuparams,
    BLOSSOM_BRANCH_ID,
    HEARTWOOD_BRANCH_ID,
    CANOPY_BRANCH_ID,
    NU5_BRANCH_ID,
)

import logging

HAS_CANOPY = [
    '-nurejectoldversions=false', 
    '-anchorconfirmations=1',
    nuparams(BLOSSOM_BRANCH_ID, 205),
    nuparams(HEARTWOOD_BRANCH_ID, 210),
    nuparams(CANOPY_BRANCH_ID, 220),
    nuparams(NU5_BRANCH_ID, 225),
]

class RemoveSproutShieldingTest (BitcoinTestFramework):
    def __init__(self):
        super().__init__()
        self.num_nodes = 4
        self.cache_behavior = 'sprout'

    def setup_nodes(self):
        return start_nodes(self.num_nodes, self.options.tmpdir, extra_args=[HAS_CANOPY] * self.num_nodes)

    def run_test(self):
        # Generate blocks up to Heartwood activation
        logging.info("Generating initial blocks. Current height is 200, advance to 210 (activate Heartwood but not Canopy)")
        self.nodes[0].generate(10)
        self.sync_all()

        n0_sprout_addr0 = self.nodes[0].listaddresses()[0]['sprout']['addresses'][0]
        n2_sprout_addr = self.nodes[2].listaddresses()[0]['sprout']['addresses'][0]

        # Attempt to shield coinbase to Sprout on node 0. Should fail;
        # transfers to Sprout are no longer supported
        myopid = self.nodes[0].z_shieldcoinbase(get_coinbase_address(self.nodes[0]), n0_sprout_addr0, 0)['opid']
        wait_and_assert_operationid_status(self.nodes[0], myopid, "failed", "Sending funds into the Sprout pool is no longer supported.")

        self.nodes[0].generate(1)
        self.sync_all()

        # Check that we have the expected balance from the cached setup
        assert_equal(self.nodes[0].z_getbalance(n0_sprout_addr0), Decimal('50'))

        # Fund n0_taddr0 from previously existing Sprout funds on node 0
        n0_taddr0 = self.nodes[0].getnewaddress()
        for _ in range(3):
            recipients = [{"address": n0_taddr0, "amount": Decimal('1')}]
            myopid = self.nodes[0].z_sendmany(n0_sprout_addr0, recipients, 1, 0)
            wait_and_assert_operationid_status(self.nodes[0], myopid)
            self.sync_all()
            self.nodes[0].generate(1)
            self.sync_all()
        assert_equal(self.nodes[0].z_getbalance(n0_taddr0), Decimal('3'))

        # Create mergetoaddress taddr -> Sprout transaction and mine on node 0 before it is Canopy-aware. Should pass. This will spend the available funds in taddr0
        n1_sprout_addr0 = self.nodes[1].z_getnewaddress('sprout')
        merge_tx_0 = self.nodes[0].z_mergetoaddress(["ANY_TADDR"], n1_sprout_addr0, 0)
        wait_and_assert_operationid_status(self.nodes[0], merge_tx_0['opid'])
        print("taddr -> Sprout z_mergetoaddress tx accepted before Canopy on node 0")

        self.nodes[0].generate(1)
        self.sync_all()
        assert_equal(self.nodes[1].z_getbalance(n1_sprout_addr0), Decimal('3'))

        # Send some funds back to n0_taddr0
        recipients = [{"address": n0_taddr0, "amount": Decimal('1')}]
        myopid = self.nodes[1].z_sendmany(n1_sprout_addr0, recipients, 1, 0, 'AllowRevealedRecipients')
        wait_and_assert_operationid_status(self.nodes[1], myopid)

        # Mine to one block before Canopy activation on node 0; adding value
        # to the Sprout pool will fail now since the transaction must be
        # included in the next (or later) block, after Canopy has activated.
        self.sync_all()
        self.nodes[0].generate(4)
        self.sync_all()
        assert_equal(self.nodes[0].getblockchaininfo()['upgrades']['e9ff75a6']['status'], 'pending')
        assert_equal(self.nodes[0].z_getbalance(n0_taddr0), Decimal('1'))

        # Shield coinbase to Sprout on node 0. Should fail
        n0_coinbase_taddr = get_coinbase_address(self.nodes[0])
        n0_sprout_addr1 = self.nodes[0].z_getnewaddress('sprout')
        assert_raises_message(
            JSONRPCException,
            "Sprout shielding is not supported after Canopy",
            self.nodes[0].z_shieldcoinbase,
            n0_coinbase_taddr, n0_sprout_addr1, 0)
        print("taddr -> Sprout z_shieldcoinbase tx rejected at Canopy activation on node 0")

        # Create taddr -> Sprout z_sendmany transaction on node 0. Should fail
        n1_sprout_addr1 = self.nodes[1].z_getnewaddress('sprout')
        recipients = [{"address": n1_sprout_addr1, "amount": Decimal('1')}]
        myopid = self.nodes[0].z_sendmany(n0_taddr0, recipients, 1, 0, 'AllowRevealedSenders')
        wait_and_assert_operationid_status(self.nodes[0], myopid, "failed", "Sending funds into the Sprout pool is no longer supported.")
        print("taddr -> Sprout z_sendmany tx rejected at Canopy activation on node 0")

        # Create z_mergetoaddress [taddr, Sprout] -> Sprout transaction on node 0. Should fail
        assert_raises_message(
            JSONRPCException,
            "Sprout shielding is not supported after Canopy",
            self.nodes[0].z_mergetoaddress,
            ["ANY_TADDR", "ANY_SPROUT"], self.nodes[1].z_getnewaddress('sprout'))
        print("[taddr, Sprout] -> Sprout z_mergetoaddress tx rejected at Canopy activation on node 0")

        # Create z_mergetoaddress Sprout -> Sprout transaction on node 0. Should pass
        merge_tx_1 = self.nodes[0].z_mergetoaddress(["ANY_SPROUT"], self.nodes[1].z_getnewaddress('sprout'))
        wait_and_assert_operationid_status(self.nodes[0], merge_tx_1['opid'])
        print("Sprout -> Sprout z_mergetoaddress tx accepted at Canopy activation on node 0")

        # Activate Canopy
        self.nodes[0].generate(1)
        self.sync_all()
        assert_equal(self.nodes[0].getblockchaininfo()['upgrades']['e9ff75a6']['status'], 'active')

        # Generating a Sprout address should fail after Canopy.
        assert_raises_message(
            JSONRPCException,
            "Invalid address type, \"sprout\" is not allowed after Canopy",
            self.nodes[0].z_getnewaddress, 'sprout')
        print("Sprout z_getnewaddress rejected at Canopy activation on node 0")

        # Shield coinbase to Sapling on node 0. Should pass
        sapling_addr = self.nodes[0].z_getnewaddress('sapling')
        myopid = self.nodes[0].z_shieldcoinbase(get_coinbase_address(self.nodes[0]), sapling_addr, 0)['opid']
        wait_and_assert_operationid_status(self.nodes[0], myopid)
        print("taddr -> Sapling z_shieldcoinbase tx accepted after Canopy on node 0")

        # Mine to one block before NU5 activation.
        self.nodes[0].generate(4)
        self.sync_all()

        # Create z_mergetoaddress Sprout -> Sprout transaction on node 1. Should pass
        merge_tx_2 = self.nodes[1].z_mergetoaddress(["ANY_SPROUT"], n2_sprout_addr)
        wait_and_assert_operationid_status(self.nodes[1], merge_tx_2['opid'])
        print("Sprout -> Sprout z_mergetoaddress tx accepted at NU5 activation on node 1")

        self.nodes[1].generate(1)
        self.sync_all()

if __name__ == '__main__':
    RemoveSproutShieldingTest().main()
