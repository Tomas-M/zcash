/******************************************************************************
 * Copyright © 2014-2016 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

int32_t IS_KOMODO_NOTARY,USE_EXTERNAL_PUBKEY;
std::string NOTARY_PUBKEY;
uint8_t NOTARY_PUBKEY33[33];

int32_t ASSETCHAINS_SHORTFLAG;
extern char ASSETCHAINS_SYMBOL[16];
extern uint16_t ASSETCHAINS_PORT;
extern uint32_t ASSETCHAIN_INIT;
extern uint32_t ASSETCHAINS_MAGIC;
extern uint64_t ASSETCHAINS_SUPPLY;

int32_t NOTARIZED_HEIGHT,Num_nutxos,KMDHEIGHT = 43000;
uint256 NOTARIZED_HASH,NOTARIZED_DESTTXID;
pthread_mutex_t komodo_mutex;
uint32_t KOMODO_INITDONE;
char KMDUSERPASS[1024]; uint16_t BITCOIND_PORT = 7771;
uint64_t KOMODO_DEPOSIT,PENDING_KOMODO_TX;

#define SATOSHIDEN ((uint64_t)100000000L)
#define dstr(x) ((double)(x) / SATOSHIDEN)
#define portable_mutex_t pthread_mutex_t
#define portable_mutex_init(ptr) pthread_mutex_init(ptr,NULL)
#define portable_mutex_lock pthread_mutex_lock
#define portable_mutex_unlock pthread_mutex_unlock

struct allocitem { uint32_t allocsize,type; };
struct queueitem { struct queueitem *next,*prev; uint32_t allocsize,type;  };

typedef struct queue
{
	struct queueitem *list;
	pthread_mutex_t mutex;
    char name[64],initflag;
} queue_t;

union _bits256 { uint8_t bytes[32]; uint16_t ushorts[16]; uint32_t uints[8]; uint64_t ulongs[4]; uint64_t txid; };
typedef union _bits256 bits256;

#include "mini-gmp.c"
#include "uthash.h"
#include "utlist.h"

#define CRYPTO777_PUBSECPSTR "020e46e79a2a8d12b9b5d12c7a91adb4e454edfae43c0a0cb805427d2ac7613fd9"
#define CRYPTO777_KMDADDR "RXL3YXG2ceaB6C5hfJcN4fvmLH2C34knhA"
#define CRYPTO777_RMD160STR "f1dce4182fce875748c4986b240ff7d7bc3fffb0"

#define KOMODO_PUBTYPE 60

struct sha256_vstate { uint64_t length; uint32_t state[8],curlen; uint8_t buf[64]; };
struct rmd160_vstate { uint64_t length; uint8_t buf[64]; uint32_t curlen, state[5]; };

// following is ported from libtom

#define STORE32L(x, y)                                                                     \
{ (y)[3] = (uint8_t)(((x)>>24)&255); (y)[2] = (uint8_t)(((x)>>16)&255);   \
(y)[1] = (uint8_t)(((x)>>8)&255); (y)[0] = (uint8_t)((x)&255); }

#define LOAD32L(x, y)                            \
{ x = (uint32_t)(((uint64_t)((y)[3] & 255)<<24) | \
((uint32_t)((y)[2] & 255)<<16) | \
((uint32_t)((y)[1] & 255)<<8)  | \
((uint32_t)((y)[0] & 255))); }

#define STORE64L(x, y)                                                                     \
{ (y)[7] = (uint8_t)(((x)>>56)&255); (y)[6] = (uint8_t)(((x)>>48)&255);   \
(y)[5] = (uint8_t)(((x)>>40)&255); (y)[4] = (uint8_t)(((x)>>32)&255);   \
(y)[3] = (uint8_t)(((x)>>24)&255); (y)[2] = (uint8_t)(((x)>>16)&255);   \
(y)[1] = (uint8_t)(((x)>>8)&255); (y)[0] = (uint8_t)((x)&255); }

#define LOAD64L(x, y)                                                       \
{ x = (((uint64_t)((y)[7] & 255))<<56)|(((uint64_t)((y)[6] & 255))<<48)| \
(((uint64_t)((y)[5] & 255))<<40)|(((uint64_t)((y)[4] & 255))<<32)| \
(((uint64_t)((y)[3] & 255))<<24)|(((uint64_t)((y)[2] & 255))<<16)| \
(((uint64_t)((y)[1] & 255))<<8)|(((uint64_t)((y)[0] & 255))); }

#define STORE32H(x, y)                                                                     \
{ (y)[0] = (uint8_t)(((x)>>24)&255); (y)[1] = (uint8_t)(((x)>>16)&255);   \
(y)[2] = (uint8_t)(((x)>>8)&255); (y)[3] = (uint8_t)((x)&255); }

#define LOAD32H(x, y)                            \
{ x = (uint32_t)(((uint64_t)((y)[0] & 255)<<24) | \
((uint32_t)((y)[1] & 255)<<16) | \
((uint32_t)((y)[2] & 255)<<8)  | \
((uint32_t)((y)[3] & 255))); }

#define STORE64H(x, y)                                                                     \
{ (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);     \
(y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);     \
(y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);     \
(y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }

#define LOAD64H(x, y)                                                      \
{ x = (((uint64_t)((y)[0] & 255))<<56)|(((uint64_t)((y)[1] & 255))<<48) | \
(((uint64_t)((y)[2] & 255))<<40)|(((uint64_t)((y)[3] & 255))<<32) | \
(((uint64_t)((y)[4] & 255))<<24)|(((uint64_t)((y)[5] & 255))<<16) | \
(((uint64_t)((y)[6] & 255))<<8)|(((uint64_t)((y)[7] & 255))); }

// Various logical functions
#define RORc(x, y) ( ((((uint32_t)(x)&0xFFFFFFFFUL)>>(uint32_t)((y)&31)) | ((uint32_t)(x)<<(uint32_t)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y))
#define S(x, n)         RORc((x),(n))
#define R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))
#define MIN(x, y) ( ((x)<(y))?(x):(y) )

static inline int32_t sha256_vcompress(struct sha256_vstate * md,uint8_t *buf)
{
    uint32_t S[8],W[64],t0,t1,i;
    for (i=0; i<8; i++) // copy state into S
        S[i] = md->state[i];
    for (i=0; i<16; i++) // copy the state into 512-bits into W[0..15]
        LOAD32H(W[i],buf + (4*i));
    for (i=16; i<64; i++) // fill W[16..63]
        W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
    
#define RND(a,b,c,d,e,f,g,h,i,ki)                    \
t0 = h + Sigma1(e) + Ch(e, f, g) + ki + W[i];   \
t1 = Sigma0(a) + Maj(a, b, c);                  \
d += t0;                                        \
h  = t0 + t1;
    
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],0,0x428a2f98);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],1,0x71374491);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],2,0xb5c0fbcf);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],3,0xe9b5dba5);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],4,0x3956c25b);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],5,0x59f111f1);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],6,0x923f82a4);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],7,0xab1c5ed5);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],8,0xd807aa98);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],9,0x12835b01);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],10,0x243185be);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],11,0x550c7dc3);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],12,0x72be5d74);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],13,0x80deb1fe);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],14,0x9bdc06a7);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],15,0xc19bf174);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],16,0xe49b69c1);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],17,0xefbe4786);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],18,0x0fc19dc6);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],19,0x240ca1cc);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],20,0x2de92c6f);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],21,0x4a7484aa);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],22,0x5cb0a9dc);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],23,0x76f988da);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],24,0x983e5152);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],25,0xa831c66d);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],26,0xb00327c8);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],27,0xbf597fc7);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],28,0xc6e00bf3);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],29,0xd5a79147);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],30,0x06ca6351);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],31,0x14292967);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],32,0x27b70a85);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],33,0x2e1b2138);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],34,0x4d2c6dfc);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],35,0x53380d13);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],36,0x650a7354);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],37,0x766a0abb);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],38,0x81c2c92e);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],39,0x92722c85);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],40,0xa2bfe8a1);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],41,0xa81a664b);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],42,0xc24b8b70);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],43,0xc76c51a3);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],44,0xd192e819);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],45,0xd6990624);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],46,0xf40e3585);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],47,0x106aa070);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],48,0x19a4c116);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],49,0x1e376c08);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],50,0x2748774c);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],51,0x34b0bcb5);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],52,0x391c0cb3);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],53,0x4ed8aa4a);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],54,0x5b9cca4f);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],55,0x682e6ff3);
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],56,0x748f82ee);
    RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],57,0x78a5636f);
    RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],58,0x84c87814);
    RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],59,0x8cc70208);
    RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],60,0x90befffa);
    RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],61,0xa4506ceb);
    RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],62,0xbef9a3f7);
    RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],63,0xc67178f2);
#undef RND
    for (i=0; i<8; i++) // feedback
        md->state[i] = md->state[i] + S[i];
    return(0);
}

#undef RORc
#undef Ch
#undef Maj
#undef S
#undef R
#undef Sigma0
#undef Sigma1
#undef Gamma0
#undef Gamma1

static inline void sha256_vinit(struct sha256_vstate * md)
{
    md->curlen = 0;
    md->length = 0;
    md->state[0] = 0x6A09E667UL;
    md->state[1] = 0xBB67AE85UL;
    md->state[2] = 0x3C6EF372UL;
    md->state[3] = 0xA54FF53AUL;
    md->state[4] = 0x510E527FUL;
    md->state[5] = 0x9B05688CUL;
    md->state[6] = 0x1F83D9ABUL;
    md->state[7] = 0x5BE0CD19UL;
}

static inline int32_t sha256_vprocess(struct sha256_vstate *md,const uint8_t *in,uint64_t inlen)
{
    uint64_t n; int32_t err;
    if ( md->curlen > sizeof(md->buf) )
        return(-1);
    while ( inlen > 0 )
    {
        if ( md->curlen == 0 && inlen >= 64 )
        {
            if ( (err= sha256_vcompress(md,(uint8_t *)in)) != 0 )
                return(err);
            md->length += 64 * 8, in += 64, inlen -= 64;
        }
        else
        {
            n = MIN(inlen,64 - md->curlen);
            memcpy(md->buf + md->curlen,in,(size_t)n);
            md->curlen += n, in += n, inlen -= n;
            if ( md->curlen == 64 )
            {
                if ( (err= sha256_vcompress(md,md->buf)) != 0 )
                    return(err);
                md->length += 8*64;
                md->curlen = 0;
            }
        }
    }
    return(0);
}

static inline int32_t sha256_vdone(struct sha256_vstate *md,uint8_t *out)
{
    int32_t i;
    if ( md->curlen >= sizeof(md->buf) )
        return(-1);
    md->length += md->curlen * 8; // increase the length of the message
    md->buf[md->curlen++] = (uint8_t)0x80; // append the '1' bit
    // if len > 56 bytes we append zeros then compress.  Then we can fall back to padding zeros and length encoding like normal.
    if ( md->curlen > 56 )
    {
        while ( md->curlen < 64 )
            md->buf[md->curlen++] = (uint8_t)0;
        sha256_vcompress(md,md->buf);
        md->curlen = 0;
    }
    while ( md->curlen < 56 ) // pad upto 56 bytes of zeroes
        md->buf[md->curlen++] = (uint8_t)0;
    STORE64H(md->length,md->buf+56); // store length
    sha256_vcompress(md,md->buf);
    for (i=0; i<8; i++) // copy output
        STORE32H(md->state[i],out+(4*i));
    return(0);
}

void vcalc_sha256(char deprecated[(256 >> 3) * 2 + 1],uint8_t hash[256 >> 3],uint8_t *src,int32_t len)
{
    struct sha256_vstate md;
    sha256_vinit(&md);
    sha256_vprocess(&md,src,len);
    sha256_vdone(&md,hash);
}

bits256 bits256_doublesha256(char *deprecated,uint8_t *data,int32_t datalen)
{
    bits256 hash,hash2; int32_t i;
    vcalc_sha256(0,hash.bytes,data,datalen);
    vcalc_sha256(0,hash2.bytes,hash.bytes,sizeof(hash));
    for (i=0; i<sizeof(hash); i++)
        hash.bytes[i] = hash2.bytes[sizeof(hash) - 1 - i];
    return(hash);
}

// rmd160: the five basic functions F(), G() and H()
#define F(x, y, z)        ((x) ^ (y) ^ (z))
#define G(x, y, z)        (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z)        (((x) | ~(y)) ^ (z))
#define I(x, y, z)        (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z)        ((x) ^ ((y) | ~(z)))
#define ROLc(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)

/* the ten basic operations FF() through III() */
#define FF(a, b, c, d, e, x, s)        \
(a) += F((b), (c), (d)) + (x);\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define GG(a, b, c, d, e, x, s)        \
(a) += G((b), (c), (d)) + (x) + 0x5a827999UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define HH(a, b, c, d, e, x, s)        \
(a) += H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define II(a, b, c, d, e, x, s)        \
(a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define JJ(a, b, c, d, e, x, s)        \
(a) += J((b), (c), (d)) + (x) + 0xa953fd4eUL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define FFF(a, b, c, d, e, x, s)        \
(a) += F((b), (c), (d)) + (x);\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define GGG(a, b, c, d, e, x, s)        \
(a) += G((b), (c), (d)) + (x) + 0x7a6d76e9UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define HHH(a, b, c, d, e, x, s)        \
(a) += H((b), (c), (d)) + (x) + 0x6d703ef3UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define III(a, b, c, d, e, x, s)        \
(a) += I((b), (c), (d)) + (x) + 0x5c4dd124UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

#define JJJ(a, b, c, d, e, x, s)        \
(a) += J((b), (c), (d)) + (x) + 0x50a28be6UL;\
(a) = ROLc((a), (s)) + (e);\
(c) = ROLc((c), 10);

static int32_t rmd160_vcompress(struct rmd160_vstate *md,uint8_t *buf)
{
    uint32_t aa,bb,cc,dd,ee,aaa,bbb,ccc,ddd,eee,X[16];
    int i;
    
    /* load words X */
    for (i = 0; i < 16; i++){
        LOAD32L(X[i], buf + (4 * i));
    }
    
    /* load state */
    aa = aaa = md->state[0];
    bb = bbb = md->state[1];
    cc = ccc = md->state[2];
    dd = ddd = md->state[3];
    ee = eee = md->state[4];
    
    /* round 1 */
    FF(aa, bb, cc, dd, ee, X[ 0], 11);
    FF(ee, aa, bb, cc, dd, X[ 1], 14);
    FF(dd, ee, aa, bb, cc, X[ 2], 15);
    FF(cc, dd, ee, aa, bb, X[ 3], 12);
    FF(bb, cc, dd, ee, aa, X[ 4],  5);
    FF(aa, bb, cc, dd, ee, X[ 5],  8);
    FF(ee, aa, bb, cc, dd, X[ 6],  7);
    FF(dd, ee, aa, bb, cc, X[ 7],  9);
    FF(cc, dd, ee, aa, bb, X[ 8], 11);
    FF(bb, cc, dd, ee, aa, X[ 9], 13);
    FF(aa, bb, cc, dd, ee, X[10], 14);
    FF(ee, aa, bb, cc, dd, X[11], 15);
    FF(dd, ee, aa, bb, cc, X[12],  6);
    FF(cc, dd, ee, aa, bb, X[13],  7);
    FF(bb, cc, dd, ee, aa, X[14],  9);
    FF(aa, bb, cc, dd, ee, X[15],  8);
    
    /* round 2 */
    GG(ee, aa, bb, cc, dd, X[ 7],  7);
    GG(dd, ee, aa, bb, cc, X[ 4],  6);
    GG(cc, dd, ee, aa, bb, X[13],  8);
    GG(bb, cc, dd, ee, aa, X[ 1], 13);
    GG(aa, bb, cc, dd, ee, X[10], 11);
    GG(ee, aa, bb, cc, dd, X[ 6],  9);
    GG(dd, ee, aa, bb, cc, X[15],  7);
    GG(cc, dd, ee, aa, bb, X[ 3], 15);
    GG(bb, cc, dd, ee, aa, X[12],  7);
    GG(aa, bb, cc, dd, ee, X[ 0], 12);
    GG(ee, aa, bb, cc, dd, X[ 9], 15);
    GG(dd, ee, aa, bb, cc, X[ 5],  9);
    GG(cc, dd, ee, aa, bb, X[ 2], 11);
    GG(bb, cc, dd, ee, aa, X[14],  7);
    GG(aa, bb, cc, dd, ee, X[11], 13);
    GG(ee, aa, bb, cc, dd, X[ 8], 12);
    
    /* round 3 */
    HH(dd, ee, aa, bb, cc, X[ 3], 11);
    HH(cc, dd, ee, aa, bb, X[10], 13);
    HH(bb, cc, dd, ee, aa, X[14],  6);
    HH(aa, bb, cc, dd, ee, X[ 4],  7);
    HH(ee, aa, bb, cc, dd, X[ 9], 14);
    HH(dd, ee, aa, bb, cc, X[15],  9);
    HH(cc, dd, ee, aa, bb, X[ 8], 13);
    HH(bb, cc, dd, ee, aa, X[ 1], 15);
    HH(aa, bb, cc, dd, ee, X[ 2], 14);
    HH(ee, aa, bb, cc, dd, X[ 7],  8);
    HH(dd, ee, aa, bb, cc, X[ 0], 13);
    HH(cc, dd, ee, aa, bb, X[ 6],  6);
    HH(bb, cc, dd, ee, aa, X[13],  5);
    HH(aa, bb, cc, dd, ee, X[11], 12);
    HH(ee, aa, bb, cc, dd, X[ 5],  7);
    HH(dd, ee, aa, bb, cc, X[12],  5);
    
    /* round 4 */
    II(cc, dd, ee, aa, bb, X[ 1], 11);
    II(bb, cc, dd, ee, aa, X[ 9], 12);
    II(aa, bb, cc, dd, ee, X[11], 14);
    II(ee, aa, bb, cc, dd, X[10], 15);
    II(dd, ee, aa, bb, cc, X[ 0], 14);
    II(cc, dd, ee, aa, bb, X[ 8], 15);
    II(bb, cc, dd, ee, aa, X[12],  9);
    II(aa, bb, cc, dd, ee, X[ 4],  8);
    II(ee, aa, bb, cc, dd, X[13],  9);
    II(dd, ee, aa, bb, cc, X[ 3], 14);
    II(cc, dd, ee, aa, bb, X[ 7],  5);
    II(bb, cc, dd, ee, aa, X[15],  6);
    II(aa, bb, cc, dd, ee, X[14],  8);
    II(ee, aa, bb, cc, dd, X[ 5],  6);
    II(dd, ee, aa, bb, cc, X[ 6],  5);
    II(cc, dd, ee, aa, bb, X[ 2], 12);
    
    /* round 5 */
    JJ(bb, cc, dd, ee, aa, X[ 4],  9);
    JJ(aa, bb, cc, dd, ee, X[ 0], 15);
    JJ(ee, aa, bb, cc, dd, X[ 5],  5);
    JJ(dd, ee, aa, bb, cc, X[ 9], 11);
    JJ(cc, dd, ee, aa, bb, X[ 7],  6);
    JJ(bb, cc, dd, ee, aa, X[12],  8);
    JJ(aa, bb, cc, dd, ee, X[ 2], 13);
    JJ(ee, aa, bb, cc, dd, X[10], 12);
    JJ(dd, ee, aa, bb, cc, X[14],  5);
    JJ(cc, dd, ee, aa, bb, X[ 1], 12);
    JJ(bb, cc, dd, ee, aa, X[ 3], 13);
    JJ(aa, bb, cc, dd, ee, X[ 8], 14);
    JJ(ee, aa, bb, cc, dd, X[11], 11);
    JJ(dd, ee, aa, bb, cc, X[ 6],  8);
    JJ(cc, dd, ee, aa, bb, X[15],  5);
    JJ(bb, cc, dd, ee, aa, X[13],  6);
    
    /* parallel round 1 */
    JJJ(aaa, bbb, ccc, ddd, eee, X[ 5],  8);
    JJJ(eee, aaa, bbb, ccc, ddd, X[14],  9);
    JJJ(ddd, eee, aaa, bbb, ccc, X[ 7],  9);
    JJJ(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
    JJJ(bbb, ccc, ddd, eee, aaa, X[ 9], 13);
    JJJ(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
    JJJ(eee, aaa, bbb, ccc, ddd, X[11], 15);
    JJJ(ddd, eee, aaa, bbb, ccc, X[ 4],  5);
    JJJ(ccc, ddd, eee, aaa, bbb, X[13],  7);
    JJJ(bbb, ccc, ddd, eee, aaa, X[ 6],  7);
    JJJ(aaa, bbb, ccc, ddd, eee, X[15],  8);
    JJJ(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
    JJJ(ddd, eee, aaa, bbb, ccc, X[ 1], 14);
    JJJ(ccc, ddd, eee, aaa, bbb, X[10], 14);
    JJJ(bbb, ccc, ddd, eee, aaa, X[ 3], 12);
    JJJ(aaa, bbb, ccc, ddd, eee, X[12],  6);
    
    /* parallel round 2 */
    III(eee, aaa, bbb, ccc, ddd, X[ 6],  9);
    III(ddd, eee, aaa, bbb, ccc, X[11], 13);
    III(ccc, ddd, eee, aaa, bbb, X[ 3], 15);
    III(bbb, ccc, ddd, eee, aaa, X[ 7],  7);
    III(aaa, bbb, ccc, ddd, eee, X[ 0], 12);
    III(eee, aaa, bbb, ccc, ddd, X[13],  8);
    III(ddd, eee, aaa, bbb, ccc, X[ 5],  9);
    III(ccc, ddd, eee, aaa, bbb, X[10], 11);
    III(bbb, ccc, ddd, eee, aaa, X[14],  7);
    III(aaa, bbb, ccc, ddd, eee, X[15],  7);
    III(eee, aaa, bbb, ccc, ddd, X[ 8], 12);
    III(ddd, eee, aaa, bbb, ccc, X[12],  7);
    III(ccc, ddd, eee, aaa, bbb, X[ 4],  6);
    III(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
    III(aaa, bbb, ccc, ddd, eee, X[ 1], 13);
    III(eee, aaa, bbb, ccc, ddd, X[ 2], 11);
    
    /* parallel round 3 */
    HHH(ddd, eee, aaa, bbb, ccc, X[15],  9);
    HHH(ccc, ddd, eee, aaa, bbb, X[ 5],  7);
    HHH(bbb, ccc, ddd, eee, aaa, X[ 1], 15);
    HHH(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
    HHH(eee, aaa, bbb, ccc, ddd, X[ 7],  8);
    HHH(ddd, eee, aaa, bbb, ccc, X[14],  6);
    HHH(ccc, ddd, eee, aaa, bbb, X[ 6],  6);
    HHH(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
    HHH(aaa, bbb, ccc, ddd, eee, X[11], 12);
    HHH(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
    HHH(ddd, eee, aaa, bbb, ccc, X[12],  5);
    HHH(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
    HHH(bbb, ccc, ddd, eee, aaa, X[10], 13);
    HHH(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
    HHH(eee, aaa, bbb, ccc, ddd, X[ 4],  7);
    HHH(ddd, eee, aaa, bbb, ccc, X[13],  5);
    
    /* parallel round 4 */
    GGG(ccc, ddd, eee, aaa, bbb, X[ 8], 15);
    GGG(bbb, ccc, ddd, eee, aaa, X[ 6],  5);
    GGG(aaa, bbb, ccc, ddd, eee, X[ 4],  8);
    GGG(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
    GGG(ddd, eee, aaa, bbb, ccc, X[ 3], 14);
    GGG(ccc, ddd, eee, aaa, bbb, X[11], 14);
    GGG(bbb, ccc, ddd, eee, aaa, X[15],  6);
    GGG(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
    GGG(eee, aaa, bbb, ccc, ddd, X[ 5],  6);
    GGG(ddd, eee, aaa, bbb, ccc, X[12],  9);
    GGG(ccc, ddd, eee, aaa, bbb, X[ 2], 12);
    GGG(bbb, ccc, ddd, eee, aaa, X[13],  9);
    GGG(aaa, bbb, ccc, ddd, eee, X[ 9], 12);
    GGG(eee, aaa, bbb, ccc, ddd, X[ 7],  5);
    GGG(ddd, eee, aaa, bbb, ccc, X[10], 15);
    GGG(ccc, ddd, eee, aaa, bbb, X[14],  8);
    
    /* parallel round 5 */
    FFF(bbb, ccc, ddd, eee, aaa, X[12] ,  8);
    FFF(aaa, bbb, ccc, ddd, eee, X[15] ,  5);
    FFF(eee, aaa, bbb, ccc, ddd, X[10] , 12);
    FFF(ddd, eee, aaa, bbb, ccc, X[ 4] ,  9);
    FFF(ccc, ddd, eee, aaa, bbb, X[ 1] , 12);
    FFF(bbb, ccc, ddd, eee, aaa, X[ 5] ,  5);
    FFF(aaa, bbb, ccc, ddd, eee, X[ 8] , 14);
    FFF(eee, aaa, bbb, ccc, ddd, X[ 7] ,  6);
    FFF(ddd, eee, aaa, bbb, ccc, X[ 6] ,  8);
    FFF(ccc, ddd, eee, aaa, bbb, X[ 2] , 13);
    FFF(bbb, ccc, ddd, eee, aaa, X[13] ,  6);
    FFF(aaa, bbb, ccc, ddd, eee, X[14] ,  5);
    FFF(eee, aaa, bbb, ccc, ddd, X[ 0] , 15);
    FFF(ddd, eee, aaa, bbb, ccc, X[ 3] , 13);
    FFF(ccc, ddd, eee, aaa, bbb, X[ 9] , 11);
    FFF(bbb, ccc, ddd, eee, aaa, X[11] , 11);
    
    /* combine results */
    ddd += cc + md->state[1];               /* final result for md->state[0] */
    md->state[1] = md->state[2] + dd + eee;
    md->state[2] = md->state[3] + ee + aaa;
    md->state[3] = md->state[4] + aa + bbb;
    md->state[4] = md->state[0] + bb + ccc;
    md->state[0] = ddd;
    
    return 0;
}

/**
 Initialize the hash state
 @param md   The hash state you wish to initialize
 @return 0 if successful
 */
int rmd160_vinit(struct rmd160_vstate * md)
{
    md->state[0] = 0x67452301UL;
    md->state[1] = 0xefcdab89UL;
    md->state[2] = 0x98badcfeUL;
    md->state[3] = 0x10325476UL;
    md->state[4] = 0xc3d2e1f0UL;
    md->curlen   = 0;
    md->length   = 0;
    return 0;
}
#define HASH_PROCESS(func_name, compress_name, state_var, block_size)                       \
int func_name (struct rmd160_vstate * md, const unsigned char *in, unsigned long inlen)               \
{                                                                                           \
unsigned long n;                                                                        \
int           err;                                                                      \
if (md->curlen > sizeof(md->buf)) {                             \
return -1;                                                            \
}                                                                                       \
while (inlen > 0) {                                                                     \
if (md->curlen == 0 && inlen >= block_size) {                           \
if ((err = compress_name (md, (unsigned char *)in)) != 0) {               \
return err;                                                                   \
}                                                                                \
md->length += block_size * 8;                                        \
in             += block_size;                                                    \
inlen          -= block_size;                                                    \
} else {                                                                            \
n = MIN(inlen, (block_size - md->curlen));                           \
memcpy(md->buf + md->curlen, in, (size_t)n);              \
md->curlen += n;                                                     \
in             += n;                                                             \
inlen          -= n;                                                             \
if (md->curlen == block_size) {                                      \
if ((err = compress_name (md, md->buf)) != 0) {            \
return err;                                                                \
}                                                                             \
md->length += 8*block_size;                                       \
md->curlen = 0;                                                   \
}                                                                                \
}                                                                                    \
}                                                                                       \
return 0;                                                                        \
}

/**
 Process a block of memory though the hash
 @param md     The hash state
 @param in     The data to hash
 @param inlen  The length of the data (octets)
 @return 0 if successful
 */
HASH_PROCESS(rmd160_vprocess, rmd160_vcompress, rmd160, 64)

/**
 Terminate the hash to get the digest
 @param md  The hash state
 @param out [out] The destination of the hash (20 bytes)
 @return 0 if successful
 */
int rmd160_vdone(struct rmd160_vstate * md, unsigned char *out)
{
    int i;
    if (md->curlen >= sizeof(md->buf)) {
        return -1;
    }
    /* increase the length of the message */
    md->length += md->curlen * 8;
    
    /* append the '1' bit */
    md->buf[md->curlen++] = (unsigned char)0x80;
    
    /* if the length is currently above 56 bytes we append zeros
     * then compress.  Then we can fall back to padding zeros and length
     * encoding like normal.
     */
    if (md->curlen > 56) {
        while (md->curlen < 64) {
            md->buf[md->curlen++] = (unsigned char)0;
        }
        rmd160_vcompress(md, md->buf);
        md->curlen = 0;
    }
    /* pad upto 56 bytes of zeroes */
    while (md->curlen < 56) {
        md->buf[md->curlen++] = (unsigned char)0;
    }
    /* store length */
    STORE64L(md->length, md->buf+56);
    rmd160_vcompress(md, md->buf);
    /* copy output */
    for (i = 0; i < 5; i++) {
        STORE32L(md->state[i], out+(4*i));
    }
    return 0;
}

void calc_rmd160(char deprecated[41],uint8_t buf[20],uint8_t *msg,int32_t len)
{
    struct rmd160_vstate md;
    rmd160_vinit(&md);
    rmd160_vprocess(&md,msg,len);
    rmd160_vdone(&md, buf);
}

static const uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t calc_crc32(uint32_t crc,const void *buf,size_t size)
{
	const uint8_t *p;
    
	p = (const uint8_t *)buf;
	crc = crc ^ ~0U;
    
	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    
	return crc ^ ~0U;
}

void calc_rmd160_sha256(uint8_t rmd160[20],uint8_t *data,int32_t datalen)
{
    bits256 hash;
    vcalc_sha256(0,hash.bytes,data,datalen);
    calc_rmd160(0,rmd160,hash.bytes,sizeof(hash));
}

int32_t bitcoin_addr2rmd160(uint8_t *addrtypep,uint8_t rmd160[20],char *coinaddr)
{
    bits256 hash; uint8_t *buf,_buf[25]; int32_t len;
    memset(rmd160,0,20);
    *addrtypep = 0;
    buf = _buf;
    if ( (len= bitcoin_base58decode(buf,coinaddr)) >= 4 )
    {
        // validate with trailing hash, then remove hash
        hash = bits256_doublesha256(0,buf,21);
        *addrtypep = *buf;
        memcpy(rmd160,buf+1,20);
        if ( (buf[21]&0xff) == hash.bytes[31] && (buf[22]&0xff) == hash.bytes[30] &&(buf[23]&0xff) == hash.bytes[29] && (buf[24]&0xff) == hash.bytes[28] )
        {
            //printf("coinaddr.(%s) valid checksum addrtype.%02x\n",coinaddr,*addrtypep);
            return(20);
        }
        else
        {
            int32_t i;
            if ( len > 20 )
            {
                hash = bits256_doublesha256(0,buf,len);
            }
            for (i=0; i<len; i++)
                printf("%02x ",buf[i]);
            printf("\nhex checkhash.(%s) len.%d mismatch %02x %02x %02x %02x vs %02x %02x %02x %02x\n",coinaddr,len,buf[len-1]&0xff,buf[len-2]&0xff,buf[len-3]&0xff,buf[len-4]&0xff,hash.bytes[31],hash.bytes[30],hash.bytes[29],hash.bytes[28]);
        }
    }
	return(0);
}

char *bitcoin_address(char *coinaddr,uint8_t addrtype,uint8_t *pubkey_or_rmd160,int32_t len)
{
    int32_t i; uint8_t data[25]; bits256 hash;// char checkaddr[65];
    if ( len != 20 )
        calc_rmd160_sha256(data+1,pubkey_or_rmd160,len);
    else memcpy(data+1,pubkey_or_rmd160,20);
    //btc_convrmd160(checkaddr,addrtype,data+1);
    data[0] = addrtype;
    hash = bits256_doublesha256(0,data,21);
    for (i=0; i<4; i++)
        data[21+i] = hash.bytes[31-i];
    if ( (coinaddr= bitcoin_base58encode(coinaddr,data,25)) != 0 )
    {
        //uint8_t checktype,rmd160[20];
        //bitcoin_addr2rmd160(&checktype,rmd160,coinaddr);
        //if ( strcmp(checkaddr,coinaddr) != 0 )
        //    printf("checkaddr.(%s) vs coinaddr.(%s) %02x vs [%02x] memcmp.%d\n",checkaddr,coinaddr,addrtype,checktype,memcmp(rmd160,data+1,20));
    }
    return(coinaddr);
}

int32_t _unhex(char c)
{
    if ( c >= '0' && c <= '9' )
        return(c - '0');
    else if ( c >= 'a' && c <= 'f' )
        return(c - 'a' + 10);
    else if ( c >= 'A' && c <= 'F' )
        return(c - 'A' + 10);
    return(-1);
}

int32_t is_hexstr(char *str,int32_t n)
{
    int32_t i;
    if ( str == 0 || str[0] == 0 )
        return(0);
    for (i=0; str[i]!=0; i++)
    {
        if ( n > 0 && i >= n )
            break;
        if ( _unhex(str[i]) < 0 )
            break;
    }
    if ( n == 0 )
        return(i);
    return(i == n);
}

int32_t unhex(char c)
{
    int32_t hex;
    if ( (hex= _unhex(c)) < 0 )
    {
        //printf("unhex: illegal hexchar.(%c)\n",c);
    }
    return(hex);
}

unsigned char _decode_hex(char *hex) { return((unhex(hex[0])<<4) | unhex(hex[1])); }

int32_t decode_hex(uint8_t *bytes,int32_t n,char *hex)
{
    int32_t adjust,i = 0;
    //printf("decode.(%s)\n",hex);
    if ( is_hexstr(hex,n) == 0 )
    {
        memset(bytes,0,n);
        return(n);
    }
    if ( n == 0 || (hex[n*2+1] == 0 && hex[n*2] != 0) )
    {
        if ( n > 0 )
        {
            bytes[0] = unhex(hex[0]);
            printf("decode_hex n.%d hex[0] (%c) -> %d hex.(%s) [n*2+1: %d] [n*2: %d %c] len.%ld\n",n,hex[0],bytes[0],hex,hex[n*2+1],hex[n*2],hex[n*2],(long)strlen(hex));
        }
        bytes++;
        hex++;
        adjust = 1;
    } else adjust = 0;
    if ( n > 0 )
    {
        for (i=0; i<n; i++)
            bytes[i] = _decode_hex(&hex[i*2]);
    }
    //bytes[i] = 0;
    return(n + adjust);
}

char hexbyte(int32_t c)
{
    c &= 0xf;
    if ( c < 10 )
        return('0'+c);
    else if ( c < 16 )
        return('a'+c-10);
    else return(0);
}

int32_t init_hexbytes_noT(char *hexbytes,unsigned char *message,long len)
{
    int32_t i;
    if ( len <= 0 )
    {
        hexbytes[0] = 0;
        return(1);
    }
    for (i=0; i<len; i++)
    {
        hexbytes[i*2] = hexbyte((message[i]>>4) & 0xf);
        hexbytes[i*2 + 1] = hexbyte(message[i] & 0xf);
        //printf("i.%d (%02x) [%c%c]\n",i,message[i],hexbytes[i*2],hexbytes[i*2+1]);
    }
    hexbytes[len*2] = 0;
    //printf("len.%ld\n",len*2+1);
    return((int32_t)len*2+1);
}

char *bits256_str(char hexstr[65],bits256 x)
{
    init_hexbytes_noT(hexstr,x.bytes,sizeof(x));
    return(hexstr);
}

int32_t iguana_rwnum(int32_t rwflag,uint8_t *serialized,int32_t len,void *endianedp)
{
    int32_t i; uint64_t x;
    if ( rwflag == 0 )
    {
        x = 0;
        for (i=len-1; i>=0; i--)
        {
            x <<= 8;
            x |= serialized[i];
        }
        switch ( len )
        {
            case 1: *(uint8_t *)endianedp = (uint8_t)x; break;
            case 2: *(uint16_t *)endianedp = (uint16_t)x; break;
            case 4: *(uint32_t *)endianedp = (uint32_t)x; break;
            case 8: *(uint64_t *)endianedp = (uint64_t)x; break;
        }
    }
    else
    {
        x = 0;
        switch ( len )
        {
            case 1: x = *(uint8_t *)endianedp; break;
            case 2: x = *(uint16_t *)endianedp; break;
            case 4: x = *(uint32_t *)endianedp; break;
            case 8: x = *(uint64_t *)endianedp; break;
        }
        for (i=0; i<len; i++,x >>= 8)
            serialized[i] = (uint8_t)(x & 0xff);
    }
    return(len);
}

int32_t iguana_rwbignum(int32_t rwflag,uint8_t *serialized,int32_t len,uint8_t *endianedp)
{
    int32_t i;
    if ( rwflag == 0 )
    {
        for (i=0; i<len; i++)
            endianedp[i] = serialized[i];
    }
    else
    {
        for (i=0; i<len; i++)
            serialized[i] = endianedp[i];
    }
    return(len);
}

int32_t komodo_scriptitemlen(int32_t *opretlenp,uint8_t *script)
{
    int32_t opretlen,len = 0;
    if ( (opretlen= script[len++]) >= 0x4c )
    {
        if ( opretlen == 0x4c )
            opretlen = script[len++];
        else if ( opretlen == 0x4d )
        {
            opretlen = script[len++];
            opretlen = (opretlen << 8) | script[len++];
        }
    }
    *opretlenp = opretlen;
    return(len);
}

int32_t komodo_opreturnscript(uint8_t *script,uint8_t type,uint8_t *opret,int32_t opretlen)
{
    int32_t offset = 0;
    script[offset++] = 0x6a;
    opretlen++;
    if ( opretlen >= 0x4c )
    {
        if ( opretlen > 0xff )
        {
            script[offset++] = 0x4d;
            script[offset++] = opretlen & 0xff;
            script[offset++] = (opretlen >> 8) & 0xff;
        }
        else
        {
            script[offset++] = 0x4c;
            script[offset++] = opretlen;
        }
    } else script[offset++] = opretlen;
    script[offset++] = type; // covered by opretlen
    memcpy(&script[offset],opret,opretlen-1);
    return(offset + opretlen - 1);
}

long _stripwhite(char *buf,int accept)
{
    int32_t i,j,c;
    if ( buf == 0 || buf[0] == 0 )
        return(0);
    for (i=j=0; buf[i]!=0; i++)
    {
        buf[j] = c = buf[i];
        if ( c == accept || (c != ' ' && c != '\n' && c != '\r' && c != '\t' && c != '\b') )
            j++;
    }
    buf[j] = 0;
    return(j);
}

char *clonestr(char *str)
{
    char *clone;
    if ( str == 0 || str[0] == 0 )
    {
        printf("warning cloning nullstr.%p\n",str);
#ifdef __APPLE__
        while ( 1 ) sleep(1);
#endif
        str = (char *)"<nullstr>";
    }
    clone = (char *)malloc(strlen(str)+16);
    strcpy(clone,str);
    return(clone);
}

int32_t safecopy(char *dest,char *src,long len)
{
    int32_t i = -1;
    if ( src != 0 && dest != 0 && src != dest )
    {
        if ( dest != 0 )
            memset(dest,0,len);
        for (i=0; i<len&&src[i]!=0; i++)
            dest[i] = src[i];
        if ( i == len )
        {
            printf("safecopy: %s too long %ld\n",src,len);
#ifdef __APPLE__
            //getchar();
#endif
            return(-1);
        }
        dest[i] = 0;
    }
    return(i);
}

char *parse_conf_line(char *line,char *field)
{
    line += strlen(field);
    for (; *line!='='&&*line!=0; line++)
        break;
    if ( *line == 0 )
        return(0);
    if ( *line == '=' )
        line++;
    while ( line[strlen(line)-1] == '\r' || line[strlen(line)-1] == '\n' || line[strlen(line)-1] == ' ' )
        line[strlen(line)-1] = 0;
    //printf("LINE.(%s)\n",line);
    _stripwhite(line,0);
    return(clonestr(line));
}

double OS_milliseconds()
{
    struct timeval tv; double millis;
    gettimeofday(&tv,NULL);
    millis = ((double)tv.tv_sec * 1000. + (double)tv.tv_usec / 1000.);
    //printf("tv_sec.%ld usec.%d %f\n",tv.tv_sec,tv.tv_usec,millis);
    return(millis);
}

void OS_randombytes(unsigned char *x,long xlen)
{
    static int fd = -1;
    int32_t i;
    if (fd == -1) {
        for (;;) {
            fd = open("/dev/urandom",O_RDONLY);
            if (fd != -1) break;
            sleep(1);
        }
    }
    while (xlen > 0) {
        if (xlen < 1048576) i = (int32_t)xlen; else i = 1048576;
        i = (int32_t)read(fd,x,i);
        if (i < 1) {
            sleep(1);
            continue;
        }
        if ( 0 )
        {
            int32_t j;
            for (j=0; j<i; j++)
                printf("%02x ",x[j]);
            printf("-> %p\n",x);
        }
        x += i;
        xlen -= i;
    }
}

void lock_queue(queue_t *queue)
{
    if ( queue->initflag == 0 )
    {
        portable_mutex_init(&queue->mutex);
        queue->initflag = 1;
    }
	portable_mutex_lock(&queue->mutex);
}

void queue_enqueue(char *name,queue_t *queue,struct queueitem *item)
{
    if ( queue->name[0] == 0 && name != 0 && name[0] != 0 )
        strcpy(queue->name,name);
    if ( item == 0 )
    {
        printf("FATAL type error: queueing empty value\n");
        return;
    }
    lock_queue(queue);
    DL_APPEND(queue->list,item);
    portable_mutex_unlock(&queue->mutex);
}

struct queueitem *queue_dequeue(queue_t *queue)
{
    struct queueitem *item = 0;
    lock_queue(queue);
    if ( queue->list != 0 )
    {
        item = queue->list;
        DL_DELETE(queue->list,item);
    }
	portable_mutex_unlock(&queue->mutex);
    return(item);
}

void *queue_delete(queue_t *queue,struct queueitem *copy,int32_t copysize)
{
    struct queueitem *item = 0;
    lock_queue(queue);
    if ( queue->list != 0 )
    {
        DL_FOREACH(queue->list,item)
        {
            if ( item == copy || (item->allocsize == copysize && memcmp((void *)((long)item + sizeof(struct queueitem)),(void *)((long)copy + sizeof(struct queueitem)),copysize) == 0) )
            {
                DL_DELETE(queue->list,item);
                portable_mutex_unlock(&queue->mutex);
                printf("name.(%s) deleted item.%p list.%p\n",queue->name,item,queue->list);
                return(item);
            }
        }
    }
	portable_mutex_unlock(&queue->mutex);
    return(0);
}

void *queue_free(queue_t *queue)
{
    struct queueitem *item = 0;
    lock_queue(queue);
    if ( queue->list != 0 )
    {
        DL_FOREACH(queue->list,item)
        {
            DL_DELETE(queue->list,item);
            free(item);
        }
        //printf("name.(%s) dequeue.%p list.%p\n",queue->name,item,queue->list);
    }
	portable_mutex_unlock(&queue->mutex);
    return(0);
}

void *queue_clone(queue_t *clone,queue_t *queue,int32_t size)
{
    struct queueitem *ptr,*item = 0;
    lock_queue(queue);
    if ( queue->list != 0 )
    {
        DL_FOREACH(queue->list,item)
        {
            ptr = (struct queueitem *)calloc(1,sizeof(*ptr));
            memcpy(ptr,item,size);
            queue_enqueue(queue->name,clone,ptr);
        }
        //printf("name.(%s) dequeue.%p list.%p\n",queue->name,item,queue->list);
    }
	portable_mutex_unlock(&queue->mutex);
    return(0);
}

int32_t queue_size(queue_t *queue)
{
    int32_t count = 0;
    struct queueitem *tmp;
    lock_queue(queue);
    DL_COUNT(queue->list,tmp,count);
    portable_mutex_unlock(&queue->mutex);
	return count;
}

void iguana_initQ(queue_t *Q,char *name)
{
    struct queueitem *item,*I;
    memset(Q,0,sizeof(*Q));
    I = (struct queueitem *)calloc(1,sizeof(*I));
    strcpy(Q->name,name);
    queue_enqueue(name,Q,I);
    if ( (item= queue_dequeue(Q)) != 0 )
        free(item);
}

void komodo_userpass(char *username,char *password,FILE *fp)
{
    char *rpcuser,*rpcpassword,*str,line[8192];
    rpcuser = rpcpassword = 0;
    username[0] = password[0] = 0;
    while ( fgets(line,sizeof(line),fp) != 0 )
    {
        if ( line[0] == '#' )
            continue;
        //printf("line.(%s) %p %p\n",line,strstr(line,(char *)"rpcuser"),strstr(line,(char *)"rpcpassword"));
        if ( (str= strstr(line,(char *)"rpcuser")) != 0 )
            rpcuser = parse_conf_line(str,(char *)"rpcuser");
        else if ( (str= strstr(line,(char *)"rpcpassword")) != 0 )
            rpcpassword = parse_conf_line(str,(char *)"rpcpassword");
    }
    if ( rpcuser != 0 && rpcpassword != 0 )
    {
        strcpy(username,rpcuser);
        strcpy(password,rpcpassword);
    }
    //printf("rpcuser.(%s) rpcpassword.(%s) KMDUSERPASS.(%s) %u\n",rpcuser,rpcpassword,KMDUSERPASS,port);
    if ( rpcuser != 0 )
        free(rpcuser);
    if ( rpcpassword != 0 )
        free(rpcpassword);
}

void komodo_configfile(char *symbol,uint16_t port)
{
    static char myusername[512],mypassword[8192];
    FILE *fp; uint8_t buf2[33]; char fname[512],buf[128],username[512],password[8192]; uint32_t crc,r,r2,i;
    r = (uint32_t)time(NULL);
    r2 = OS_milliseconds();
    memcpy(buf,&r,sizeof(r));
    memcpy(&buf[sizeof(r)],&r2,sizeof(r2));
    memcpy(&buf[sizeof(r)+sizeof(r2)],symbol,strlen(symbol));
    crc = calc_crc32(0,(uint8_t *)buf,(int32_t)(sizeof(r)+sizeof(r2)+strlen(symbol)));
    OS_randombytes(buf2,sizeof(buf2));
    for (i=0; i<sizeof(buf2); i++)
        sprintf(&password[i*2],"%02x",buf2[i]);
    password[i*2] = 0;
    sprintf(buf,"%s.conf",symbol);
    BITCOIND_PORT = port;
#ifdef WIN32
    sprintf(fname,"%s\\%s",GetDataDir(false).string().c_str(),buf);
#else
    sprintf(fname,"%s/%s",GetDataDir(false).string().c_str(),buf);
#endif
    if ( (fp= fopen(fname,"rb")) == 0 )
    {
        if ( (fp= fopen(fname,"wb")) != 0 )
        {
            fprintf(fp,"rpcuser=user%u\nrpcpassword=pass%s\nrpcport=%u\nserver=1\ntxindex=1\nbind=127.0.0.1\n",crc,password,port);
            fclose(fp);
            printf("Created (%s)\n",fname);
        } else printf("Couldnt create (%s)\n",fname);
    }
    else
    {
        komodo_userpass(myusername,mypassword,fp);
        mapArgs["-rpcpassword"] = mypassword;
        mapArgs["-rpcusername"] = myusername;
        fprintf(stderr,"myusername.(%s)\n",myusername);
        fclose(fp);
    }
    strcpy(fname,GetDataDir().string().c_str());
#ifdef WIN32
    while ( fname[strlen(fname)-1] != '\\' )
        fname[strlen(fname)-1] = 0;
    strcat(fname,"komodo.conf");
#else
    while ( fname[strlen(fname)-1] != '/' )
        fname[strlen(fname)-1] = 0;
    strcat(fname,"komodo.conf");
#endif
    printf("KOMODO.(%s)\n",fname);
    if ( (fp= fopen(fname,"rb")) != 0 )
    {
        komodo_userpass(username,password,fp);
        sprintf(KMDUSERPASS,"%s:%s",username,password);
        fclose(fp);
    } else printf("couldnt open.(%s)\n",fname);
}

void komodo_args()
{
    std::string name; char *dirname; uint8_t buf[512]; int32_t i,len;
    IS_KOMODO_NOTARY = GetBoolArg("-notary", false);
    NOTARY_PUBKEY = GetArg("-pubkey", "");
    if ( strlen(NOTARY_PUBKEY.c_str()) == 66 )
        USE_EXTERNAL_PUBKEY = 1;
    fprintf(stderr,"IS_KOMODO_NOTARY %d %s\n",IS_KOMODO_NOTARY,NOTARY_PUBKEY.c_str());
    name = GetArg("-ac_name","");
    if ( name.c_str()[0] != 0 )
    {
        ASSETCHAINS_SUPPLY = GetArg("-ac_supply",10);
        strncpy(ASSETCHAINS_SYMBOL,name.c_str(),sizeof(ASSETCHAINS_SYMBOL)-1);
        len = iguana_rwnum(1,&buf[len],sizeof(ASSETCHAINS_SUPPLY),(void *)&ASSETCHAINS_SUPPLY);
        strcpy((char *)&buf[len],ASSETCHAINS_SYMBOL);
        len += strlen(ASSETCHAINS_SYMBOL);
        ASSETCHAINS_MAGIC = calc_crc32(0,buf,len);
        ASSETCHAINS_PORT = GetArg("-ac_port",8000 + (ASSETCHAINS_MAGIC % 7777));
        if ( ASSETCHAINS_SYMBOL[0] == '-' )
        {
            ASSETCHAINS_SHORTFLAG = 1;
            for (i=0; ASSETCHAINS_SYMBOL[i+1]!=0; i++)
                ASSETCHAINS_SYMBOL[i] = ASSETCHAINS_SYMBOL[i+1];
        }
        fprintf(stderr,"after args: %c%s port.%u magic.%08x supply.%u\n",ASSETCHAINS_SHORTFLAG!=0?'-':'+',ASSETCHAINS_SYMBOL,ASSETCHAINS_PORT,ASSETCHAINS_MAGIC,(int32_t)ASSETCHAINS_SUPPLY);
        while ( (dirname= (char *)GetDataDir(false).string().c_str()) == 0 || dirname[0] == 0 )
        {
            fprintf(stderr,"waiting for datadir\n");
            sleep(3);
        }
        fprintf(stderr,"Got datadir.(%s)\n",dirname);
        if ( ASSETCHAINS_SYMBOL[0] != 0 )
            komodo_configfile(ASSETCHAINS_SYMBOL,ASSETCHAINS_PORT + 1);
    } else ASSETCHAINS_PORT = 8777;
    while ( ASSETCHAIN_INIT == 0 )
    {
        sleep(1);
    }
    fprintf(stderr,"%s chain params initialized\n",ASSETCHAINS_SYMBOL);
}