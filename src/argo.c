#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

static int additionalIndent = 0;
static int nameOption = -1;
char intToHex(int x);
/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
int argo_read_object(FILE *f) {
    int returnValue = argo_next_value;
    ARGO_VALUE *sentinel = (argo_value_storage + argo_next_value + 1);
    (*(argo_value_storage + argo_next_value)).content.object.member_list = sentinel;
    argo_next_value++;
    ARGO_VALUE *prvNode;
    ARGO_VALUE *tmpNode;
    int index = 0;
    char c = fgetc(f);
    while (c != '}') {
        argo_next_value++;
        nameOption = 0;
        argo_read_value(f);
        nameOption = -1;
        if (fgetc(f) != ':')
            return -1;
        tmpNode = argo_read_value(f);
        if (index == 0) {
            (*sentinel).next = tmpNode;
            (*tmpNode).prev = sentinel;
            (*tmpNode).next = sentinel;
            index = 1;
        } else {
            (*tmpNode).prev = prvNode;
            (*prvNode).next = tmpNode;
        }
        c = fgetc(f);
        prvNode = tmpNode;
        if (c == '}')
            break;
        if (c != ',')
            return -1;

    }
    argo_next_value++;
    (*tmpNode).next = sentinel;
    (*sentinel).prev = tmpNode;
    return returnValue;
}
int argo_read_array(FILE *f) {
    int returnValue = argo_next_value;
    ARGO_VALUE *sentinel = (argo_value_storage + argo_next_value + 1);
    (*(argo_value_storage + argo_next_value)).content.array.element_list = sentinel;
    argo_next_value++;
    ARGO_VALUE *prvNode;
    ARGO_VALUE *tmpNode;
    int index = 0;
    char c = fgetc(f);
    while (c != ']') {
        argo_next_value++;
        tmpNode = argo_read_value(f);
        if (index == 0) {
            (*sentinel).next = tmpNode;
            (*tmpNode).prev = sentinel;
            (*tmpNode).next = sentinel;
            index = 1;
        } else {
            (*tmpNode).prev = prvNode;
            (*prvNode).next = tmpNode;
        }
        c = fgetc(f);
        prvNode = tmpNode;
        if (c == ']')
            break;
        if (c != ',')
            return -1;

    }
    argo_next_value++;
    (*tmpNode).next = sentinel;
    (*sentinel).prev = tmpNode;
    return returnValue;
}
ARGO_VALUE *argo_read_value(FILE *f) {
    ARGO_VALUE *v;
    char s = fgetc(f);
    if (s == '{') {
        v = (argo_value_storage + argo_next_value);
        (*v).type = ARGO_OBJECT_TYPE;
        ungetc(s,f);
        int x = argo_read_object(f);
        if (x != -1) {
            return (argo_value_storage + x);
        }
        return NULL;
    }
    else if (s == '[') {
        (v) = (argo_value_storage + argo_next_value);
        (*v).type = ARGO_ARRAY_TYPE;
        ungetc(s, f);
        int x = (argo_read_array(f));
        if (x != -1) {
            return argo_value_storage + x;
        }
        return NULL;
    }
    else if (s == '\"') {
        (v) = (argo_value_storage + argo_next_value);
        if ((*v).type == 0)
            (*v).type = ARGO_STRING_TYPE;
        if (nameOption == -1) {
            int x = argo_read_string(&(*(argo_value_storage + argo_next_value)).content.string, f);
            if (x != -1) {
                if (argo_next_value == 0) {
                    argo_next_value++;
                }
                return (argo_value_storage + x);
            }
        } else {
            argo_read_string(&(*(argo_value_storage + argo_next_value)).name, f);
        }
        return NULL;
    }
    else if ((s >= '0' && s <= '9') || s == '-') {
        ungetc(s, f);
        (v) = (argo_value_storage + argo_next_value);
        if ((*v).type == 0)
            (*v).type = ARGO_NUMBER_TYPE;
        int x = argo_read_number(&(*(argo_value_storage + argo_next_value)).content.number, f);
        if (x != -1) {
            if (argo_next_value == 0) {
                argo_next_value++;
            }
            return (argo_value_storage + x);
        }
        return NULL;
    }


    return NULL;
}

