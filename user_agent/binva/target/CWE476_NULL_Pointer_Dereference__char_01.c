/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE476_NULL_Pointer_Dereference__char_01.c
Label Definition File: CWE476_NULL_Pointer_Dereference.label.xml
Template File: sources-sink-01.tmpl.c
*/
/*
 * @description
 * CWE: 476 NULL Pointer Dereference
 * BadSource:  Set data to NULL
 * GoodSource: Initialize data
 * Sink:
 *    BadSink : Print data
 * Flow Variant: 01 Baseline
 *
 * */


#include <wchar.h>

#ifndef OMITBAD

void CWE476_NULL_Pointer_Dereference__char_01_bad()
{
    char * data;
    /* Initialize data */
    data = NULL;
    /* FLAW: Set data to NULL - it will be used in the sink */
    data = NULL;
    /* POTENTIAL FLAW: Attempt to use data, which may be NULL */
    /* printLine() checks for NULL, so we cannot use it here */
    printf("%s\n", data);
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* goodG2B uses the GoodSource with the BadSink */
static void goodG2B()
{
    char * data;
    /* Initialize data */
    data = NULL;
    /* FIX: Initialize data - it will be used in the sink */
    data = "Good";
    /* POTENTIAL FLAW: Attempt to use data, which may be NULL */
    /* printLine() checks for NULL, so we cannot use it here */
    printf("%s\n", data);
}

void CWE476_NULL_Pointer_Dereference__char_01_good()
{
    goodG2B();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */


int main(int argc, char * argv[])
{
    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    CWE476_NULL_Pointer_Dereference__char_01_good();
#endif /* OMITGOOD */
#ifndef OMITBAD
    CWE476_NULL_Pointer_Dereference__char_01_bad();
#endif /* OMITBAD */
    return 0;
}

