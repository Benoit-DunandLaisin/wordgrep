#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIGGEST_DICT_WORD 30+1
#define SIZEOF_DICTIONNARY_WORDLIST 2000

struct dictionnary_block
{
    char *pWordList;
    int iMaxMemory;
    int iUsedMemory;
    struct dictionnary_block *pNext;
};
struct dictionnary
{
    struct dictionnary_block *pHead;
    int iNumberOfWord;
    int iBiggestWord;
};

/* Load and parse a file as dictionnary */
int parseDictionnary(struct dictionnary *pDict, const char *pDictFile);
/* Check the existence of a word in the dictionnary */
int grepWord(const struct dictionnary *pDict, const char *pWord, const int bFreeMemory);
/* Check the existence of a word in a dictionnary block */
int grepWordInBlocks(const struct dictionnary_block *pBlock, const char *sSearched);
/* Read an input and wordgrep it*/
int readInput(const struct dictionnary *pDict, FILE *fInput, const int bFreeMemory);

/******* MAIN ******/
int main(int argc, char *argv[])
{
    /* Variable definitions and initialisations*/
    struct dictionnary myDict;
    FILE *fFileInput;
    int i = 0;

    myDict.pHead = NULL;
    myDict.iNumberOfWord = 0;
    myDict.iBiggestWord = 0;

    /* Go */
#ifdef SHOW_DEBUG
    fprintf(stderr,"Start main function\n");
#endif
    if (argc == 1)
    {
        fprintf(stderr,"Without a dictionnary file, there is nothing to do.\n");
        return 1;
    }
    else
    {
        if (parseDictionnary(&myDict, argv[1]))
        {
            return 1;
        }

        if (argc == 2)
        {
            /* No input file, so read the standard input */
#ifdef SHOW_DEBUG
            fprintf(stderr,"Start reading standard input\n");
#endif
            readInput(&myDict, stdin, 0);
        }
        else
        {
            for(i=2; i<argc; i++)
            {
#ifdef SHOW_DEBUG
                fprintf(stderr,"Start reading %s\n", argv[i]);
#endif
                fFileInput=fopen(argv[i],"r");
                if (fFileInput == NULL)
                {
                    perror(argv[i]);
                }
                else
                {
                    readInput(&myDict, fFileInput, 0);
                    fclose(fFileInput);
                }
            }
        }
#ifdef SHOW_DEBUG
        fprintf(stderr,"End good main function\n");
#endif
        readInput(NULL,NULL,1);
        grepWord(NULL,NULL,1);
        if (myDict.pHead != NULL)
        {
            if (myDict.pHead->pWordList != NULL)
            {
                free(myDict.pHead->pWordList);
            }
            free(myDict.pHead);
        }
        return 0;
    }
}


