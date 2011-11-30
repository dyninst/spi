/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE121_Stack_Based_Buffer_Overflow__char_type_overrun_memcpy_02.c
Label Definition File: CWE121_Stack_Based_Buffer_Overflow.label.xml
Template File: point-flaw-02.tmpl.c
*/
/*
 * @description
 * CWE: 121 Stack Based Buffer Overflow
 * Sinks: type_overrun_memcpy
 *    GoodSink: Perform the memcpy() and prevent overwriting part of the structure
 *    BadSink : Overwrite part of the structure by incorrectly using the sizeof(struct) in memcpy()
 * Flow Variant: 02 Control flow: if(1) and if(0)
 *
 * */

#include "std_testcase.h"

/* SRC_STR is 20 char long, including the null terminator */
#define SRC_STR "0123456789abcde0123"

typedef struct _charvoid
{
    char x[16];
    void * y;
    void * z;
} charvoid;

#ifndef OMITBAD

void CWE121_Stack_Based_Buffer_Overflow__char_type_overrun_memcpy_02_bad()
{
    if(1)
    {
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FLAW: Use the sizeof(cv_struct) which will overwrite the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
    else
    {
        /* INCIDENTAL: CWE 561 Dead Code, the code below will never run */
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FIX: Use sizeof(cv_struct.x) to avoid overwriting the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct.x));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* good1() uses if(0) instead of if(1) */
static void good1()
{
    if(0)
    {
        /* INCIDENTAL: CWE 561 Dead Code, the code below will never run */
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FLAW: Use the sizeof(cv_struct) which will overwrite the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
    else
    {
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FIX: Use sizeof(cv_struct.x) to avoid overwriting the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct.x));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
}

/* good2() reverses the bodies in the if statement */
static void good2()
{
    if(1)
    {
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FIX: Use sizeof(cv_struct.x) to avoid overwriting the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct.x));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
    else
    {
        /* INCIDENTAL: CWE 561 Dead Code, the code below will never run */
        {
            charvoid cv_struct;
            cv_struct.y = SRC_STR;
            /* Print the initial block pointed to by cv_struct.y */
            printLine((char *)cv_struct.y);
            /* FLAW: Use the sizeof(cv_struct) which will overwrite the pointer y */
            memcpy(cv_struct.x, SRC_STR, sizeof(cv_struct));
            cv_struct.x[(sizeof(cv_struct.x)/sizeof(char))-1] = '\0'; /* null terminate the string */
            printLine((char *)cv_struct.x);
            printLine((char *)cv_struct.y);
        }
    }
}

void CWE121_Stack_Based_Buffer_Overflow__char_type_overrun_memcpy_02_good()
{
    good1();
    good2();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */

#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE121_Stack_Based_Buffer_Overflow__char_type_overrun_memcpy_02_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");
    CWE121_Stack_Based_Buffer_Overflow__char_type_overrun_memcpy_02_bad();
    printLine("Finished bad()");
#endif /* OMITBAD */
    return 0;
}

#endif
