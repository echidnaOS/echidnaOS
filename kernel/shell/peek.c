#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"
#include <kernel.h>

void peek(char *argstring)
{
    long type, repeat, repr, output, indirection;
    type = repr = output = indirection = 0;
    intptr_t location = 0;
    repeat = 1;

#define TYPE_BYTE    1
#define TYPE_WORD    2
#define TYPE_DWORD   3
#define TYPE_QWORD   4
#define TYPE_FLOAT   5
#define TYPE_DOUBLE  6
#define TYPE_STRING  7

#define REPR_CHAR    1
#define REPR_HEX     2
#define REPR_OCT     3
#define REPR_BIN     4

    for (;;)
    {
        switch ( *argstring )
        {
            //end of string
            case 0:
            {
                if ( !output )
                    goto help;

                return;
            }

            //--help
            case '-':
            {
                if ( indirection )
                    goto bad_arg;

                if ( strncmp(argstring, "--help", 6) == 0 )
                    goto help;

                //skip malformed argument
                goto bad_arg;
            }

            //separator
            case ' ':
            {
                argstring++;

                if ( indirection )
                    indirection = 0;

                continue;
            }

            //modifier
            case '/':
            {
                //modifiers with preceding asterisks are considered malformed
                if ( indirection )
                    goto bad_arg;

                long newrepeat = 1;
                long newtype = 0;
                long newrepr = 0;

                if ( isdigit(*(++argstring)) )
                {
                    char *endptr = NULL;
                    newrepeat = strtoul(argstring, &endptr, 10);
                    if ( !repeat )
                        repeat = 1;
                    argstring = endptr;
                }

                switch ( *argstring++ )
                {
                    case 'b':
                        newtype = TYPE_BYTE;
                        break;

                    case 'c':
                        newtype = TYPE_BYTE;
                        newrepr = REPR_CHAR;

                        if ( *argstring != ' ' )
                            goto bad_arg;

                        break;

                    case 's':
                        newtype = TYPE_STRING;

                        if ( *argstring != ' ' )
                            goto bad_arg;

                    case 'f':
                        newtype = TYPE_FLOAT;

                    //'lf' double
                    case 'l':
                        if ( *argstring != 'f' )
                            goto bad_arg;

                        newtype = TYPE_DOUBLE;
                        argstring++;
                        break;

                    case 'w':
                        newtype = TYPE_WORD;
                        break;

                    case 'd':
                        newtype = TYPE_DWORD;
                        break;

                    case 'q':
                        newtype = TYPE_QWORD;
                        break;

                    //invalid
                    default:
                        goto bad_arg;
                }

                switch ( *argstring++ )
                {
                    case ' ':
                        break;

                    case 'x':
                        newrepr = REPR_HEX;
                        break;

                    case 'o':
                        newrepr = REPR_OCT;
                        break;

                    case 'b':
                        newrepr = REPR_BIN;
                }

                if ( *argstring != ' ' && argstring[-1] != ' ' )
                    goto bad_arg;

                type = newtype;
                repr = newrepr;
                repeat = newrepeat;

                printf("DEBUG:\ntype = %ld;\nrepr = %ld;\nrepeat = %ld;\n\n", type, repr, repeat);
            }

            //indirection
            case '*':
            {
                while ( *argstring == '*' )
                {
                    argstring++;
                    indirection++;
                }

                continue;
            }

            //location
            case '0':
            {
                if ( tolower(*(++argstring)) == 'x' )
                {
                    char *end = NULL;
                    location = strtoul(++argstring, &end, 16);

                    if ( !location || (*end && !isspace(*end)) )
                        goto bad_arg;

                    printf("DEBUG:\nlocation = %lx;\n\n", location);

                    goto print_value;
                }
                
                //skip malformed argument
                indirection = 0;
                location = 0;

                goto bad_arg;
            }

            //flag, register, or invalid
            default:
            {
                if ( isdigit(*argstring) )
                    goto bad_arg;

                int i = 0;

                //worst loop ever written
                //looks up flag/register
                while (

                    //valid index
                    i < FLAGS_REGS_COUNT &&

                    //matching identifier not found
                    strncasecmp(
                        argstring,
                        flagsNregs[i],
                        strlen(flagsNregs[i])
                    ) != 0
                  //increment
                ) i++; /* end of loop */

                if ( i == FLAGS_REGS_COUNT )
                {
                    indirection = 0;
                    continue;
                }

                printf("DEBUG:\nfound %s\n\n", flagsNregs[i]);

                char value[32];
                memset(value, 0, 32);

                get_register_value(flagsNregs[i], value);
                if ( indirection )
                    location = *(long *)value;
                else
                    location = (long)(void*)value;

print_value:
                while ( indirection )
                    location = (long)*(char **)location;

                //print
                char *fstring = NULL;
                char buf[9];
                memset(buf, 0, 9);

                switch ( type )
                {
                    case TYPE_BYTE:
                    {
                        switch ( repr )
                        {
                            case REPR_CHAR:
                                fstring = "%c";
                                break;

                            case REPR_BIN:
                                fstring = "%s ";
                                break;

                            case REPR_OCT:
                                fstring = "%03hho ";
                                break;

                            case REPR_HEX:
                                fstring = "%02hhx ";
                                break;

                            default:
                                fstring = "%03hhd ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            //don't just pile them on one line: 8 bytes to a line
                            if ( j % 8 == 0 && repr != REPR_CHAR )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                ltoa(*(uint8_t *)location, buf, 2);

                                for ( int k = strlen(buf); k < 8; k++ )
                                    text_putchar('0');

                                printf(fstring, buf);
                                memset(buf, 0, 9);
                            }
                            else
                                printf(fstring, *(uint8_t *)location);

                            location += sizeof(int8_t);
                        }

                        text_putchar('\n');

                        break;
                    }

                    case TYPE_WORD:
                    {
                        switch ( repr )
                        {
                            case REPR_BIN:
                                fstring = "%s ";
                                break;

                            case REPR_OCT:
                                fstring = "%06ho ";
                                break;

                            case REPR_HEX:
                                fstring = "%04hx ";
                                break;

                            default:
                            fstring = "%05hd ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            if ( j % 8 == 0 )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                for ( int k = 0; k < 2; k++ )
                                {
                                    ltoa(*(uint8_t *)(location+k), buf, 2);

                                    for ( int l = strlen(buf); l < 8; k++ )
                                        text_putchar('0');

                                    printf(fstring, buf);
                                    memset(buf, 0, 9);
                                }
                            }
                            else
                                printf(fstring, *(uint16_t *)location);

                            location += sizeof(int16_t);
                        }

                        text_putchar('\n');

                        break;
                    }

                    case 0:
                    case TYPE_DWORD:
                        switch ( repr )
                        {
                            case REPR_BIN:
                                fstring = "%s ";
                                break;

                            case REPR_OCT:
                                fstring = "%011o ";
                                break;

                            case REPR_HEX:
                                fstring = "%08x ";
                                break;

                            default:
                            fstring = "%010d ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            if ( j % 6 == 0 )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                if ( j && j % 2 == 0 )
                                    text_putchar('\n');

                                for ( int k = 0; k < 4; k++ )
                                {
                                    ltoa(*(uint8_t *)(location+k), buf, 2);

                                    for ( int l = strlen(buf); l < 8; l++ )
                                        text_putchar('0');

                                    printf(fstring, buf);
                                    memset(buf, 0, 9);
                                }
                            }
                            else
                                printf(fstring, *(uint32_t *)location);

                            location += sizeof(int32_t);
                        }

                        text_putchar('\n');

                        break;

                    case TYPE_QWORD:
                    {
                        switch ( repr )
                        {
                            case REPR_BIN:
                                fstring = "%s";
                                break;

                            case REPR_OCT:
                                fstring = "%022lo ";
                                break;

                            case REPR_HEX:
                                fstring = "%04lx ";
                                break;

                            default:
                            fstring = "%05ld ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            if ( j % 2 == 0 )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                if ( j & 1 )
                                    text_putchar('\n');

                                for ( int k = 0; k < 8; k++ )
                                {
                                    ltoa(*(uint8_t *)(location+k), buf, 2);

                                    for ( int l = strlen(buf); l < 8; l++ )
                                        text_putchar('0');

                                    printf(fstring, buf);
                                    memset(buf, 0, 9);
                                }
                            }
                            else
                                printf(fstring, *(uint64_t *)location);

                            location += sizeof(int64_t);
                        }

                        text_putchar('\n');

                        break;
                    }

                    case TYPE_FLOAT:
                        switch ( repr )
                        {
                            case REPR_BIN:
                                fstring = "%s ";
                                break;

                            case REPR_OCT:
                                fstring = "%011o ";
                                break;

                            case REPR_HEX:
                                fstring = "%08x ";
                                break;

                            default:
                            fstring = "%f ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            if ( j % 6 == 0 )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                for ( int k = 0; k < 4; k++ )
                                {
                                    ltoa(*(int8_t *)(location+k), buf, 2);

                                    for ( int l = strlen(buf); l < 8; l++ )
                                        text_putchar('0');

                                    printf(fstring, buf);
                                    memset(buf, 0, 9);
                                }
                            }
                            else
                                printf(fstring, *(float *)location);

                            location += sizeof(float);
                        }

                        text_putchar('\n');

                        break;

                    case TYPE_DOUBLE:
                        switch ( repr )
                        {
                            case REPR_BIN:
                                fstring = "%s ";
                                break;

                            case REPR_OCT:
                                fstring = "%011o ";
                                break;

                            case REPR_HEX:
                                fstring = "%08x ";
                                break;

                            default:
                            fstring = "%010d ";
                        }

                        for ( int j = 0; j < repeat; j++ )
                        {
                            if ( j % 6 == 0 )
                                text_putchar('\n');

                            if ( repr == REPR_BIN )
                            {
                                for ( int k = 0; k < 8; k++ )
                                {
                                    ltoa(*(int8_t *)(location+k), buf, 2);

                                    for ( int l = strlen(buf); l < 8; l++ )
                                        text_putchar('0');

                                    printf(fstring, buf);
                                    memset(buf, 0, 9);
                                }
                            }
                            else
                                printf(fstring, *(double *)location);

                            location += sizeof(double);
                        }

                        text_putchar('\n');

                        break;

                    case TYPE_STRING:
                        for ( int j = 0; j < repeat; j++ )
                        {
                            puts(*(char **)location);
                            location += sizeof(void *);
                        }
                }

                location = 0;
                output = 1;
                goto bad_arg;
            }
        }