int argo_append_special(char c) {
    if (c == '\"' || c == '\\') {
        return c;
    } else if (c==98) {
        return 8;
    } else if(c==114){
        return 13;
    } else if (c==110) {
        return 10;
    } else if(c==102) {
        return  12;
    } else if(c==116)
        return 9;
    return -1;
}
int readHex(int x) {
    if (x >= 48 && x<=57)
        return x - 48;
    else if ((x >= 97 && x <= 102))
        return x - 87;
     else if (x >= 65 && x <= 70)
        return x - 55;
    else
        return -1;
}
/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f) {
    char c = fgetc(f);
    while (c != '\"'){
        int index = 0;
        if (c == '\\') {
            c = fgetc(f);
            if (argo_append_special(c) !=  -1) {
                argo_append_char(s, argo_append_special(c));
            }else if (c == 'u') {
                char fplace, splace, tplace, foplace;
                printf("\n");
                while (index < 4) {
                    c = fgetc(f);
                    c = readHex(c);
                    printf("%x", c);
                    if (c == -1)
                        return -1;
                    else if (index == 0)
                        fplace = c;
                    else if (index == 1)
                        splace = c;
                    else if (index == 2)
                        tplace = c;
                    else if (index == 3)
                        foplace = c;
                    index++;
                }
                c = (4096 * fplace) +  (256 * splace) + (tplace * 16) + foplace;
                printf("\n");
                argo_append_char(s,c);
            } else
                return -1;
        } else if (c > 0x1F && c < 0xFF){
            argo_append_char(s,c);
        } else
            return -1;
        c = fgetc(f);
    }
    return argo_next_value;
}

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
//  */
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    int negFlag = -1;
    negFlag = negFlag - 0;
    int enegFlag = -1;
    int eposFlag = -1;
    eposFlag = eposFlag -0;
    int eFlag = -1;
    int decFlag = -1;
    int firstDigFlag = 0;
    int firstEDigFlag = -1;
    int leadingZeroFlag = -1;
    int firstDecDigit = -1;
    int validInt = 0;
    validInt = validInt - 0;
    long int sum = 0;
    long int decimalPlaces = 0;
    long int wholeNumber = 0;
    int exponent = 0;
    char prev = 'q';
    char c = fgetc(f);
    while (1) {
        if (c == '-' && (prev != 'q' && prev !='e' && prev != 'E')) {
            break;
        } else if (c == '-' && !(prev != 'q' && prev !='e' && prev != 'E')) {
            if (prev == 'q'){
                negFlag = 0;
            } else
                enegFlag = 0;
        }
        if (c == '+' && (prev != 'e' && prev!= 'E'))
            break;
        else if (c == '+')
            eposFlag = 0;

        if (prev == 'q' && c == '0')
            leadingZeroFlag = 0;
        if (c >= 48 && c <= 57) {
            if (firstDigFlag == -1 && decFlag == -1 && leadingZeroFlag == 0)
                return -1;
            if (eFlag != 0) {
                sum= sum * 10;
                sum = sum + (c - 48);
            }
            if (c != '0')
                firstDigFlag = 0;
            if (decFlag ==-1 && eFlag == -1)
                wholeNumber++;
            if (decFlag == 0 && eFlag == -1) {
                decimalPlaces++;
                firstDecDigit = 0;
            } else if (eFlag == 0) {
                firstEDigFlag = 0;
                exponent = exponent * 10;
                exponent = exponent + (c - 48);
            }

        }
        if (c == '.' && decFlag == 0 && firstDecDigit == -1)
            return -1;
        else  if (c == '.' && decFlag == 0 && firstDecDigit == 0)
            break;
        else if (c == '.') {
            decFlag = 0;
            validInt = -1;
        }

        if (c == 'e' || c == 'E') {
            if (eFlag == -1) {
                eFlag = 0;
            } else
                break;
        }
        if (c != '-')
            prev = c;
        if (!(c >= 48 && c <= 57) && (c != '.') && (c != '-') && (c != '+') && (c != 'e') && (c != 'E'))
            break;
        c = fgetc(f);
    }
    ungetc(c, f);
    if (decFlag == 0 && firstDecDigit == -1)
        return -1;
    if (eFlag == 0 && firstEDigFlag == -1)
        return -1;
    if (firstDigFlag == -1)
        return -1;

    if (decFlag == -1 && eFlag == -1) {
        if (enegFlag == -1) {
            if (negFlag == 0)
                sum = sum * -1;
            (*n).valid_int = 1;
            (*n).valid_float = 1;
            (*n).valid_string = 1;
            (*n).int_value = sum;
            (*n).float_value = sum;
        }
    } else {
        if (enegFlag == -1) {
            double float_v = sum;
            while (decimalPlaces > 0) {
                float_v = float_v / 10;
                decimalPlaces--;
            }
            while (exponent > 0) {
                float_v = float_v * 10;
                exponent--;
            }
            if (negFlag == 0)
                float_v = float_v * -1;
            (*n).valid_int = 0;
            (*n).valid_float = 1;
            (*n).valid_string = 1;
            (*n).float_value = float_v;
        } else {
            double float_v = sum;
            while (decimalPlaces > 0) {
                float_v = float_v / 10;
                decimalPlaces--;
            }
            while (exponent > 0) {
                float_v = float_v / 10;
                exponent--;
            }
            if (negFlag == 0)
                float_v = float_v * -1;
            (*n).valid_int = 0;
            (*n).valid_float = 1;
            (*n).valid_string = 1;
            (*n).float_value = float_v;
        }
    }
    return argo_next_value;
}