int parseDictionnary(struct dictionnary *pDict, const char *pDictFile)
{
    /* Load and parse a file as dictionnary
        *pDict = Dictionnary structure
        *pDictFile = Path to the file to open as dictionnary
        returns O if OK, else returns 1
     */
    FILE *fDictFile = NULL;
    char pLine[BIGGEST_DICT_WORD+1];
    int iStartPos, iLength;
    unsigned short bFlags; /* boolean flags */
    struct dictionnary_block *pCurrentBlock;

#ifdef SHOW_DEBUG
    fprintf(stderr,"Starting parseDictionnary function\n");
#endif

    if (pDictFile == NULL)
    {
        fprintf(stderr,"parseDictionnary doesn't accept NULL parameter(s).\n");
        return 1;
    }

    fDictFile = fopen(pDictFile, "r");
    if (fDictFile == NULL)
    {
        perror(pDictFile);
        fprintf(stderr,"The given dictfile is missing\n");
        return 1;
    }

    pDict->pHead = (struct dictionnary_block*) malloc(sizeof(struct dictionnary_block));
    if (pDict->pHead == NULL)
    {
        fprintf(stderr,"Not enough memory\n");
        return 1;
    }
    pCurrentBlock=pDict->pHead;
    pCurrentBlock->iMaxMemory=0;
    pCurrentBlock->iUsedMemory=0;
    pCurrentBlock->pWordList=NULL;
    bFlags=0x02;
    while(!feof(fDictFile))
    {
        if (fgets(pLine,BIGGEST_DICT_WORD,fDictFile) != NULL)
        {
            if (strstr(pLine,"\n") == NULL)
            {
                /* The line is not complete, so the word will be ignored
                    Future readings will be ignored until the next \n*/
                bFlags=bFlags & ~(1u << 1);
#ifdef SHOW_DEBUG
                fprintf(stderr,"Too long line starting with: \"%s\"\n", pLine);
#endif
            }
            else
            {
                if (!(bFlags & (1u << 1)))
                {
                    /* Next line will be treated */
                    bFlags=bFlags | (1u << 1);
                }
                else
                {
                    /* Trim left then right then filter */
                    iStartPos=0;
                    while((pLine[iStartPos] <= 0x20) && (iStartPos < BIGGEST_DICT_WORD))
                        iStartPos++;

                    iLength=strlen(pLine);
                    while((pLine[iLength] <= 0x20) && (iLength > iStartPos))
                        iLength--;
                    iLength=iLength-iStartPos+1;

                    if (iLength >= 0)
                    {
                        bFlags=bFlags | (1u << 0);
                        pLine[iStartPos+iLength]='\0';
                        if (pLine[iStartPos] == '#')
                        {
                            bFlags=bFlags & ~(1u << 0);
                        }
                        else
                        {
                            if (strstr(&pLine[iStartPos]," ") != NULL)
                            {
                                /* More than one word in the line */
                                bFlags=bFlags & ~(1u << 0);
                            }
                        }

                        if (bFlags & (1u << 0))
                        {
                            if ((pCurrentBlock->iUsedMemory+iLength+1) > pCurrentBlock->iMaxMemory)
                            {
                                if (pCurrentBlock->pWordList != NULL)
                                {
#ifdef SHOW_DEBUG
                                    fprintf(stderr,"Allocating a new dictionnary block\n");
#endif
                                    pCurrentBlock->pNext = (struct dictionnary_block*) malloc(sizeof(struct dictionnary_block));
                                    if (pCurrentBlock->pNext == NULL)
                                    {
                                        fprintf(stderr,"Not enough memory\n");
                                        return 1;
                                    }
                                    pCurrentBlock=pCurrentBlock->pNext;
                                    pCurrentBlock->iMaxMemory=0;
                                    pCurrentBlock->iUsedMemory=0;
                                    pCurrentBlock->pWordList=NULL;
                                }
                                if (pCurrentBlock->pWordList == NULL)
                                {
#ifdef SHOW_DEBUG
                                    fprintf(stderr,"Allocating a new wordlist of %i chars\n", SIZEOF_DICTIONNARY_WORDLIST);
#endif
                                    pCurrentBlock->pWordList = (char*) malloc(SIZEOF_DICTIONNARY_WORDLIST * sizeof(char));
                                    if (pCurrentBlock->pWordList == NULL)
                                    {
                                        fprintf(stderr,"Not enough memory\n");
                                        return 1;
                                    }
                                    pCurrentBlock->pNext=NULL;
                                    pCurrentBlock->iMaxMemory=SIZEOF_DICTIONNARY_WORDLIST;
                                    pCurrentBlock->pWordList[0]='|';
                                    pCurrentBlock->pWordList[1]='\0';
                                    pCurrentBlock->iUsedMemory=1;
                                }
                            }
                            if ((iLength+2) > pCurrentBlock->iMaxMemory)
                            {
                                /* Problem is that the current word is bigger than the max memory */
#ifdef SHOW_DEBUG
                                fprintf(stderr,"Word \"%*s\" is bigger than the max memory\n", iLength, &pLine[iStartPos]);
#endif
                            }
                            else
                            {
                                strncat(pCurrentBlock->pWordList,&pLine[iStartPos],iLength);
                                strcat(pCurrentBlock->pWordList,"|");
                                pCurrentBlock->iUsedMemory=pCurrentBlock->iUsedMemory+iLength+1;
                                pDict->iNumberOfWord++;
                                if (pDict->iBiggestWord<strlen(pLine)) pDict->iBiggestWord=iLength;
                            }
                        }
                    }
                }
            }
        }
    }
    fclose(fDictFile);
    if (pDict->iNumberOfWord == 0)
    {
        fprintf(stderr,"Dictionnary file is empty, there is nothing to do\n");
        return 1;
    }
    else
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Ending good parseDictionnary function\n");
        fprintf(stderr,"  pDict->iNumberOfWord=%d\n",pDict->iNumberOfWord);
        fprintf(stderr,"  pDict->iBiggestWord =%d\n",pDict->iBiggestWord);
        fprintf(stderr,"  wordlist=\n");
        pCurrentBlock=pDict->pHead;
        while (pCurrentBlock!=NULL)
        {
            fprintf(stderr,"%s\n",pCurrentBlock->pWordList);
            pCurrentBlock=pCurrentBlock->pNext;
        }
#endif
        return 0;
    }
}

int grepWord(const struct dictionnary *pDict, const char *pWord, const int bFreeMemory)
{
    /* Check the existence of a word in the dictionnary
        *pDict = Dictionnary to search in
        *pWord = Word to search for
        returns 1 if not found, else returns 0
     */
    static char *sSearched = NULL;

    if (bFreeMemory)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Free memory used to store searched word\n");
#endif
        if (sSearched != NULL)
        {
            free(sSearched);
        }
        return 0;
    }
    if ((pDict == NULL) || (pWord == NULL))
    {
        fprintf(stderr,"grepWord doesn't accept NULL parameter(s).\n");
        return 1;
    }
    if (pWord[0] == '\0')
    {
        return 1;
    }
    if (sSearched == NULL)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Allocating memory of %d char(s) to store searched word\n",(pDict->iBiggestWord+2));
