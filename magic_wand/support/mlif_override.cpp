#include "quantize.h"
#include "printing.h"
#include "exit.h"
#include <cstring>


extern "C" {
void mlif_process_input(const void *, size_t, void *, size_t);
void mlif_process_output(void *, size_t, const void *, size_t);
}


void mlif_process_input(const void *in_data, size_t in_size, void *model_input_ptr, size_t model_input_sz)
{
    if (in_size != model_input_sz)
    {
        DBGPRINTF("MLIF: Wrong input size!\n");
        exit(EXIT_MLIF_INVALID_SIZE);
    }

    memcpy(model_input_ptr, in_data, in_size);
}

void mlif_process_output(void *model_output_ptr, size_t model_output_sz, const void *expected_out_data, size_t expected_out_size)
{
    const char* labels[] = {"kWingGesture", "kRingGesture", "kSlopeGesture", "kNoGesture"};
    if (model_output_sz != 16)
    {
        DBGPRINTF("MLIF: Wrong output size!\n");
        exit(EXIT_MLIF_INVALID_SIZE);
    }

    float results[4];
    int prediction = 0;
    for (int i = 0; i < 4; i++)
    {
        results[i] = ((float*)model_output_ptr)[i];
        DBGPRINTF("Category %i [%s]: %.9g\n", i, labels[i], results[i]);

        if (results[i] >= results[prediction])
        {
            prediction = i;
        }
    }
    DBGPRINTF("Predicted category: %i [%s]\n", prediction, labels[prediction]);


    for (int i = 0; i < 4; i++)
    {
        float expected = ((float*)expected_out_data)[i];
        if (results[i] != expected)
        {
            DBGPRINTF("MLIF: Wrong output in category %i! Expected %.9g\n", i, expected);
            exit(EXIT_MLIF_MISSMATCH);
        }
    }
}
