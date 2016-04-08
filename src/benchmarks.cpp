#include <unistd.h>
#include <boost/filesystem.hpp>
#include "zerocash/ZerocashParams.h"
#include "coins.h"
#include "util.h"
#include "init.h"
#include "primitives/transaction.h"

#include "benchmarks.h"

struct timeval tv_start;

void timer_start()
{
    gettimeofday(&tv_start, 0);
}

double timer_stop()
{
    double elapsed;
    struct timeval tv_end;
    gettimeofday(&tv_end, 0);
    elapsed = double(tv_end.tv_sec-tv_start.tv_sec) +
        (tv_end.tv_usec-tv_start.tv_usec)/double(1000000);
    return elapsed;
}

double benchmark_sleep()
{
    timer_start();
    sleep(1);
    return timer_stop();
}

double benchmark_parameter_loading()
{
    // FIXME: this is duplicated with the actual loading code
    boost::filesystem::path pk_path = ZC_GetParamsDir() / "zc-testnet-public-alpha-proving.key";
    boost::filesystem::path vk_path = ZC_GetParamsDir() / "zc-testnet-public-alpha-verification.key";

    timer_start();
    auto vk_loaded = libzerocash::ZerocashParams::LoadVerificationKeyFromFile(
        vk_path.string(),
        INCREMENTAL_MERKLE_TREE_DEPTH
    );
    auto pk_loaded = libzerocash::ZerocashParams::LoadProvingKeyFromFile(
        pk_path.string(),
        INCREMENTAL_MERKLE_TREE_DEPTH
    );
    libzerocash::ZerocashParams zerocashParams = libzerocash::ZerocashParams(
        INCREMENTAL_MERKLE_TREE_DEPTH,
        &pk_loaded,
        &vk_loaded
    );
    return timer_stop();
}

double benchmark_create_joinsplit()
{
    CScript scriptPubKey;

    std::vector<PourInput> vpourin;
    std::vector<PourOutput> vpourout;

    while (vpourin.size() < NUM_POUR_INPUTS) {
        vpourin.push_back(PourInput(INCREMENTAL_MERKLE_TREE_DEPTH));
    }

    while (vpourout.size() < NUM_POUR_OUTPUTS) {
        vpourout.push_back(PourOutput(0));
    }

    uint256 anchor;

    timer_start();
    CPourTx pourtx(*pzerocashParams,
                   scriptPubKey,
                   anchor,
                   {vpourin[0], vpourin[1]},
                   {vpourout[0], vpourout[1]},
                   0,
                   0);
    double ret = timer_stop();
    assert(pourtx.Verify(*pzerocashParams));
    return ret;
}

extern double benchmark_equihash_solve()
{

}
