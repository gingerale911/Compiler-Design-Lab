#ifndef HEADER_PREPROCESSOR_H
#define HEADER_PREPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int preprocessFile(const char *inputFile, const char *outputFile) {

    FILE *fa, *fb;
    char line[500];

    fa = fopen(inputFile, "r");
    if (fa == NULL) {
        printf("Cannot open input file\n");
        return 0;
    }

    fb = fopen(outputFile, "w");
    if (fb == NULL) {
        printf("Cannot open output file\n");
        fclose(fa);
        return 0;
    }

    printf("Preprocessing Started...\n"); 
    while (fgets(line, sizeof(line), fa)) {
 
        if (strstr(line, "main(") != NULL) {
            fprintf(fb, "%s", line); 
            while (fgets(line, sizeof(line), fa)) {
                fprintf(fb, "%s", line);
            }
            break;
        } 
        if (line[0] == '#') {

            char buffer[100];
            int i = 1, j = 0; 
            while (isspace(line[i]))
                i++; 
            while (isalpha(line[i])) {
                buffer[j++] = line[i++];
            }
            buffer[j] = '\0';  
            if (strcmp(buffer, "include") == 0 ||
                strcmp(buffer, "define") == 0) { 
                continue;
            }
        } 
        fprintf(fb, "%s", line);
    }

    fclose(fa);
    fclose(fb);

    printf("Preprocessing Completed Successfully\n");
    return 1;
}

#endif
