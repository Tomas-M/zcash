// Copyright (c) 2012-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coins.h"

#include <assert.h>

#ifdef ZEROCASH
CCoinsImmuntable MakeFakeZerocoinCCoin(){
    CTransaction dummy;
    {
    CScript scriptPubKey;
    scriptPubKey.clear();
    scriptPubKey << FLAG_ZC_POUR;
    CTxOut out(0,scriptPubKey);
    dummy.vout.push_back(out);
    }
    {
     CScript scriptPubKey;
     scriptPubKey.clear();
     scriptPubKey << FLAG_ZC_MINT;
     CTxOut out(1,scriptPubKey);
     dummy.vout.push_back(out);
    }
    CCoinsImmuntable fake(dummy);
    return fake;
}
bool isSerialSpendable(CCoinsView &v,const uint256 &txid, const uint256 &serial){
    uint256 value;
    return isSerialSpendable(v,txid,serial);
}
bool isSerialSpendable(CCoinsView &v,const uint256 &txid, const uint256 &serial,uint256 &idOfTxThatSpentIt){
    if(v.GetSerial(txid,idOfTxThatSpentIt)){
        return idOfTxThatSpentIt == always_spendable_txid;
    }else{
        return true;
    }
}
bool markeSerialAsSpent(CCoinsView &v, const uint256 &serial,const uint256 &txid){
    LogPrint("zerocoin","zerocoin markeSerialAsSpent: marking serial %s as spent by %s \n",serial.ToString(),txid.ToString());
    return v.SetSerial(serial,txid);
}
bool markSerialAsUnSpent(CCoinsView &v, const uint256 &serial){
    LogPrint("zerocoin","zerocoin markSerialAsUnSpent: marking serial %s as unspent.\n",serial.ToString());
    return v.SetSerial(serial,always_spendable_txid);
}
bool CCoinsViewCache::GetSerial(const uint256& serial, uint256& txid) {
    if (cacheSerial.count(serial)) {
        txid = cacheSerial[serial];
        return false;
    }
    if (base->GetSerial(serial, txid)) {
        // since always spendable is the marker for deleting a txid,
        // the base code should NEVER actually return a value wit that in it.
        assert(txid != always_spendable_txid);
        cacheSerial[serial] = txid;
        return true;
    }
    return false;
}
#endif /* ZEROCASH */

// calculate number of bytes for the bitmask, and its number of non-zero bytes
// each bit in the bitmask represents the availability of one output, but the
// availabilities of the first two outputs are encoded separately
void CCoins::CalcMaskSize(unsigned int &nBytes, unsigned int &nNonzeroBytes) const {
    unsigned int nLastUsedByte = 0;
    for (unsigned int b = 0; 2+b*8 < vout.size(); b++) {
        bool fZero = true;
        for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++) {
            if (!vout[2+b*8+i].IsNull()) {
                fZero = false;
                continue;
            }
        }
        if (!fZero) {
            nLastUsedByte = b + 1;
            nNonzeroBytes++;
        }
    }
    nBytes += nLastUsedByte;
}

bool CCoins::Spend(const COutPoint &out, CTxInUndo &undo) {
    if (out.n >= vout.size())
        return false;
    if (vout[out.n].IsNull())
        return false;
    undo = CTxInUndo(vout[out.n]);
    vout[out.n].SetNull();
    Cleanup();
    if (vout.size() == 0) {
        undo.nHeight = nHeight;
        undo.fCoinBase = fCoinBase;
        undo.nVersion = this->nVersion;
    }
    return true;
}

bool CCoins::Spend(int nPos) {
    CTxInUndo undo;
    COutPoint out(0, nPos);
    return Spend(out, undo);
}


bool CCoinsView::GetCoins(const uint256 &txid, CCoins &coins) { return false; }
bool CCoinsView::SetCoins(const uint256 &txid, const CCoins &coins) { return false; }
bool CCoinsView::HaveCoins(const uint256 &txid) { return false; }
#ifdef ZEROCASH
bool CCoinsView::SetSerial(const uint256 &serial, const uint256 &txid) {return false;}
bool CCoinsView::GetSerial(const uint256 &serial, uint256 &txid) {return false;}
#endif /* ZEROCASH */

