#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define NAME_LEN 128
#define BUF_SZ 10000

int debug = 0;

typedef struct {
    char debug_mode;
    char file_name[NAME_LEN];
    int unit_size;
    unsigned char mem_buf[BUF_SZ];
    size_t mem_count;
    int mode;
} state;

typedef struct {
    char *name;
    void (*fun)(state*);
}fun_desc;


void toggleDebugMode(state* s){
    if(s->debug_mode==0){
        printf("Debug flag now on\n");
        s->debug_mode=1;
    }
    else{
        printf("Debug flag now off\n");
        s->debug_mode=0;
    }
}


void ToggleDisplayMode(state* s){
    if(s->mode==0){
        printf("Display flag now on, hexadecimal representation\n");
        s->mode=1;
    }
    else{
        printf("Display flag now off, decimal representation\n");
        s->mode=0;
    }
}


void setFileName(state* s){
    printf("Enter new File Name: \n");
    char filename[100];

    fgets(filename , 128 , stdin);
    memcpy(s->file_name , filename , sizeof(filename));
    s->file_name[strcspn(s->file_name , "\n")] = 0;

    if(s->debug_mode == 1){
        printf("Debug: file name set to %s\n", filename);
    }
}

void setUnitSize (state* s) {
    printf("Enter new Unit Size: \n");
    int unitSize;
    scanf("%d", &unitSize);
    if (unitSize == 4 || unitSize == 2 || unitSize ==1)
    {
        s->unit_size = unitSize;
        if (s->debug_mode == 1)
        {
            printf("%s%d\n" , "Debug: size set to: " , unitSize);
        }
    }
    else
        fprintf(stderr , "%s\n" , "invalid number");
}

void quit (state* s) {
    if (debug) { printf("quitting..\n");}
    free(s);
    exit(0);
}

void printMenu(fun_desc menu[], state* s){
    for (int i = 0; i <= 8; i++)
    {
        printf("%d%c%s\n" , i , ')' , menu[i].name);
    }
    printf("\n");
    if (s->debug_mode == 1){
        printf("%s%d%s%s%s%d" ,"unit size: ", s->unit_size ,
               " file name: ", s->file_name ,
               " mem_count: ", s->mem_count);
        printf("\n");

    }
}


void loadIntoMemory(state* s){
    printf("%s\n" , s->file_name);
    char *input[128];
    int location;
    int size = 0;
    if ((char *) s->file_name[0] == "")
    {
        fprintf(stderr , "%s\n" , "file name is empty");
    }

    FILE *reader  = fopen(s->file_name ,"r+");
    if ( reader == NULL)
    {
        fprintf(stderr , "%s\n" , "cant open the file");
        return;
    }
    printf("%s\n" , "please enter <location>  <length>");
    fgets((char *) input, NAME_LEN , stdin);
    printf("%d\n" , reader);
    sscanf((const char *) input, "%x%d" , &location , &size);
    printf("0x%x\n%d\n" , location , size);
    fseek(reader , location , SEEK_SET);
    fread(s->mem_buf , size , s->unit_size ,  reader);
    s->mem_count = size*s->unit_size;
    printf("%s%d%s\n" , "loaded " , size , " units into memory");
}

char* unit_to_format_deci(int unit) {
    static char* formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    return formats[unit-1];
}

char* unit_to_format_hexa(int unit) {
    static char* formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    return formats[unit-1];
}


void read_units_to_memory(FILE* input, char* buffer, int count, int size) {
    fread(buffer, size, count, input);

}

/* Prints the buffer to screen by converting it to text with printf */
void print_units(FILE* output, const char* buffer, int count,bool decimal, int size) {
    const char* end = buffer + size*count;
    while (buffer < end) {
        //print ints
        int var = *((int*)(buffer));
        if(decimal){
            printf(unit_to_format_deci(size),var);
        }
        else{
            printf(unit_to_format_hexa(size),var);
        }
        buffer += size;
    }
}


void memoryDeslplay(state* s){

    char *input[128];
    int address;
    int units;

    printf("%s\n" , "please enter <address>  <length>");
    fgets((char *) input, NAME_LEN , stdin);
    sscanf((const char *) input, "%x%d" , &address , &units);


    if (s->mode == 0) {
        printf("%s\n%s\n" , "Decimal" , "==========");
        if (address == 0){
            print_units(stdout, (const char *) &(s->mem_buf), units, true, s->unit_size );
        }
        else {
            print_units(stdout, (const char *) &address, units, true, s->unit_size );

        }
    }
    if (s->mode == 1){
        printf("%s\n%s\n" , "Hexadecimal" , "==========");
        if (address == 0){
            print_units(stdout, (const char *) &(s->mem_buf), units, false, s->unit_size );
        }
        else {
            print_units(stdout, (char *) &address, units, false, s->unit_size );

        }
    }
}
void saveIntoFile(state* s){
    fprintf(stdout,"Please enter <source-address> <target-location> <length>:\n");
    int sourceAddress = 0;
    int targetLocation = 0;
    int length = 0;
    fscanf(stdin,"%x %x %d",&sourceAddress,&targetLocation,&length);
    FILE* file = fopen(s->file_name,"r+");
    if(file==NULL) {
        printf("Error open file\n");
        return;
    }
    fseek(file,0,SEEK_END);
    if(targetLocation > ftell(file)){
        perror("target location is greater then file size");
        return;
    }
    fseek(file,0,SEEK_SET); //back to start of file
    fseek(file,targetLocation,SEEK_SET);
    if(sourceAddress==0){
        fwrite(&(s->mem_buf),s->unit_size,length,file);
    }
    else{
        fwrite(&(sourceAddress),s->unit_size,length,file);
    }
    if (s->debug_mode == 1){
        printf("%s%d%s%x%s%x" ,"writing: ", length , " bytes from source address: ", sourceAddress , " to target address:" , targetLocation);
        printf("\n");

    }
    fclose(file);
}
void memoryModify(state* s) {
    char *input[128];
    int address;
    int val;
    printf("%s\n", "please enter <location> <val> ");
    fgets((char *) input, NAME_LEN, stdin);
    sscanf((const char *) input, "%x%x", &address, &val);
    memcpy(&s->mem_buf[address], &val, s->unit_size);
    if (s->debug_mode == 1) {
        printf("%s%d%s%x", "overwriting: ", s->unit_size, " bytes with value: ", val);
        printf("\n");
    }
}


int main(int argc, char **argv){
    state* s = malloc (sizeof(state));
    fun_desc menu[] = {{"Toggle Debut Mode" , toggleDebugMode} ,
                       {"Set File Name" , setFileName} ,
                       {"Set Unit Size" , setUnitSize} ,
                       {"Load Into Memory" , loadIntoMemory } ,
                       {"Toggle Display Mode" , ToggleDisplayMode } ,
                       {"Memory Display" , memoryDeslplay} ,
                       {"Save Into File" , saveIntoFile },
                       {"Memory Modify" , memoryModify},
                       {"quit" , quit},
                       { NULL , NULL}};

    int length = sizeof(menu) / sizeof(menu[0]);


    while (1) {
        printMenu(menu , s);
        int option;
        scanf("%d", &option);
        printf("%s%d\n" , "Option : " , option);
        if (option < 0 || option > length-1)
        {
            printf("%s" , "Not within bounds");
            exit(1);
        }
        else
        {
            printf("%s\n" , "Within bounds");
            //printf("%d\n" , num);
        }

        fgetc(stdin);
        menu[option].fun(s);

        printf("Done.\n\n");
    }


}
