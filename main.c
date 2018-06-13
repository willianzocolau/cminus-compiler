#include <getopt.h>

#include "globals.h"
#include "util.h"

/* Set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE

/* Set NO_ANALYSE to TRUE to get a parser-only compiler */
#define NO_ANALYSE FALSE

/* Set NO_CODE to get a compiler that does not generate code */
#define NO_CODE FALSE

#if NO_PARSE
#define BUILDTYPE "SCANNER ONLY"
#include "scan.h" /* Test the scanner only */
#else
#include "parse.h"
#if !NO_ANALYSE
#define BUILDTYPE "SCANNER/PARSER/ANALYSER ONLY"
#include "analyze.h"
#if !NO_CODE
#undef BUILDTYPE
#define BUILDTYPE "COMPLETE COMPILER"
#include "cgen.h"
#endif
#else
#define BUILDTYPE "SCANNER/PARSER ONLY"
#endif
#endif

int lineno = 0;

FILE *source;  /*  The input source file            */
FILE *listing; /*  The listing output file          */
FILE *code;    /*  The target code destination file */

#define MAXFILENAMESIZE 64

char sourceFileName[MAXFILENAMESIZE];

int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;

int Error = FALSE;

/* The syntax tree */
TreeNode *syntaxTree;

int ParseCommandLine(int argc, char **argv)
{
    char c;                /* the option being parsed    */
    int errorFlag = 0;     /* has an error occurred yet? */
    int gotSourceName = 0; /* we need the source file name */

    opterr = 0; /* Suppress getopt()'s default error-handing behavior */
    while ((c = getopt(argc, argv, "slyacf:")) != EOF)
    {
        switch (c)
        {
        case 's':
            EchoSource = TRUE;
            break;
        case 'l':
            TraceScan = TRUE;
            break;
        case 'y':
            TraceParse = TRUE;
            break;
        case 'a':
            TraceAnalyze = TRUE;
            break;
        case 'c':
            TraceCode = TRUE;
            break;
        case 'f':
            /* Can't specify filename more than once */
            if (gotSourceName)
                errorFlag++;
            else
            {
                gotSourceName = TRUE;
                strncpy(sourceFileName, optarg, MAXFILENAMESIZE);
            }
            break;
        default:
            errorFlag++;
        } /* switch(c) */
    }

    /* Source file argument is mandatory */
    if (!gotSourceName)
        ++errorFlag;

    return errorFlag;
}

int main(int argc, char **argv)
{
    if (ParseCommandLine(argc, argv) != 0)
    {
        exit(1);
    }

    if (strchr(sourceFileName, '.') == NULL)
        strcat(sourceFileName, ".cm");

    /* Open the source file */
    source = fopen(sourceFileName, "r");

    /* If it failed, bomb out. */
    if (source == NULL)
    {
        fprintf(stderr, "Sorry, but the source file %s could not be found.\n",
                sourceFileName);
        exit(1);
    };

    /* By default, send output to standard output */
    listing = stdout;

    fprintf(listing, "*** C- COMPILATION: %s\n", sourceFileName);
    fprintf(listing, "*** Compiler built as " BUILDTYPE " version.\n");

    /* If the compiler was built scanner-only, then only run the scanner */
#if NO_PARSE
    while (getToken() != ENDOFFILE)
    {
        /* do nothing */
    };
#else
    fprintf(listing, "*** Parsing source program...\n");
    syntaxTree = parse();

    /* Tracing enabled?  Let's have it... */
    if (TraceParse)
    {
        fprintf(listing, "*** Dumping syntax tree\n");
        printTree(syntaxTree);
    };

#if !NO_ANALYSE
    if (!Error)
    {
        fprintf(listing, "*** Building symbol table...\n");
        buildSymtab(syntaxTree);
        fprintf(listing, "*** Performing type checking...\n");
        typeCheck(syntaxTree);
    }

#if !NO_CODE
    if (!Error)
    {
        char *codefile;
        int fnlen = strcspn(sourceFileName, ".");
        codefile = (char *)calloc(fnlen + 4, sizeof(char));
        strncpy(codefile, sourceFileName, fnlen);
        strcat(codefile, ".asm");
        code = fopen(codefile, "w");
        if (code == NULL)
        {
            printf("Unable to open %s\n", codefile);
            exit(1);
        }
        codeGen(syntaxTree, codefile);
        fprintf(listing, "*** Output written to \"%s\"\n", codefile);
        if (TraceCode)
            fprintf(listing, "*** CODE TRACING OPTION ENABLED; see output\n");
        fclose(code);
    }
#endif
#endif
#endif

    if (!Error)
        fprintf(listing, "*** COMPILATION COMPLETE: %d lines processed.\n", lineno);
    else
        fprintf(listing, "*** ERRORS WERE ENCOUNTERED: %d lines processed.\n", lineno);

    return EXIT_SUCCESS;
}

/* END OF FILE */
