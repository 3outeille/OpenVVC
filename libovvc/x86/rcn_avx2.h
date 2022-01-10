#ifndef RCN_AVX2_H
#define RCN_AVX2_H
#include "rcn_structures.h"

void rcn_init_alf_functions_avx2(struct RCNFunctions *rcn_func);
void rcn_init_sao_functions_avx2(struct RCNFunctions *rcn_func);
void rcn_init_ict_functions_avx2(struct RCNFunctions *rcn_func, uint8_t type);

#endif//RCN_AVX2_H
