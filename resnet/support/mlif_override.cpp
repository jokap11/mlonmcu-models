#include "quantize.h"
#include "printing.h"
#include "exit.h"
#include <cstring>


extern "C" {
int mlif_process_input(const void *, size_t, void *, size_t);
int mlif_process_output(void *, size_t, const void *, size_t);
}


int mlif_process_input(const void *in_data, size_t in_size, void *model_input_ptr, size_t model_input_sz)
{
    if (in_size != model_input_sz)
    {
        DBGPRINTF("MLIF: Wrong input size!\n");
        return EXIT_MLIF_INVALID_SIZE;
    }

    memcpy(model_input_ptr, in_data, in_size);
    return 0;
}

int mlif_process_output(void *model_output_ptr, size_t model_output_sz, const void *expected_out_data, size_t expected_out_size)
{
    static unsigned addr;
    if (model_output_sz != 10 && model_output_sz != 1)
    {
        DBGPRINTF("MLIF: Wrong output size!\n");
        return EXIT_MLIF_INVALID_SIZE;
    }else if (model_output_sz == 10){

        addr = 0x800100; //set once
        float out_scale = 0.00390625;
        int out_zeropt = -128;

        float results[10];
        int prediction = 0;
        for (int i = 0; i < 10; i++)
        {
            results[i] = DequantizeInt8ToFloat(((int8_t*)model_output_ptr)[i], out_scale, out_zeropt);
            DBGPRINTF("Category %i: %.9g\n", i, results[i]);

            if (results[i] >= results[prediction])
            {
                prediction = i;
            }
        }
        unsigned cat_addr = 0x800000;
        *((volatile int32_t *)cat_addr) = prediction;
        DBGPRINTF("Predicted category: %i\n", prediction);

        // for (int i = 0; i < 10; i++)
        // {
        //     float expected = ((float*)expected_out_data)[i];
        //     if (results[i] != expected)
        //     {
        //         DBGPRINTF("MLIF: Wrong output in category %i! Expected %.9g\n", i, expected);
        //         return EXIT_MLIF_MISSMATCH;
        //     }
        // }
    }

    DBGPRINTF("Printing everything after %x :\n", addr);
    for (size_t i = 0; i < model_output_sz; i++)
    {
               int32_t written_byte = 0xDEADBEEF; //Check if actually read
        written_byte = *((int8_t*)model_output_ptr + i);
        *((volatile int32_t *)addr) = written_byte;  //Little Endian so 44 is at 0x800100 = 0x11223344
        addr = addr +4;
        DBGPRINTF("Write hex number %#04hhx to memory \n", written_byte);
    }

    return 0;
}