bad_arg:
        while ( *argstring && *argstring != ' ' )
            argstring++;
    }

help:
    puts(
         "Usage:  peek [modifier] <location> [...]\n"
         "\n"
         "    A location is a hexadecimal number starting with the 0x prefix\n"
         "    which represents a location in memory. It may also be the name of\n"
         "    a register (e.g. esp), or the name of a flag (e.g. CF). Indirection\n"
         "    can be used by means of the '*'' character prefixing the location as\n"
         "    long as the location is not a flag. A hexadecimal location must be 4\n"
         "    byte aligned, unless it has the byte, word, or char modifier, which\n"
         "    have alignments equal to their size in bytes, affecting it.\n"
         "\n"
         "    Modifiers affect how the value at the location is displayed. A\n"
         "    modifier starts with a '/', optionally has a repeat argument, and\n"
         "    ends with a type character. The repeat argument is a base 10 integer\n"
         "    that denotes how many items of the following type will be displayed\n"
         "    starting from the location argument it modifies, and the type argument\n"
         "    is one of:\n"
         "\n"
         "        b  (byte)\n"
         "        c  (char)\n"
         "        s  (string)\n"
         "        f  (single precision float)\n"
         "        lf (double precision float)\n"
         "        w  (word)\n"
         "        d  (double word)\n"
         "        q  (quad word)\n"
         "\n"
         "    Numerical types can also have a trailing b (binary), o (octal), or\n"
         "    x (hexadecimal) trailing them, however, f and lf will be represented\n"
         "    as integral types based on their binary representation. Without one of\n"
         "    these three, the output will be in base 10. A modifier modifies all of\n"
         "    the following location arguments until a new modifier is given.\n"
         "    Modifiers have no effect on flags.\n"
         "\n"
         "    Warnings:\n"
         "    Malformed modifiers and locations will be silently ignored.\n"
         "    Attempting to access invalid memory will cause segfaults.\n"
         "\n"
         "    Examples:\n"
         "        peek /s 0xFFFF3538\n"
         "        peek /8dx *ebp\n"
         "        peek CF\n"
         "        peek eax"
    );
}

