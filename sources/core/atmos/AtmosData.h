#pragma once

const int SAMPLE_SIZE = 5;

// Bc == Binomial coefficient

namespace Bc_5_1
{
    const int SAMPLE_1[SAMPLE_SIZE] = {1, 0, 0, 0, 0};
    const int SAMPLE_2[SAMPLE_SIZE] = {0, 1, 0, 0, 0};
    const int SAMPLE_3[SAMPLE_SIZE] = {0, 0, 1, 0, 0};
    const int SAMPLE_4[SAMPLE_SIZE] = {0, 0, 0, 1, 0};
    const int SAMPLE_5[SAMPLE_SIZE] = {0, 0, 0, 0, 1};

    const int SIZE = 5;

    const int* const SAMPLES[SIZE] =
        { SAMPLE_1,
          SAMPLE_2,
          SAMPLE_3,
          SAMPLE_4,
          SAMPLE_5 };
}

namespace Bc_5_2
{
    const int SAMPLE_1 [SAMPLE_SIZE] = {1, 1, 0, 0, 0};
    const int SAMPLE_2 [SAMPLE_SIZE] = {1, 0, 1, 0, 0};
    const int SAMPLE_3 [SAMPLE_SIZE] = {1, 0, 0, 1, 0};
    const int SAMPLE_4 [SAMPLE_SIZE] = {1, 0, 0, 0, 1};
    const int SAMPLE_5 [SAMPLE_SIZE] = {0, 1, 1, 0, 0};
    const int SAMPLE_6 [SAMPLE_SIZE] = {0, 1, 0, 1, 0};
    const int SAMPLE_7 [SAMPLE_SIZE] = {0, 1, 0, 0, 1};
    const int SAMPLE_8 [SAMPLE_SIZE] = {0, 0, 1, 1, 0};
    const int SAMPLE_9 [SAMPLE_SIZE] = {0, 0, 1, 0, 1};
    const int SAMPLE_10[SAMPLE_SIZE] = {0, 0, 0, 1, 1};

    const int SIZE = 10;

    const int* const SAMPLES[SIZE] =
        { SAMPLE_1,
          SAMPLE_2,
          SAMPLE_3,
          SAMPLE_4,
          SAMPLE_5,
          SAMPLE_6,
          SAMPLE_7,
          SAMPLE_8,
          SAMPLE_9,
          SAMPLE_10 };
}

namespace Bc_5_3
{
    const int SAMPLE_1 [SAMPLE_SIZE] = {0, 0, 1, 1, 1};
    const int SAMPLE_2 [SAMPLE_SIZE] = {0, 1, 0, 1, 1};
    const int SAMPLE_3 [SAMPLE_SIZE] = {0, 1, 1, 0, 1};
    const int SAMPLE_4 [SAMPLE_SIZE] = {0, 1, 1, 1, 0};
    const int SAMPLE_5 [SAMPLE_SIZE] = {1, 0, 0, 1, 1};
    const int SAMPLE_6 [SAMPLE_SIZE] = {1, 0, 1, 0, 1};
    const int SAMPLE_7 [SAMPLE_SIZE] = {1, 0, 1, 1, 0};
    const int SAMPLE_8 [SAMPLE_SIZE] = {1, 1, 0, 0, 1};
    const int SAMPLE_9 [SAMPLE_SIZE] = {1, 1, 0, 1, 0};
    const int SAMPLE_10[SAMPLE_SIZE] = {1, 1, 1, 0, 0};

    const int SIZE = 10;

    const int* const SAMPLES[SIZE] =
        { SAMPLE_1,
          SAMPLE_2,
          SAMPLE_3,
          SAMPLE_4,
          SAMPLE_5,
          SAMPLE_6,
          SAMPLE_7,
          SAMPLE_8,
          SAMPLE_9,
          SAMPLE_10 };
}

namespace Bc_5_4
{
    const int SAMPLE_1[SAMPLE_SIZE] = {0, 1, 1, 1, 1};
    const int SAMPLE_2[SAMPLE_SIZE] = {1, 0, 1, 1, 1};
    const int SAMPLE_3[SAMPLE_SIZE] = {1, 1, 0, 1, 1};
    const int SAMPLE_4[SAMPLE_SIZE] = {1, 1, 1, 0, 1};
    const int SAMPLE_5[SAMPLE_SIZE] = {1, 1, 1, 1, 0};

    const int SIZE = 5;

    const int* const SAMPLES[SIZE] =
        { SAMPLE_1,
          SAMPLE_2,
          SAMPLE_3,
          SAMPLE_4,
          SAMPLE_5 };
}

const int BC_TYPES_SIZE = 4;
const int* const* const BC_TYPES[BC_TYPES_SIZE] =
    { Bc_5_1::SAMPLES,
      Bc_5_2::SAMPLES,
      Bc_5_3::SAMPLES,
      Bc_5_4::SAMPLES };

const int BC_TYPES_SIZES[BC_TYPES_SIZE] =
    { Bc_5_1::SIZE,
      Bc_5_2::SIZE,
      Bc_5_3::SIZE,
      Bc_5_4::SIZE };
