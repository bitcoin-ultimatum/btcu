#!/usr/bin/env python3
# Copyright (c) 2022 The BTCU developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test mempool limiting together/eviction with the wallet."""

from test_framework.test_framework import BtcuTestFramework
from test_framework.util import *
from time import sleep, time

class BTCU_RPCValidatorsTest(BtcuTestFramework):
    def set_test_params(self):
        self.num_nodes = 3
        self.setup_clean_chain = True
        self.extra_args = [[]] * self.num_nodes
        self.extra_args[0].append('-sporkkey=932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi')
        self.extra_args[0].append('-debug=staking')

    def setup_network(self, split=False):
        super().setup_network()
        connect_nodes_bi(self.nodes, 0, 2)

    def run_test(self):
        sporkName = "SPORK_1017_LEASING_ENFORCEMENT"
        self.master_port = 23666 # regtest port
        self.master_alias = "test_mnv"
        self.master_id = 0
        self.miner_id = 1
        self.nominer_id = 2

        self.master = self.nodes[self.master_id]
        self.miner = self.nodes[self.miner_id]
        self.nominer = self.nodes[self.nominer_id]

        #activate leasing
        res = self.activate_spork(0, sporkName)

        #generate to POS
        self.miner.generate(250)

        self.sync_all()
        self.master_priv_key = self.master.createmasternodekey()
        self.master_priv_key = self.master_priv_key.split(" : ")[0]

        #send 1000 btcu to masterrnode
        mn_address = self.master.getaccountaddress(self.master_alias)
        self.miner.sendtoaddress(mn_address, 1000)

        #lease to mn address for validator condtiion
        self.miner.leasetoaddress(mn_address, 10000, 1647873656)

        #generate up to 30 blocks for mn output be ready (from sendto mn_address)
        self.miner.generate(30)
        self.sync_all()

        def setupMN():
            self.log.info("  creating masternode..")

            master_output = self.master.getmasternodeoutputs()
            assert_equal(len(master_output), 1)
            master_index = master_output[0]["outputidx"]

            self.log.info("  updating masternode.conf...")

            conf_data = self.master_alias + " 127.0.0.1:" + str(self.master_port)
            conf_data += " " + str(self.master_priv_key)
            conf_data += " " + str(master_output[0]["txhash"])
            conf_data += " " + str(master_index)
            conf_path = os.path.join(self.options.tmpdir, "node{}".format(self.master_id), "regtest", "masternode.conf")
            with open(conf_path, "a+") as file_object:
                file_object.write("\n")
                file_object.write(conf_data)

        def initMN():
            self.log.info("  initialize masternode..")
            self.master.initmasternode(self.master_priv_key, "127.0.0.1:"+str(self.master_port))
            # self.generatePosSync(self.miner2_id)

            SYNC_FINISHED = [999] * self.num_nodes
            synced = [-1] * self.num_nodes
            timeout = time() + 45
            while synced != SYNC_FINISHED and time() < timeout:
                for i in range(self.num_nodes):
                    if synced[i] != SYNC_FINISHED[i]:
                        synced[i] = self.nodes[i].mnsync("status")["RequestedMasternodeAssets"]
                if synced != SYNC_FINISHED:
                    sleep(5)
            assert_equal(synced, SYNC_FINISHED)

        def startMN():
            self.log.info("  starting masternode..")
            ret = self.master.startmasternode("alias", "false", self.master_alias, True)
            assert_equal(ret["result"], "success")

        self.log.info("Creating masternode...")

        setupMN()
        initMN()
        startMN()
        
        

        #send to masternode address 100 btcu for validator transactions
        self.miner.sendtoaddress(mn_address, 100)

        #message registration validator in non-registration epoch
        assert_equal(self.master.mnregvalidator(self.master_alias), "39 blocks until start of the registration phase")

        #generate 39 blocks until validator registration phase
        self.miner.generate(39)
        self.sync_all()

        #exception non-masternode
        assert_raises_rpc_error(-8, 'CreateValidatorReg failed', self.miner.mnregvalidator, self.master_alias)

        #exception with fake name masternode
        assert_raises_rpc_error(-8, "CreateValidatorReg failed", self.master.mnregvalidator, "fakename")

        #reg validator phase
        self.master.mnregvalidator(self.master_alias)

        #exception double registration transaction in mempool
        assert_raises_rpc_error(-4, 'Failed CheckValidatorTransaction(): duplicated-validator-transaction', self.master.mnregvalidator, self.master_alias)

        self.miner.generate(10)
        self.sync_all()

        #exception double registration transaction in block history
        assert_raises_rpc_error(-4, 'Failed CheckValidatorTransaction(): bad-validator-already-registered', self.master.mnregvalidator, self.master_alias)

        registered_validators = self.miner.mnregvalidatorlist()
        self.log.info("Registered validators:")
        self.log.info(registered_validators)

        #vote to validator
        input = [{ "pubkey" : registered_validators[0]['pubkey'], "vote" : "yes"}]

        #message voting validator in non-vote epoch
        assert_equal(self.miner.mnvotevalidator(input), '30 blocks until start of the voting phase')

        self.miner.generate(30)
        self.sync_all()

        #exception with no validator node voting
        assert_raises_rpc_error(-8, "Failed to get validator key: CreateValidatorVote failed. You don't have permission for voting transaction.", self.nominer.mnvotevalidator, input)

        #vote
        self.miner.mnvotevalidator(input)

        #exception double voting transaction in mempool
        assert_raises_rpc_error(-4, 'Failed CheckValidatorTransaction(): duplicated-validator-transaction', self.miner.mnvotevalidator, input)

        self.miner.generate(10)
        self.sync_all()

        #exception double voting transaction in block history
        assert_raises_rpc_error(-4, 'Failed CheckValidatorTransaction(): bad-validator-already-voted', self.miner.mnvotevalidator, input)

        #list votes
        self.log.info("Voted validators:")
        self.log.info(self.master.mnvotevalidatorlist())


        self.miner.generate(30)
        self.sync_all()

        #list validators
        self.log.info("Active custom validators:")
        self.log.info(self.master.mnvalidatorlist())

        #try generate block with custom validator signing
        count1 = self.master.getinfo()["blocks"]
        self.log.info("Generating block with custom validator")
        self.master.generate(1, True)
        self.sync_all()
        count2 = self.master.getinfo()["blocks"]
        #check count of blocks before and after generate with custom validator
        assert_equal(count1, count2 - 1)

        self.log.info("done")


if __name__ == '__main__':
    BTCU_RPCValidatorsTest().main()