void get_register_value(char *regname, void *var)
{
    long bitmask, *tmp = var;
    int i = 0;

    //worst loop ever written
    //looks up flag/register
    while (

        //valid index
        i < FLAGS_REGS_COUNT &&

        //matching identifier not found
        strncasecmp(
            regname,
            flagsNregs[i],
            strlen(regname)
        ) != 0
      //increment
    ) i++; /* end of loop */

#define gpr_grab(reg) \
asm volatile (             \
    "mov %0,"reg"\n"       \
    :                      \
    : "m" (*tmp)           \
    :                      \
)

#ifdef __64BIT__
#define PREFIX "r"
#else
#define PREFIX "e"
#endif

    switch ( i )
    {

        case 0:
            gpr_grab(PREFIX"ax");
            break;

        case 1:
            gpr_grab(PREFIX"ax");
            break;

        case 2:
            gpr_grab(PREFIX"ax");
            break;

        case 3:
            gpr_grab(PREFIX"ax");
            break;

        case 4:
            gpr_grab(PREFIX"ax");
            break;

        case 5:
            gpr_grab(PREFIX"bx");
            break;

        case 6:
            gpr_grab(PREFIX"bx");
            break;

        case 7:
            gpr_grab(PREFIX"bx");
            break;

        case 8:
            gpr_grab(PREFIX"bx");
            break;

        case 9:
            gpr_grab(PREFIX"bx");
            break;

        case 10:
            gpr_grab(PREFIX"cx");
            break;

        case 11:
            gpr_grab(PREFIX"cx");
            break;

        case 12:
            gpr_grab(PREFIX"cx");
            break;

        case 13:
            gpr_grab(PREFIX"cx");
            break;

        case 14:
            gpr_grab(PREFIX"cx");
            break;

        case 15:
            gpr_grab(PREFIX"dx");
            break;

        case 16:
            gpr_grab(PREFIX"dx");
            break;

        case 17:
            gpr_grab(PREFIX"dx");
            break;

        case 18:
            gpr_grab(PREFIX"dx");
            break;

        case 19:
            gpr_grab(PREFIX"dx");
            break;

        case 20:
            gpr_grab(PREFIX"si");
            break;

        case 21:
            gpr_grab(PREFIX"si");
            break;

        case 22:
            gpr_grab(PREFIX"si");
            break;

        case 23:
            gpr_grab(PREFIX"si");
            break;

        case 24:
            gpr_grab(PREFIX"di");
            break;

        case 25:
            gpr_grab(PREFIX"di");
            break;

        case 26:
            gpr_grab(PREFIX"di");
            break;

        case 27:
            gpr_grab(PREFIX"di");
            break;

        case 28:
            gpr_grab(PREFIX"sp");
            break;

        case 29:
            gpr_grab(PREFIX"sp");
            break;

        case 30:
            gpr_grab(PREFIX"sp");
            break;

        case 31:
            gpr_grab(PREFIX"sp");
            break;

        case 32:
            gpr_grab(PREFIX"bp");
            break;

        case 33:
            gpr_grab(PREFIX"bp");
            break;

        case 34:
            gpr_grab(PREFIX"bp");
            break;

        case 35:
            gpr_grab(PREFIX"bp");
            break;

        case 36:
            asm volatile (
                "call get_retval;\n"
                "get_retval:\n"
                "pop %0;\n"
                :
                : "m" (*tmp)
                :
            );
            break;

        case 37:
            asm volatile (
                "call get_retval1;\n"
                "get_retval1:\n"
                "pop %0;\n"
                :
                : "m" (*tmp)
                :
            );
            break;

        case 38:
            asm volatile (
                "call get_retval2;\n"
                "get_retval2:\n"
                "pop %0;\n"
                :
                : "m" (*tmp)
                :
            );
            break;
/* not yet bothering with fp
        case 39:
            gpr_grab("st(0)");
            break;

        case 40:
            gpr_grab("mm0");
            break;

        case 41:
            gpr_grab("st(1)");
            break;

        case 42:
            gpr_grab("mm1");
            break;

        case 43:
            gpr_grab("st(2)");
            break;

        case 44:
            gpr_grab("mm2");
            break;

        case 45:
            gpr_grab("st(3)");
            break;

        case 46:
            gpr_grab("mm3");
            break;

        case 47:
            gpr_grab("st(4)");
            break;

        case 48:
            gpr_grab("mm4");
            break;

        case 49:
            gpr_grab("st(5)");
            break;

        case 50:
            gpr_grab("mm5");
            break;

        case 51:
            gpr_grab("st(6)");
            break;

        case 52:
            gpr_grab("mm6");
            break;

        case 53:
            gpr_grab("st(7)");
            break;

        case 54:
            gpr_grab("mm7");
            break;

        case 55:
            break;

        case 56:
            gpr_grab("ymm0");
            break;

        case 57:
            gpr_grab("xmm0");
            break;

        case 58:
            break;

        case 59:
            gpr_grab("ymm1");
            break;

        case 60:
            gpr_grab("xmm1");
            break;

        case 61:
            break;

        case 62:
            gpr_grab("ymm2");
            break;

        case 63:
            gpr_grab("xmm2");
            break;

        case 64:
            break;

        case 65:
            gpr_grab("ymm3");
            break;

        case 66:
            gpr_grab("xmm3");
            break;

        case 67:
            break;

        case 68:
            gpr_grab("ymm4");
            break;

        case 69:
            gpr_grab("xmm4");
            break;

        case 70:
            break;

        case 71:
            gpr_grab("ymm5");
            break;

        case 72:
            gpr_grab("xmm5");
            break;

        case 73:
            break;

        case 74:
            gpr_grab("ymm6");
            break;

        case 75:
            gpr_grab("xmm6");
            break;

        case 76:
            break;

        case 77:
            gpr_grab("ymm7");
            break;

        case 78:
            gpr_grab("xmm7");
            break;

        case 79:
            break;

        case 80:
            gpr_grab("ymm8");
            break;

        case 81:
            gpr_grab("xmm8");
            break;

        case 82:
            break;

        case 83:
            gpr_grab("ymm9");
            break;

        case 84:
            gpr_grab("xmm9");
            break;

        case 85:
            break;

        case 86:
            gpr_grab("ymm10");
            break;

        case 87:
            gpr_grab("xmm10");
            break;

        case 88:
            break;

        case 89:
            gpr_grab("ymm11");
            break;

        case 90:
            gpr_grab("xmm11");
            break;

        case 91:
            break;

        case 92:
            gpr_grab("ymm12");
            break;

        case 93:
            gpr_grab("xmm12");
            break;

        case 94:
            break;

        case 95:
            gpr_grab("ymm13");
            break;

        case 96:
            gpr_grab("xmm13");
            break;

        case 97:
            break;

        case 98:
            gpr_grab("ymm14");
            break;

        case 99:
            gpr_grab("xmm14");
            break;

        case 100:
            break;

        case 101:
            gpr_grab("ymm15");
            break;

	    case 102:
	        gpr_grab("xmm15");
            break;

        case 103:
            break;

        case 104:
            break;

        case 105:
            break;

        case 106:
            break;

        case 107:
            break;

        case 108:
            break;

        case 109:
            break;

        case 110:
            break;

        case 111:
            break;

        case 112:
            break;

        case 113:
            break;

        case 114:
            break;

        case 115:
            break;

        case 116:
            break;

        case 117:
            break;

        case 118:
            break;
*/
#define cr_get(reg) \
asm volatile (           \
    "mov %0,"reg";\n"    \
    : "=r" (*tmp)        \
    :                    \
    :                    \
)

        case 119:
            cr_get("cr0");
            break;
/*  There is no CR1
        case 120:
            cr_get("cr1");
            break;
*/
        case 121:
            cr_get("cr2");
            break;

        case 122:
            cr_get("cr3");
            break;

        case 123:
            cr_get("cr4");
            break;
/*
        case 124:
            cr_get("cr5");
            break;

        case 125:
            cr_get("cr6");
            break;

        case 126:
            cr_get("cr7");
            break;
*/
        case 127:
            cr_get("cr8");
            break;
#ifdef __64BIT__
        case 128:
            cr_get("cr9");
            break;

        case 129:
            cr_get("cr10");
            break;

        case 130:
            cr_get("cr11");
            break;

        case 131:
            cr_get("cr12");
            break;

        case 132:
            cr_get("cr13");
            break;

        case 133:
            cr_get("cr14");
            break;

        case 134:
            cr_get("cr15");
            break;
#endif
#define dr_get(reg) \
asm volatile (           \
    "mov %0,"reg";\n"    \
    : "=r" (*tmp)         \
    :                    \
    :                    \
)

        case 135:
            dr_get("dr0");
            break;

        case 136:
            dr_get("dr1");
            break;

        case 137:
            dr_get("dr2");
            break;

        case 138:
            dr_get("dr3");
            break;
/*
        case 139:
            dr_get("dr4");
            break;

        case 140:
            dr_get("dr5");
            break;

        case 141:
            dr_get("dr6");
            break;

        case 142:
            dr_get("dr7");
            break;*/
#ifdef __64BIT__
        case 143:
            dr_get("dr8");
            break;

        case 144:
            dr_get("dr9");
            break;

        case 145:
            dr_get("dr10");
            break;

        case 146:
            dr_get("dr11");
            break;

        case 147:
            dr_get("dr12");
            break;

        case 148:
            dr_get("dr13");
            break;

        case 149:
            dr_get("cr14");
            break;

        case 150:
            dr_get("dr15");
            break;
#endif
        case 167: //gdtr
        case 168:
        case 169:
        case 170:
            //
            break;
        case 171:  //rflags
            bitmask = ~0ul;
            goto flags;
        case 172: //eflags
            bitmask = 0xFFFFFFFFul;
            goto flags;
        case 173: //flags
            bitmask = 0xFFFFul;
            goto flags;
        case 174:
            bitmask = 1 << 0;
            goto flags;
        case 175:
            bitmask = 1 << 2;
            goto flags;
        case 176:
            bitmask = 1 << 4;
            goto flags;
        case 177:
            bitmask = 1 << 6;
            goto flags;
        case 178:
            bitmask = 1 << 7;
            goto flags;
        case 179:
            bitmask = 1 << 8;
            goto flags;
        case 180:
            bitmask = 1 << 9;
            goto flags;
        case 181:
            bitmask = 1 << 10;
            goto flags;
        case 182:
            bitmask = 1 << 11; 
            goto flags;
        case 183:
            bitmask = 3 << 12;
            goto flags;
        case 184:
            bitmask = 1 << 14;
            goto flags;
        case 185:
            bitmask = 1 << 16;
            goto flags;
        case 186:
            bitmask = 1 << 17;
            goto flags;
        case 187:
            bitmask = 1 << 18;
            goto flags;
        case 188:
            bitmask = 1 << 19;
            goto flags;
        case 189:
            bitmask = 1 << 20;
            goto flags;
        case 190:
            bitmask = 1 << 21;
flags:
            asm volatile (
                "pushfd;"
                "pop %0;"
                :
                : "m" (*tmp)
                :
            );
            *(unsigned long *)var = *(unsigned long *)var & bitmask ? 1 : 0;
            break;

        default:
            break;
    }
}