#endif
        sSearched= (char *) malloc((pDict->iBiggestWord+3)*sizeof(char));
        if (sSearched == NULL)
        {
            fprintf(stderr,"Not enough memory\n");
            return 1;
        }
    }
    sSearched[0]='|';
    sSearched[1]='\0';
    strcat(sSearched,pWord);
    strcat(sSearched,"|");

    return grepWordInBlocks(pDict->pHead, sSearched);

}

int grepWordInBlocks(const struct dictionnary_block *pBlock, const char *sSearched)
{
    /* Check the existence of a word in a dictionnary block and the next one
        *pBlock = Dictionnary block to search in
        *pSearched = string to search for
        returns 1 if not found, else returns 0
     */
    if (pBlock == NULL)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Well, no more block\n");
#endif
        return 1;
    }
    if (pBlock->pWordList == NULL)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Well, no wordlist here\n");
#endif
        return 1;
    }
    if (strstr(pBlock->pWordList,sSearched) != NULL)
    {
        /* Found the word */
        return 0;
    }
    else
    {
        if (pBlock->pNext != NULL)
        {
#ifdef SHOW_DEBUG
            fprintf(stderr,"Word not found in this block, so looking in the next one\n");
#endif
            return grepWordInBlocks(pBlock->pNext, sSearched);
        }
        else
        {
#ifdef SHOW_DEBUG
            fprintf(stderr,"Word not found in all block(s)\n");
#endif
            return 1;
        }
    }
}

int readInput(const struct dictionnary *pDict, FILE *fInput, const int bFreeMemory)
{
    /* Read word by word the input and grep it into stdout
        *pDict = Dictionnary to search in
        *fInput = Input data
        bFreeMemory = 1 to free memory used by pWord
    */
    static char *pWord = NULL;
    int iLength;
    char cCar;
    fpos_t StartPos, CurrentPos;
    int iLineIsGrep;

    if (bFreeMemory)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Free memory used to store input word\n");
#endif
        if (pWord != NULL)
        {
            free(pWord);
        }
        return 0;
    }
    if ((fInput == NULL) || (pDict == NULL))
    {
        fprintf(stderr,"readInput doesn't accept NULL parameter(s).\n");
        return 1;
    }
    if (pWord == NULL)
    {
#ifdef SHOW_DEBUG
        fprintf(stderr,"Allocating memory of %d char(s) to store input word\n",(pDict->iBiggestWord+1));
#endif
        pWord= (char *) calloc((pDict->iBiggestWord+2),sizeof(char));
        if (pWord == NULL)
        {
            fprintf(stderr,"Not enough memory\n");
            return 1;
        }
    }
    iLength=0;
    iLineIsGrep=0;
    fgetpos(fInput,&StartPos);
    while((cCar=fgetc(fInput)) != EOF)
    {
        if (iLineIsGrep)
        {
            /* print the characters until the end of the current line */

            if ((cCar == '\n') || (cCar == '\r'))
            {
                fgetpos(fInput,&StartPos);
                iLineIsGrep=0;
#ifdef SHOW_DEBUG
                fprintf(stderr,"\"\nWe reach the end of the line\n");
#else
                fputc('\n',stdout);
#endif
            }
            else
            {
#ifdef SHOW_DEBUG
                fputc(cCar,stderr);
#else
                fputc(cCar,stdout);
#endif
            }
        }
        else
        {
            if (cCar <= 0x20)
            {
                /* We got a full word */
                pWord[iLength]='\0';
                if (iLength <= pDict->iBiggestWord)
                {
                    /* Bigger than the biggest word of the dictionnary */
                    iLineIsGrep=!grepWord(pDict,pWord,0);
                    if (iLineIsGrep)
                    {
                        /* print the beginning of the line */
                        fgetpos(fInput,&CurrentPos);
                        fsetpos(fInput,&StartPos);
#ifdef SHOW_DEBUG
                        fprintf(stderr,"Catch the word \"%s\" so print the beginning:\n  \"",pWord);
#endif
                        while(StartPos.__pos<CurrentPos.__pos)
                        {
                            cCar=fgetc(fInput);
#ifdef SHOW_DEBUG
                            fputc(cCar,stderr);
#else
                            fputc(cCar,stdout);
#endif
                            fgetpos(fInput,&StartPos);
                        }
#ifdef SHOW_DEBUG
                        fprintf(stderr,"\"\nNext is printing until the end of the line:\n  \"");
#endif
                    }
                }
                else
                {
#ifdef SHOW_DEBUG
                    fprintf(stderr,"Word starting with \"%s\" is too long, so is ignored\n",pWord);
#endif
                }
                iLength=0;
                if ((cCar == '\n') || (cCar == '\r'))
                {
                    fgetpos(fInput,&StartPos);
                }
            }
            else
            {
                if (iLength <= pDict->iBiggestWord)
                {
                    pWord[iLength]=cCar;
                    iLength++;
                }
            }
        }
    }
    return 0;
}