int argo_write_array(ARGO_VALUE *v, FILE *f) {
    fputc('[', f);
    additionalIndent+= (0x0000000F & global_options);
    if ((global_options & 0x30000000) == 0x30000000)
        fputc('\n', f);
    ARGO_ARRAY array = (*v).content.array;
    ARGO_VALUE sentinel = *array.element_list;
    ARGO_VALUE *tmpNode = (sentinel).next;
    do {
        if ((global_options & 0x30000000) == 0x30000000 && (*tmpNode).next != sentinel.next) {
            int x = 0;
            x = x + (additionalIndent);
            while (x > 0) {
                fputc(' ', f);
                x--;
            }
        }
        argo_write_value((tmpNode), f);
        tmpNode = (*tmpNode).next;
        if (tmpNode != sentinel.next && (*tmpNode).next != sentinel.next){
            fputc(',', f);
            if ((global_options & 0x30000000) == 0x30000000)
        fputc('\n', f);
        }
    } while(tmpNode != sentinel.next);
    if ((global_options & 0x30000000) == 0x30000000)
        fputc('\n', f);
    additionalIndent-= (0x0000000F & global_options);
    if (additionalIndent > 0) {
        int tmpIndent = additionalIndent;
        while (tmpIndent > 0) {
            fputc(' ', f);
            tmpIndent--;
        }
    }
    fputc(']', f);

    return 1;
}

int argo_write_object(ARGO_VALUE *v, FILE *f) {
    fputc('{', f);
    additionalIndent+= (0x0000000F & global_options);
    if ((global_options & 0x30000000) == 0x30000000)
        fputc('\n', f);
    ARGO_OBJECT object = (*v).content.object;
    ARGO_VALUE sentinel = *object.member_list;;
    ARGO_VALUE *tmpNode = (sentinel).next;
    do {
        if ((global_options & 0x30000000) == 0x30000000 && (*tmpNode).next != sentinel.next) {
            int x =0;
            x = x + (additionalIndent);
            while (x > 0) {
                fputc(' ', f);
                x--;
            }
        }
        if ((*tmpNode).name.length != 0) {
            argo_write_string(&((*tmpNode).name), f);
            fputs(":", f );
            if ((global_options & 0x30000000) == 0x30000000)
                fputc(' ', f);
            argo_write_value(tmpNode, f);
            if ((*(*tmpNode).next).name.length != 0)
                fputc(',', f);
        }
        if ((global_options & 0x30000000) == 0x30000000 && ((*tmpNode).name.length != 0))
            fputc('\n', f);
        tmpNode = (*tmpNode).next;
    } while(tmpNode != sentinel.next);
    additionalIndent-= (0x0000000F & global_options);
    if (additionalIndent > 0) {
        int tmpIndent = additionalIndent;
        while (tmpIndent > 0) {
            fputc(' ', f);
            tmpIndent--;
        }
    }
    fputc('}', f);
    if (additionalIndent == 0)
        fputc('\n', f);
    return 0;
}
int argo_write_basic(int v, FILE *f) {
    if (v == 0)
        fputs("null", f);
    else if (v == 2)
        fputs("false", f);
    else if (v == 1)
        fputs("true", f);
    return 0;
}
/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */

int argo_write_value(ARGO_VALUE *v, FILE *f) {
    if ((*v).type == ARGO_NO_TYPE) {

    } else if ((*v).type == ARGO_BASIC_TYPE) {
        argo_write_basic((*v).content.basic, f);
    } else if ((*v).type == ARGO_NUMBER_TYPE) {
        argo_write_number(&(*v).content.number, f);
    } else if ((*v).type == ARGO_STRING_TYPE) {
        argo_write_string(&(*v).content.string, f);
    } else if ((*v).type == ARGO_OBJECT_TYPE) {
        argo_write_object(v, f);
    } else if ((*v).type == ARGO_ARRAY_TYPE) {
        argo_write_array(v, f);
    }

    return -1;
}
char intToHex(int x) {
    if (x >= 0 && x < 10)
        return (x + 48);
    else if (x >= 10 && x <16) {
        return (x + 87);
    }
    return -1;
}
void writeHex(int x, FILE *f) {
    char one = '0';
    char two = '0';
    char three = '0';
    char four = '0';
    int index = 0;
    int remainder;
    do {
        remainder = x % 16;
        if (index == 0)
            four = intToHex(remainder);
        else if (index == 1)
            three = intToHex(remainder);
        else if (index == 2)
            two = intToHex(remainder);
        else if (index == 3)
            one = intToHex(remainder);
        index = index + 1;
        x = x /16;
    } while(x != 0);
    // if (index > 5)
    //     return -1;
    fputc('\\', f);
    fputc('u', f);
    fputc(one, f);
    fputc(two, f);
    fputc(three, f);
    fputc(four, f);
    // return 0;
}

