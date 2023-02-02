#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct link link;

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

struct fun_desc {
    char *name;
    void (*fun)();
};

struct link {
    link *nextVirus;
    virus *vir;
};

link * listHead = NULL;
char *fileName;


void PrintHex(unsigned char *buffer, size_t length, FILE * output){
    for (int i=0; i<length; i++)
        fprintf(output,"%02hhX ", buffer[i]);
    fprintf(output,"\n\n");
}

virus* readVirus(FILE* input){
    struct virus *v=(virus*)malloc(sizeof(virus));
    if(fread(v,1,18,input)!=0){
        v->sig = (unsigned char*)malloc(v->SigSize);
        fread(v->sig,1,v->SigSize,input);
        return v;
    } else
    {
        return NULL;
    }
}

void printVirus(virus* virus, FILE* output){
    fprintf(output , "%s%s\n" ,"Virus name: ",virus->virusName);
    fprintf(output , "%s%d\n" , "Virus size: ",virus->SigSize);
    fprintf(output , "%s\n" ,"signature:");
    PrintHex(virus->sig,virus->SigSize,output);
}


void list_print(link *virus_list, FILE* input){
    if (virus_list==NULL) {
        fprintf(stdout, "Invalid input // list_print");
        return;
    }
    link * curr = virus_list;
    do{
        printVirus(curr->vir,input);
        curr = curr->nextVirus;
    } while (curr != NULL);
}


link* list_append(link* virus_list, virus* data){

    link *newLink = malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = NULL;
    link *curr = virus_list;

    if (virus_list == NULL)
    {
        return newLink;
    }

    while (curr->nextVirus != NULL)
    {
        curr = curr->nextVirus;
    }
    curr->nextVirus = newLink;

    return virus_list;
}

void list_free(link *virus_list) {
    link *currLink = virus_list;
    if (currLink != NULL) {
        list_free(currLink->nextVirus);
        free(currLink->vir->sig);
        free(currLink->vir);
        free(currLink);
    }
}
void quit() {
    list_free(listHead);
    exit(0);

}

link* load_list(FILE* file) {
    link *head = NULL;
    int length;
    fseek(file, 0L, SEEK_END);
    length = ftell(file);
    rewind(file);
    char buffer[4];
    fread(&buffer, 1, 4, file);
    int bytes = 4;
    while (bytes < length) {
        virus *nextVirus = readVirus(file);  //to free??
        head = list_append(head, nextVirus);
        bytes = bytes + 18 + nextVirus->SigSize;
    }
    fclose(file);
    return head;
}

void Load_Signatures(FILE *file1){
    char* fileName=NULL;
    char buf[BUFSIZ];
    printf("Enter signature file name: \n");
    fgets(buf,sizeof(buf),stdin);
    sscanf(buf,"%ms",&fileName);
    FILE* file = fopen(fileName,"rb");
    if(file==NULL){
        fprintf(stderr,"Invalid file\n");
        exit(0);
    }
    free(fileName);
    listHead = load_list(file);
}

void print_signatures(FILE *file){
    if(listHead == NULL){
        printf("%s","No Signature file was inserted");
    }
    else{
        list_print(listHead,stdout);

    }
}

long isVirus(char *buffer, virus *curr_virus, unsigned long size) {
    int location = 0;
    while(location < size)
    {
        if(memcmp(curr_virus->sig, buffer+location, curr_virus->SigSize) == 0)
            return location;

        location = location + 1;
    }
    return -1;
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link *curr = virus_list;
    while(curr != NULL)
    {
        long location = isVirus(buffer, curr->vir, size);
        if(location != -1)
        {
            fprintf(stdout, "Starting byte location: %ld\n", location);
            fprintf(stdout, "Virus name: %s\n", curr->vir->virusName);
            fprintf(stdout, "Signature size: %d\n\n", curr->vir->SigSize);

        }
        if (curr->nextVirus == NULL)
        {
            curr = NULL;
            break;
        }

        curr = curr->nextVirus;
    }
}

void detect_viruses(FILE *file){
    FILE *inputFile;
    char buffer[10000];
    inputFile=fopen(fileName, "r");

    if (inputFile != NULL && listHead != NULL)
    {
        size_t len = fread(buffer, 1, 10000, inputFile);
        detect_virus(buffer,len,listHead);
        fclose(inputFile);
    }
    else
        fprintf(stderr,"Either a file or a signature file was inserted\n");
}


void kill_virus(int signitureOffset, int signitureSize){
    FILE* infectedFile;
    infectedFile = fopen(fileName,"r+b");
    if(infectedFile){
        fseek(infectedFile, signitureOffset+1, SEEK_SET );
        unsigned char newByte = 0x90;
        fwrite(&newByte,1,1,infectedFile);
        fclose(infectedFile);
    }
    else
        fprintf(stderr,"ERROR: There is no file with this name\n");
}

void fixFile() {
    unsigned short byteLocation;
    unsigned short signatureSize;
    if (fileName == NULL) {
        fprintf(stderr, "no file was provided\n");
        exit(0);
    }
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        fprintf(stderr, "file is empty\n");
        exit(0);
    }
    printf("Enter the starting byte and virus size:\n");
    scanf("%hd %hd", &byteLocation, &signatureSize);
    kill_virus(byteLocation, signatureSize);
    fclose(file);

}



int main(int argc, char **argv){
    FILE* suspectedfile=NULL;
    int option=-1;
    struct fun_desc menu [] =
            {{ "Load signatures", Load_Signatures}, { "Print signatures", print_signatures },
             { "Detect viruses", detect_viruses }, { "Fix File", fixFile }, { "Quit", quit }, { NULL, NULL } };

    int bounds=(sizeof(menu)/sizeof(menu[0]))-1;
    if (argc>1){
        fileName = argv[1];
        suspectedfile = fopen(fileName,"rb");
        if(suspectedfile ==NULL){
            fprintf(stderr,"Suspected file is empty\n");
            quit();
        }
        fclose(suspectedfile);
    }

    while(1)
    {
        printf("Please choose a function:\n");
        for (int i=0; i<bounds; i++){
            printf("%d) %s\n", i+1, menu[i].name);
        }
        //fseek(stdin,0,SEEK_END);//for clean the buffer
        printf("Option: ");
        scanf("%d",&option);

        if ((option<0) | (option>bounds-1)){
            printf("Not within bounds\n");
            quit();
        }
        else{
            printf("Within bounds\n\n");
            fgetc(stdin);
            menu[option-1].fun(suspectedfile);
        }
    }
    return 0;
}

