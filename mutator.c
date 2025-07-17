#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed) {
    srand(Seed);

    const char *input = (const char *)Data;
    size_t input_len = Size > 0 ? Size : 1;
    if (input_len > 128) input_len = 128;

    char *output = (char *)Data;
    const char *mutation_type[] = {"repeat", "sandwich", "tree"};
    const char *type = mutation_type[rand() % 3];

    size_t out_len = 0;

    if (strcmp(type, "repeat") == 0) {
        // Repeat the original content until we hit max size
        while (out_len + input_len < MaxSize) {
            memcpy(output + out_len, input, input_len);
            out_len += input_len;
        }
    } else if (strcmp(type, "sandwich") == 0) {
        // Prefix + input + Suffix, like {{input}}
        const char *prefix = "{{";
        const char *suffix = "}}";
        size_t prefix_len = strlen(prefix);
        size_t suffix_len = strlen(suffix);

        size_t repeats = (MaxSize - input_len) / (prefix_len + suffix_len);
        for (size_t i = 0; i < repeats && out_len + prefix_len < MaxSize; i++) {
            memcpy(output + out_len, prefix, prefix_len);
            out_len += prefix_len;
        }

        if (out_len + input_len < MaxSize) {
            memcpy(output + out_len, input, input_len);
            out_len += input_len;
        }

        for (size_t i = 0; i < repeats && out_len + suffix_len < MaxSize; i++) {
            memcpy(output + out_len, suffix, suffix_len);
            out_len += suffix_len;
        }

    } else if (strcmp(type, "tree") == 0) {
        // Tree nesting like:
        // - input
        // - - input
        // ...
        const char *prefix = "- ";
        size_t prefix_len = strlen(prefix);

        while (1) {
            for (size_t i = 0; i < out_len / (input_len + prefix_len); i++) {
                if (out_len + prefix_len >= MaxSize) goto done;
                memcpy(output + out_len, prefix, prefix_len);
                out_len += prefix_len;
            }
            if (out_len + input_len >= MaxSize) break;
            memcpy(output + out_len, input, input_len);
            out_len += input_len;

            if (out_len + 1 < MaxSize) {
                output[out_len++] = '\n';
            }
        }
    }

done:
    return out_len;
}







#ifdef TESTING

#include <stdio.h>
#include <sys/time.h>
#define MAX_INPUT_SIZE 4096

int main(void) {
    uint8_t input[MAX_INPUT_SIZE];
    size_t input_size = fread(input, 1, MAX_INPUT_SIZE, stdin);

    if (input_size == 0) {
        fprintf(stderr, "No input received.\n");
        return 1;
    }

    // Buffer for mutation (overwrites input in-place)
    uint8_t buffer[MAX_INPUT_SIZE];
    // memcpy(buffer, input, input_size);
    while (1) {
    	memcpy(buffer, input, input_size);
	// unsigned int seed = 1234; // You can randomize this if desired
    	struct timeval tp;
    	gettimeofday(&tp, NULL);
    	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    	size_t mutated_size = LLVMFuzzerCustomMutator(buffer, input_size, MAX_INPUT_SIZE, ms);

    	fwrite(buffer, 1, mutated_size, stdout);
    }
    /*
    // unsigned int seed = 1234; // You can randomize this if desired
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    
    size_t mutated_size = LLVMFuzzerCustomMutator(buffer, input_size, MAX_INPUT_SIZE, ms);

    fwrite(buffer, 1, mutated_size, stdout);
    */

    return 0;
}

#endif