int specialValue(int x, FILE *f) {
    if (x == '\"') {
        fputs("\\", f);
        fputc('"', f);
    } else if (x == '\\') {
        fputs("\\", f);
        fputc('\\', f);
    } else if (x == 8) {
        fputs("\\", f);
        fputc('b', f);
    } else if (x == 12) {
        fputs("\\", f);
        fputc('f', f);
    } else if (x == 10) {
        fputs("\\", f);
        fputc('n', f);
    } else if (x == 13) {
        fputs("\\", f);
        fputc('r', f);
    } else if (x == 9) {
        fputs("\\", f);
        fputc('t', f);
    } else if (x <= 0x1F || x > 0xFF) {
        writeHex(x, f);
    } else if (x > 0x1F && x < 0xFF) {
        return 1;
    } else {
        return -1;
    }
    return x;
}
/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f) {
    int tmpIndex = 0;
    ARGO_CHAR *tmpStringArr = (*s).content;
    fputc('"', f);
    while(tmpIndex < (*s).length) {
        int foundSpecial = specialValue(*(tmpStringArr + tmpIndex), f);
        if (foundSpecial ==  1)
            fputc(*(tmpStringArr + tmpIndex), f);
        tmpIndex = tmpIndex + 1;
    }
    fputc('"', f);
    return 1;
}

void printInt(long int x, FILE *f) {
    if (x / 10 == 0)
        fputc(x + 48, f);
    if (x / 10 != 0) {
        printInt(x / 10, f);
        fputc((x % 10) + 48, f);
    }
}

void printWhole(int x, int zeroFlag, FILE *f) {
    if (x % 10 != 0)
        zeroFlag = 0;
    if (x / 10 == 0)
        fputc(x + 48, f);
    if (x / 10 != 0) {
        printWhole(x / 10, zeroFlag, f);
        if (zeroFlag == 0)
            fputc((x % 10) + 48, f);
    }
}
void printFloat(double x, FILE *f) {
    // int exponent = 0;
    // double decimalValue = x - ((int)x);
    if ( x < 0) {
        fputc('-', f);
        x = x * -1;
    }
    fputc('0', f);
    fputc('.', f);
    int precisionCounter = 0;
    double copy = x;
    int exponent = 0;
    int zeroFlag = -1;
    if (x < 0)
        x = x * -1;
    if (x >= 1) {
        int whole = (int)x;
        while (whole != 0) {
            whole = whole / 10;
            exponent = exponent + 1;
            precisionCounter = precisionCounter + 1;
        }
        whole = (int)x;
        printWhole(whole, zeroFlag, f);
        copy = copy -(int)copy;
    } else if (copy == 0) {
        fputc('0', f);
        return;
    } else {
        while (copy < 1) {
            copy = copy * 10;
            exponent = exponent - 1;
        }
        exponent = exponent + 1;
        if (copy - (int)copy <= 0) {
            printInt((int)copy, f);
        }
    }
    int tenthSpot = 0;
    while (copy - (int)copy > 0 && precisionCounter  <= ARGO_PRECISION) {
        if (exponent < 0 || tenthSpot == -1)
            fputc((int)copy + 48, f);
        copy = copy - (int)copy;
        copy = copy * 10;
        precisionCounter = precisionCounter + 1;
        tenthSpot = -1;
        }
    if (exponent >= 0){
        fputc('e', f);
        fputc(exponent + 48, f);
    } else {
        fputc('e', f);
        exponent = exponent * -1;
        fputc('-', f);
        fputc(exponent + 48, f);
    }

}
/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
//  */
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    if ((*n).valid_int != 0) {
        long vint = (*n).int_value;
        if ((*n).int_value < 0) {
            vint = vint * -1;
            fputc('-', f);
        }
        printInt(vint, f);
    } else if ((*n).valid_float != 0)    {
        printFloat(((*n).float_value), f);
    }
    return 0;
}
