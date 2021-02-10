#include "data_rcn_angular.h"
#include <stdint.h>

const int angle_table[32] = { 0,  1,   2,   3,   4,   6,   8,   10,
                                     12, 14,  16,  18,  20,  23,  26,  29,
                                     32, 35,  39,  45,  51,  57,  64,  73,
                                     86, 102, 128, 171, 256, 341, 512, 1024 };

const int inverse_angle_table[32] = {
        0,   16384, 8192, 5461, 4096, 2731, 2048, 1638, 1365, 1170, 1024,
        910, 819,   712,  630,  565,  512,  468,  420,  364,  321,  287,
        256, 224,   191,  161,  128,  96,   64,   48,   32,   16
}; // (512 * 32) / Angle

const uint8_t intra_filter /*[2]*/[8] = {
        //    { // Luma
        24, //   1xn
        24, //   2xn
        24, //   4xn
        14, //   8xn
        2,  //  16xn
        0,  //  32xn
        0,  //  64xn
        0,  // 128xn
            //    },//Chroma is only used in 444
            //    { // Chroma
            //      40, //   1xn
            //      40, //   2xn
            //      40, //   4xn
            //      28, //   8xn
            //      4,  //  16xn
            //      0,  //  32xn
            //      40, //  64xn
            //      0,  // 128xn
            //    }
};
