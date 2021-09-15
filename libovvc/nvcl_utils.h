#ifndef OVVC_GOLOMB_H
#define OVVC_GOLOMB_H

#include <stdint.h>

#include "nvcl.h"
#include "ovutils.h"

enum DecReturn {
    OV_INVALID_DATA = -1,
    OV_ENOMEM = -2,
};

struct OVNVCLReader
{
    const uint8_t *bytestream_start;
    const uint8_t *bytestream_end;
    const uint8_t *bytestream;
    uint64_t cache;
    int nb_cached_bits;
};

static inline uint64_t read_bigendian_64(const uint8_t *bytestream);

static inline uint64_t read_bigendian_32(const uint8_t *bytestream);

static inline void fill_cache64(OVNVCLReader *rdr);

static inline void fill_cache32(OVNVCLReader *rdr);

static inline uint32_t fetch_bits(OVNVCLReader *rdr, int n);

static inline uint8_t nvcl_read_flag(OVNVCLReader *rdr);

static inline uint32_t nvcl_read_bits(OVNVCLReader *rdr, int n);

/* TODO check if prefix size can exceed 31*/
/* WARNING do not use if prefix size can exceed 31 */
static inline uint32_t nvcl_read_u_expgolomb(OVNVCLReader *rdr);

/* TODO check if prefix size can exceed 31*/
/* WARNING do not use if prefix size can exceed 31 */
static inline int32_t nvcl_read_s_expgolomb(OVNVCLReader *rdr);

/*FIXME we could check alignement bits are zero or ones for
  strict compliance check*/
static inline void nvcl_align(OVNVCLReader *rdr);

static inline void nvcl_skip_flag(OVNVCLReader *rdr);

/* WARNING does not support n > 64 */
static inline void nvcl_skip_bits(OVNVCLReader *rdr, int n);

#if 0
static inline int nvclctx_num_bits_left(OVNVCLReader *rdr);
#endif

static inline void nvcl_skip_bits(OVNVCLReader *rdr, int n)
{
    if (n > rdr->nb_cached_bits) {
        fill_cache32(rdr);
        if (rdr->nb_cached_bits < 32)
            rdr->nb_cached_bits = n;
    }

    rdr->cache <<= n;
    rdr->nb_cached_bits -= n;
}

static inline uint64_t
read_bigendian_64(const uint8_t *bytestream)
{

    uint64_t cache_val;
    cache_val = bytestream[0];
    cache_val <<= 8;
    cache_val |= bytestream[1];
    cache_val <<= 8;
    cache_val |= bytestream[2];
    cache_val <<= 8;
    cache_val |= bytestream[3];
    cache_val <<= 8;
    cache_val |= bytestream[4];
    cache_val <<= 8;
    cache_val |= bytestream[5];
    cache_val <<= 8;
    cache_val |= bytestream[6];
    cache_val <<= 8;
    cache_val |= bytestream[7];
    return cache_val;
}

static inline uint64_t
read_bigendian_32(const uint8_t *bytestream)
{
    uint64_t cache_val;
    cache_val = bytestream[0];
    cache_val <<= 8;
    cache_val |= bytestream[1];
    cache_val <<= 8;
    cache_val |= bytestream[2];
    cache_val <<= 8;
    cache_val |= bytestream[3];
    return cache_val;
}

static inline void
fill_cache64(OVNVCLReader *rdr)
{
    if (rdr->bytestream >= rdr->bytestream_end)
        return;

    rdr->cache = read_bigendian_64(rdr->bytestream);

    rdr->bytestream      += 8;
    rdr->nb_cached_bits   = 64;
}

static inline void
fill_cache32(OVNVCLReader *rdr)
{
    uint64_t cache_val;
    if (rdr->bytestream >= rdr->bytestream_end)
        return;

    cache_val = read_bigendian_32(rdr->bytestream);
    rdr->cache |= cache_val << (32 - rdr->nb_cached_bits);

    rdr->bytestream     += 4;
    rdr->nb_cached_bits += 32;
}

/* WARNING does not support n > 64 */

static inline void
skip_bits(OVNVCLReader *rdr, int n)
{
    if (n < rdr->nb_cached_bits) {
        rdr->cache <<= n;
        rdr->nb_cached_bits -= n;
    } else {
        n -= rdr->nb_cached_bits;
        fill_cache64(rdr);

        if (n) {
            rdr->cache <<= n;
            rdr->nb_cached_bits -= n;
        }
    }
}

static inline uint32_t
fetch_bits(OVNVCLReader *rdr, int n)
{
    if (n > rdr->nb_cached_bits)
        fill_cache32(rdr);

    return (rdr->cache >> (64 - n));
}

static inline void
nvcl_skip_flag(OVNVCLReader *rdr)
{
    if (!rdr->nb_cached_bits) {
        fill_cache64(rdr);
    }

    rdr->cache <<= 1;
    rdr->nb_cached_bits--;

    /* FIXME refill or not */
    if (rdr->nb_cached_bits) {
        fill_cache64(rdr);
    }
}

static inline void
nvcl_align(OVNVCLReader *rdr)
{
    int n = ((int32_t)rdr->nb_cached_bits) & 0x7;
    if (n)
        nvcl_skip_bits(rdr, n);
}

static inline unsigned int
nvcl_read_bits(OVNVCLReader *rdr, int n)
{
    register int tmp;

    if (n > rdr->nb_cached_bits) {
        fill_cache32(rdr);
        if (rdr->nb_cached_bits < 32)
            rdr->nb_cached_bits = n;
    }

    tmp = rdr->cache >> (64 - n);
    rdr->cache <<= n;
    rdr->nb_cached_bits -= n;
    return tmp;
}

static inline uint8_t
nvcl_read_flag(OVNVCLReader *rdr)
{
    uint32_t val;
    if (!rdr->nb_cached_bits)
        fill_cache64(rdr);

    val = rdr->cache >> (64 - 1);

    rdr->nb_cached_bits--;
    rdr->cache <<= 1;

    return val;
}


/* TODO check if prefix size can exceed 31*/
/* WARNING do not use if prefix size can exceed 31 */
static inline uint32_t
nvcl_read_u_expgolomb(OVNVCLReader *rdr)
{
    uint32_t tmp = fetch_bits(rdr, 32);
    uint32_t prf_length = ov_clz(tmp | 0x1);

    /* Mark prefix as read */
    nvcl_skip_bits(rdr, prf_length);

    /* Read suffix */
    return nvcl_read_bits(rdr, (prf_length + 1)) - 1;
}

/* TODO check if prefix size can exceed 31*/
/* WARNING do not use if prefix size can exceed 31 */
static inline int32_t
nvcl_read_s_expgolomb(OVNVCLReader *rdr)
{
    uint32_t tmp = fetch_bits(rdr, 32);
    uint32_t prf_length = ov_clz(tmp | 0x1);
    int32_t sign;
    int32_t val;

    /* Mark prefix as read */
    nvcl_skip_bits(rdr, prf_length);

    /* Read suffix */
    val  = nvcl_read_bits(rdr, (prf_length + 1)) - 1;

    sign = (val & 0x1) - 1;

    return ((val >> 1) ^ sign) + 1;
}

#endif
