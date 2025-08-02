#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_BYTES 100  // first bytes of input are config: 3+3+2
#define MAX_PREFIX_LEN 20
#define MAX_SUFFIX_LEN 20
#define MAX_NEST_LEN 20

size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed) {
    srand(Seed);

    if (Size < CONFIG_BYTES + 1) return Size;

    const char *prefix = (const char *)Data + rand() % Size;
    const char *suffix = (const char *)(Data + rand() % Size);
    const char *nesting = (const char *)(Data + rand() % Size);

    // const char *input = (const char *)(Data + CONFIG_BYTES);

    uint8_t input_buf[128];
    size_t input_len = Size - CONFIG_BYTES;
    if (input_len > sizeof(input_buf)) input_len = sizeof(input_buf);
    memcpy(input_buf, Data + CONFIG_BYTES, input_len);
    const char *input = (const char *)input_buf;

    // size_t input_len = Size - CONFIG_BYTES;
    if (input_len > 128) input_len = 128;

    char *output = (char *)Data;
    const char *mutation_type[] = {"repeat", "sandwich", "tree"};
    const char *type = mutation_type[rand() % 3];

    size_t out_len = 0;

    if (strcmp(type, "repeat") == 0) {
        while (out_len + input_len < MaxSize) {
            memcpy(output + out_len, input, input_len);
            out_len += input_len;
        }

    } else if (strcmp(type, "sandwich") == 0) {
        size_t prefix_len = rand() % MAX_PREFIX_LEN + 1;
        size_t suffix_len = rand() % MAX_SUFFIX_LEN + 1;
        char prefix_copy[prefix_len];
        memcpy(prefix_copy, prefix, prefix_len);
        char suffix_copy[suffix_len];
        memcpy(suffix_copy, suffix, suffix_len);

        size_t repeats = (MaxSize - input_len) / (prefix_len + suffix_len);
        for (size_t i = 0; i < repeats && out_len + prefix_len < MaxSize; i++) {
            memcpy(output + out_len, prefix_copy, prefix_len);
            out_len += prefix_len;
        }

        if (out_len + input_len < MaxSize) {
            memcpy(output + out_len, input, input_len);
            out_len += input_len;
        }

        for (size_t i = 0; i < repeats && out_len + suffix_len < MaxSize; i++) {
            memcpy(output + out_len, suffix_copy, suffix_len);
            out_len += suffix_len;
        }

    } else if (strcmp(type, "tree") == 0) {
        size_t nest_len = rand() % MAX_NEST_LEN + 1;

        char nest_copy[nest_len];
        memcpy(nest_copy, nesting, nest_len);

        while (1) {
            for (size_t i = 0; i < out_len / (input_len + nest_len); i++) {
                if (out_len + nest_len >= MaxSize) goto done;
                memcpy(output + out_len, nest_copy, nest_len);
                out_len += nest_len;
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

    if (input_size <= CONFIG_BYTES) {
        fprintf(stderr, "Need at least %d bytes for config + payload.\n", CONFIG_BYTES + 1);
        return 1;
    }

    uint8_t buffer[MAX_INPUT_SIZE];

    while (1) {
        memcpy(buffer, input, input_size);

        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        printf("mutating...\n");
        size_t mutated_size = LLVMFuzzerCustomMutator(buffer, input_size, MAX_INPUT_SIZE, ms);
        fwrite(buffer, 1, mutated_size, stdout);
    }

    return 0;
}
#endif