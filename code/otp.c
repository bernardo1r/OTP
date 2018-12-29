#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define TAM 20
#define ERRFILES printf("ERROR OPENING FILES!\n");
#define ERRINFILES if (argc == 2){printf("NO INPUT FILES DETECTED!\n");return 1;}
#define ATIME printf("%dms!\n", end-start);

/* Fazer função generica para abrir arquivos */


long inputSize;

char *keyFileName, *inputFileName; //fixar gambiarra

pthread_t *createArr(int size)
{
    pthread_t *ids = malloc(sizeof(pthread_t)*size);
    if (ids == NULL)
    {
        printf("FAILED!!!\n");
        exit(1);
    }
    return ids;
}

FILE **criaListaArq(int size)
{
    FILE **lista = malloc(size*sizeof(FILE *));
    return lista;
}

void abreArqsLeitura(FILE **listaArq, char **argv, int size)
{
    int i;
    int j=2;
    for (i=0; i < size; i++)
    {
        listaArq[i] = fopen(argv[j++], "rb");
        if (listaArq[i] == NULL)
        {
            ERRFILES
            exit(1);
        }
    }
}

FILE *abreArqEscrita(char *name)
{
    FILE *p = fopen(name, "wb");
    if (p == NULL)
    {
        ERRFILES
        exit(1);
    }
    return p;
}

FILE *abreArqLeitura(char *name)
{
    FILE *p = fopen(name, "rb");
    if (p == NULL)
    {
        ERRFILES
        exit(1);
    }
    return p;
}

void fechaArqs(FILE **listaArq, int size)
{
    int i;
    for (i=0; i < size; i++)
        fclose(listaArq[i]);
}

void leArgv(char **argv, int argc)
{
    int i;
    for (i = 2; i < argc; i++)
        printf("%s\n", argv[i]);
}

void *xorThreading(void* arg)
{
    FILE *key, *input, *output;
    int cinput, ckey, xor, *multiplier;
    long frwd, stop, i;
    multiplier = (int*) arg;
    stop = 0; //fixar a gambiarra
    frwd = inputSize/TAM;
    if ((*multiplier) == TAM - 1)
        stop = inputSize % TAM;
    key = abreArqLeitura(keyFileName);
    input = abreArqLeitura(inputFileName);
    output = fopen("output", "rb+");
    if (output == NULL)
    {
        ERRFILES
        exit(1);
    }
    fseek(key, frwd*(*multiplier), 0);
    fseek(input, frwd*(*multiplier), 0);
    fseek(output, frwd*(*multiplier), 0);
    for (i=0; i < frwd + stop; i++)
    {
        cinput = fgetc(input);
        ckey = fgetc(key);
        xor = cinput^ckey;
        fputc(xor, output);
    }
    fclose(key);
    fclose(input);
    fclose(output);
}

int *makeIds(int size)
{
    int i;
    int *ID = malloc(sizeof(int)*size);
    if (ID == NULL)
    {
        printf("ALLOCATION FAILED!\n");
        exit(1);
    }
    for (i=0; i < size; i++)
        ID[i] = i;
    return ID;
}

int main(int argc, char *argv[])
{
    int char1, i, cinput, ckey, xor;
    int size = argc-2;
    char question, buff;
    FILE **listaArq, *key, *input, *output;
    clock_t start, end;
    pthread_t *ids;
    int *ID;

    //error and help section

    if (argc <= 1)
    {
        printf("NO KEY DETECTED!\n");
        return 1;
    }
    if (strcmp(argv[1], "help")==0)
    {
        printf("This program creates an file by the OTP method\n\n");
        printf("KEYS:\n");
        printf("-a  \tAppends files and generate a keyfile\n");
        printf("Key syntax: otp -a <inputfiles>\n\n");
        printf("-otp\tApplies one time pad encryption to the input file\n");
        printf("Key syntax: otp -otp <keyfile> <inputfile>\n\n");
        return 1;
    } else if ((strcmp(argv[1], "-a"))== 0)
    {
    //append keyfile section

        ERRINFILES
        listaArq = criaListaArq(size);
        abreArqsLeitura(listaArq, argv, size);
        printf("\nFILES:\n");
        leArgv(argv, argc);
        printf("Number of files: %d\n", size);
        key = abreArqEscrita("key");
        start = clock();
        for(i=0; i< size; i++)      /*append one file at a time*/
        {
            while ((char1=fgetc(listaArq[i]))!= EOF)
                fputc(char1,key);
        }
        end = clock();
        fechaArqs(listaArq, size);
        fclose(key);
        ATIME
        printf("\nKey created successfully!\n");
        return 0;

    } else if ((strcmp(argv[1], "-otp"))== 0)
    {
    //OTP encryption section
        ERRINFILES
        if (argc < 4)
        {
            printf("TOO FEW ARGUMENTS!\n");
            return 1;
        }
        if (argc > 4)
        {
            printf("TOO MANY ARGUMENTS!\n");
            return 1;
        }
        if ((strcmp(argv[3],"output"))== 0)
        {
            printf("INPUT FILE WITH THE SAME NAME OF OUTPUT FILE!\n");
            return 1;
        }
        ID = makeIds(TAM);
        input = abreArqLeitura(argv[3]);
        fseek(input, 0, SEEK_END);
        inputSize = ftell(input);
        output = abreArqEscrita("output");
        fseek(output, inputSize-1, 0);
        fwrite(&buff, 1, 1, output);
        fclose(output);
        fclose(input);
        ids = createArr(TAM);
        inputFileName = argv[3];
        keyFileName = argv[2];
        start = clock();
        for (i=0; i < TAM; i++)
            pthread_create(&ids[i], NULL, xorThreading, &ID[i]);
        for (i=0; i< TAM; i++)
            pthread_join(ids[i], NULL);
        end = clock();
        ATIME
        printf("\nOutput file created successfully!\n");
    } else
    {
    //wrong key section

        printf("INVALID KEY!\n");
    }
    return 1;
}