uint256 CCoinsView::GetBestBlock() { return uint256(0); }
bool CCoinsView::SetBestBlock(const uint256 &hashBlock) { return false; }
#ifndef ZEROCASH
bool CCoinsView::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlock) { return false; }
#else /* ZEROCASH */
bool CCoinsView::BatchWrite(const std::map<uint256, CCoins> &mapCoins,  const  std::map<uint256, uint256> &mapSerial, const uint256 &hashBlock) { return false; }
#endif /* ZEROCASH */
bool CCoinsView::GetStats(CCoinsStats &stats) { return false; }


CCoinsViewBacked::CCoinsViewBacked(CCoinsView &viewIn) : base(&viewIn) { }
bool CCoinsViewBacked::GetCoins(const uint256 &txid, CCoins &coins) { return base->GetCoins(txid, coins); }
bool CCoinsViewBacked::SetCoins(const uint256 &txid, const CCoins &coins) { return base->SetCoins(txid, coins); }
bool CCoinsViewBacked::HaveCoins(const uint256 &txid) { return base->HaveCoins(txid); }
uint256 CCoinsViewBacked::GetBestBlock() { return base->GetBestBlock(); }
bool CCoinsViewBacked::SetBestBlock(const uint256 &hashBlock) { return base->SetBestBlock(hashBlock); }
void CCoinsViewBacked::SetBackend(CCoinsView &viewIn) { base = &viewIn; }
#ifndef ZEROCASH
bool CCoinsViewBacked::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlock) { return base->BatchWrite(mapCoins, hashBlock); }
#else /* ZEROCASH */
bool CCoinsViewBacked::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const  std::map<uint256, uint256> &mapSerial, const uint256 &hashBlock) { return base->BatchWrite(mapCoins,mapSerial, hashBlock); }
bool CCoinsViewBacked::GetSerial(const uint256& serial, uint256& txid) { return base->GetSerial(serial,txid); }
bool CCoinsViewBacked::SetSerial(const uint256& serial, const uint256& txid) { return base->SetSerial(serial,txid); }
#endif /* ZEROCASH */
bool CCoinsViewBacked::GetStats(CCoinsStats &stats) { return base->GetStats(stats); }

#ifndef ZEROCASH
CCoinsViewCache::CCoinsViewCache(CCoinsView &baseIn, bool fDummy) : CCoinsViewBacked(baseIn), hashBlock(0) { }
#else /* ZEROCASH */
CCoinsViewCache::CCoinsViewCache(CCoinsView &baseIn, bool fDummy) : CCoinsViewBacked(baseIn), hashBlock(0),zerocoin_input( MakeFakeZerocoinCCoin()) {}

bool CCoinsViewCache::SetSerial(const uint256& serial, const uint256& txid) {
    cacheSerial[serial]=txid;
    return true;
}
#endif /* ZEROCASH */

bool CCoinsViewCache::GetCoins(const uint256 &txid, CCoins &coins) {
#ifdef ZEROCASH
    if(txid == always_spendable_txid){
        coins=zerocoin_input;
        return true;
    }
#endif /* ZEROCASH */
    if (cacheCoins.count(txid)) {
        coins = cacheCoins[txid];
        return true;
    }
    if (base->GetCoins(txid, coins)) {
        cacheCoins[txid] = coins;
        return true;
    }
    return false;
}

std::map<uint256,CCoins>::iterator CCoinsViewCache::FetchCoins(const uint256 &txid) {
    std::map<uint256,CCoins>::iterator it = cacheCoins.lower_bound(txid);
    if (it != cacheCoins.end() && it->first == txid)
        return it;
    CCoins tmp;
    if (!base->GetCoins(txid,tmp))
        return cacheCoins.end();
    std::map<uint256,CCoins>::iterator ret = cacheCoins.insert(it, std::make_pair(txid, CCoins()));
    tmp.swap(ret->second);
    return ret;
}

CCoins &CCoinsViewCache::GetCoins(const uint256 &txid) {
#ifdef ZEROCASH
    if(txid == always_spendable_txid){
        return zerocoin_input;
    }
#endif /* ZEROCASH */
    std::map<uint256,CCoins>::iterator it = FetchCoins(txid);
    assert(it != cacheCoins.end());
    return it->second;
}

bool CCoinsViewCache::SetCoins(const uint256 &txid, const CCoins &coins) {
#ifdef ZEROCASH
    // The fake CCoins can never be spent, so ignore all attempts to modify it.
    if(txid == always_spendable_txid){
        return true;
    }
#endif /* ZEROCASH */
    cacheCoins[txid] = coins;
    return true;
}

