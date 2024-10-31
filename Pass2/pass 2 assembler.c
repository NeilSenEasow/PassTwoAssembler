#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int find_symbol(char *label, FILE *symtab) {
    char sym[20];
    int loc;
    rewind(symtab);
    while (fscanf(symtab, "%s %d", sym, &loc) != EOF) {
        if (strcmp(sym, label) == 0) return loc;
    }
    return -1; // Not found
}

int find_opcode(char *opcode, FILE *optab) {
    char op[20];
    int code;
    rewind(optab);
    while (fscanf(optab, "%s %d", op, &code) != EOF) {
        if (strcmp(op, opcode) == 0) return code;
    }
    return -1; // Not found
}

void pass2_assembler() {
    FILE *intermediate, *optab, *symtab, *output;
    char label[20], opcode[20], operand[20];
    int loc, start, len, sym_loc, op_code;
    
    intermediate = fopen("intermediate.txt", "r");
    optab = fopen("optab.txt", "r");
    symtab = fopen("symtab.txt", "r");
    output = fopen("output_pass2.txt", "w");

    if (!intermediate || !optab || !symtab || !output) {
        printf("Error: Could not open required files.\n");
        return;
    }

    fscanf(intermediate, "%s %s %d", label, opcode, &loc);
    if (strcmp(opcode, "START") == 0) {
        start = loc;
        fprintf(output, "H^%s^%06X^----\n", label, start); // Header record placeholder
        fscanf(intermediate, "%d %s %s %s", &loc, label, opcode, operand);
    }

    fprintf(output, "T^%06X^", start); // Text record start address
    int text_length = 0;
    char text_record[80] = "";

    while (strcmp(opcode, "END") != 0) {
        op_code = find_opcode(opcode, optab);
        if (op_code != -1) { // Found in OPTAB
            sym_loc = find_symbol(operand, symtab);
            if (sym_loc != -1) {
                char code[10];
                sprintf(code, "%02X%04X", op_code, sym_loc);
                strcat(text_record, "^");
                strcat(text_record, code);
                text_length += strlen(code) / 2;
            }
        } else if (strcmp(opcode, "WORD") == 0) {
            int value = atoi(operand);
            char code[10];
            sprintf(code, "%06X", value);
            strcat(text_record, "^");
            strcat(text_record, code);
            text_length += strlen(code) / 2;
        } else if (strcmp(opcode, "BYTE") == 0) {
            int length = strlen(operand) - 3; // Removing C' and '
            char code[10] = "";
            for (int i = 2; i < length + 2; i++) {
                char hex[3];
                sprintf(hex, "%02X", operand[i]);
                strcat(code, hex);
            }
            strcat(text_record, "^");
            strcat(text_record, code);
            text_length += strlen(code) / 2;
        } else if (strcmp(opcode, "RESW") == 0 || strcmp(opcode, "RESB") == 0) {
            if (text_length > 0) { // Write text record if there is content
                fprintf(output, "%02X%s\n", text_length, text_record);
                text_length = 0;
                strcpy(text_record, "");
            }
            if (strcmp(opcode, "RESW") == 0) loc += 3 * atoi(operand);
            else loc += atoi(operand);
        }

        fscanf(intermediate, "%d %s %s %s", &loc, label, opcode, operand);
    }

    if (text_length > 0) { // Write remaining text record
        fprintf(output, "%02X%s\n", text_length, text_record);
    }

    fprintf(output, "E^%06X\n", start); // End record

    fclose(intermediate);
    fclose(optab);
    fclose(symtab);
    fclose(output);

    printf("Pass 2 complete. Output written to output_pass2.txt.\n");
}

int main() {
    pass2_assembler();
    return 0;
}