bool CCoinsViewCache::HaveCoins(const uint256 &txid) {
#ifdef ZEROCASH
    if(txid == always_spendable_txid){
        return true;
    }
#endif /* ZEROCASH */
    return FetchCoins(txid) != cacheCoins.end();
}

uint256 CCoinsViewCache::GetBestBlock() {
    if (hashBlock == uint256(0))
        hashBlock = base->GetBestBlock();
    return hashBlock;
}

bool CCoinsViewCache::SetBestBlock(const uint256 &hashBlockIn) {
    hashBlock = hashBlockIn;
    return true;
}

#ifndef ZEROCASH
bool CCoinsViewCache::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlockIn) {
    for (std::map<uint256, CCoins>::const_iterator it = mapCoins.begin(); it != mapCoins.end(); it++)
        cacheCoins[it->first] = it->second;
    hashBlock = hashBlockIn;
    return true;
}
#else /* ZEROCASH */
bool CCoinsViewCache::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const  std::map<uint256, uint256> &mapSerial, const uint256 &hashBlockIn) {
    for (std::map<uint256, CCoins>::const_iterator it = mapCoins.begin(); it != mapCoins.end(); it++){
        assert(it->first != always_spendable_txid);
        cacheCoins[it->first] = it->second;
    }
    for (std::map<uint256, uint256>::const_iterator it = mapSerial.begin(); it != mapSerial.end(); it++){
        cacheSerial[it->first] = it->second;
    }
    hashBlock = hashBlockIn;
    return true;
}
#endif /* ZEROCASH */

bool CCoinsViewCache::Flush() {
#ifndef ZEROCASH
    bool fOk = base->BatchWrite(cacheCoins, hashBlock);
    if (fOk)
        cacheCoins.clear();
#else /* ZEROCASH */
    bool fOk = base->BatchWrite(cacheCoins,cacheSerial, hashBlock);
    if (fOk) {
        cacheCoins.clear();
        cacheSerial.clear();
    }
#endif /* ZEROCASH */
    return fOk;
}

unsigned int CCoinsViewCache::GetCacheSize() {
#ifndef ZEROCASH
    return cacheCoins.size();
#else /* ZEROCASH */
    return cacheCoins.size() + cacheSerial.size();
#endif /* ZEROCASH */
}

const CTxOut &CCoinsViewCache::GetOutputFor(const CTxIn& input)
{
    const CCoins &coins = GetCoins(input.prevout.hash);
    assert(coins.IsAvailable(input.prevout.n));
    return coins.vout[input.prevout.n];
}

int64_t CCoinsViewCache::GetValueIn(const CTransaction& tx)
{
    if (tx.IsCoinBase())
        return 0;

    int64_t nResult = 0;
#ifndef ZEROCASH
    for (unsigned int i = 0; i < tx.vin.size(); i++)
        nResult += GetOutputFor(tx.vin[i]).nValue;
#else /* ZEROCASH */
    for (unsigned int i = 0; i < tx.vin.size(); i++){
        nResult += GetOutputFor(tx.vin[i]).nValue;

        // Pours are an input to a transaction, so we count vPub as the input value
        if(tx.vin[i].IsZCPour()){
            nResult +=tx.vin[i].GetBtcContributionOfZerocoinTransaction();
        }
    }
#endif /* ZEROCASH */

    return nResult;
}

bool CCoinsViewCache::HaveInputs(const CTransaction& tx)
{
    if (!tx.IsCoinBase()) {
        // first check whether information about the prevout hash is available
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            const COutPoint &prevout = tx.vin[i].prevout;
            if (!HaveCoins(prevout.hash))
                return false;
        }

        // then check whether the actual outputs are available
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            const COutPoint &prevout = tx.vin[i].prevout;
            const CCoins &coins = GetCoins(prevout.hash);
            if (!coins.IsAvailable(prevout.n))
                return false;
        }
    }
    return true;
}

double CCoinsViewCache::GetPriority(const CTransaction &tx, int nHeight)
{
    if (tx.IsCoinBase())
        return 0.0;
    double dResult = 0.0;
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
        const CCoins &coins = GetCoins(txin.prevout.hash);
        if (!coins.IsAvailable(txin.prevout.n)) continue;
        if (coins.nHeight < nHeight) {
            dResult += coins.vout[txin.prevout.n].nValue * (nHeight-coins.nHeight);
        }
    }
    return tx.ComputePriority(dResult);
}
