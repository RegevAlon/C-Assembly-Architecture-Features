#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#define  NAME_LEN  128
#define  BUF_SZ    10000

#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdbool.h>

struct stat fd_stat;
void *map_start;
Elf32_Ehdr *header;
int currFd = -1;

typedef struct {
    char debug_mode;
    char file_name[NAME_LEN];
    int unit_size;
    unsigned char mem_buf[BUF_SZ];
    size_t mem_count;
    int mode;
    int mapped;
} state;

typedef struct {
    char *name;
    void (*fun)(state*);
}fun_desc;

char* sectionType(int type) {

    switch(type){
        case SHT_NULL : return "NULL";
        case SHT_PROGBITS : return "PROGBITS";
        case SHT_SYMTAB : return "SYMTAB";
        case SHT_STRTAB : return "STRTAB";
        case SHT_RELA : return "RELA";
        case SHT_HASH : return "HASH";
        case SHT_DYNAMIC : return "DYNAMIC";
        case SHT_NOTE : return "NOTE";
        case SHT_NOBITS : return "NOBITS";
        case SHT_REL : return "REL";
        case SHT_SHLIB : return "SHLIB";
        case SHT_DYNSYM : return "DYNSYM";
        case SHT_INIT_ARRAY : return "INIT_ARRAY";
        case SHT_FINI_ARRAY : return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY : return "PREINIT_ARRAY";
        case SHT_GROUP : return "GROUP";
        case SHT_SYMTAB_SHNDX : return "SYMTAB_SHNDX";
        case SHT_NUM : return "NUM";
        case SHT_LOOS : return "LOOS";
        case SHT_GNU_LIBLIST : return "LIBLIST";
        case SHT_CHECKSUM : return "CHECKSUM";
        case SHT_LOSUNW : return "LOSUNW";
        case SHT_SUNW_COMDAT : return "COMDAT";
        case SHT_SUNW_syminfo : return "SYMINFO";
        case SHT_GNU_verdef : return "VERDRF";
        case SHT_GNU_verneed : return "VERNEED";
        case SHT_GNU_versym : return "VERSYM";
        case SHT_LOPROC : return "LOPROC";
        case SHT_HIPROC : return "HIPROC";
        case SHT_LOUSER : return "LOUSER";
        case SHT_HIUSER : return "HIUSER";
        case SHT_GNU_HASH : return "GNU_HASH";
        default: return "";

    }
}

void printMenu(fun_desc menu[],state* s){

    for (int i = 0; i <= 5; i++)
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

bool LoadElf(state* s){
    char filename[NAME_LEN];
    printf("%s\n" , "please enter ELF file name");
    fscanf(stdin,"%s",filename);

    if (s->debug_mode == 1)
    {
        printf("%s%s\n" , "Debug: file name set to: " , filename);
    }
    if((currFd  = open(filename ,O_RDWR, 0666))  <= 0){
        perror("cant open the file");
        exit(-1);
    }
    if(fstat(currFd, &fd_stat) != 0) {
        perror("stat failed");
        exit(-1);
    }
    map_start = mmap(0 , fd_stat.st_size , PROT_READ | PROT_WRITE, MAP_SHARED , currFd , 0 );
    if (map_start == MAP_FAILED)
    {
        printf("%d\n" ,currFd);
        perror("error in mmap");
        exit(-1);
    }
    s->mapped = 1;
    return true;
}

void ExamineELFFile(state* s){
    if (LoadElf(s))
    {
        header = (Elf32_Ehdr*)map_start;
        printf("Bytes 1,2,3 of the magic: %x %x %x\n",header->e_ident[0],header->e_ident[1], header->e_ident[2]);
        printf("The data encoding scheme of the object file: %d\n", header->e_ident[5]);
        printf("Entry point: 0x%X\n", header->e_entry);
        printf("The file offset in which the section header table resides: %d\n", header->e_shoff);
        printf("The number of section header entries: %d\n", header->e_shnum);
        printf("The size of each section header entry: %d\n", header->e_shentsize);
        printf("The file offset in which the program header table resides: %d\n", header->e_phoff);
        printf("The number of program header entries: %d\n", header->e_phnum);
        printf("The size of each program header entry: %d\n", header->e_phentsize);
    }


}
void PrintSectionNames(state* s){
    if (s->mapped == 0)
    {
        perror("File is not mapped\n");
        exit(-1);
    }
    Elf32_Shdr *section_start = map_start + header->e_shoff;
    Elf32_Shdr * section_name_start = section_start + header->e_shstrndx;
    char* section_names_start_pointer = map_start + section_name_start->sh_offset;
    char * section_name;
    printf("[index]\t\t      Name    Addr\t    Off\t\tSize\t\tType\n");
    for(int i = 0; i < header->e_shnum; i++)
    {
        section_name = section_names_start_pointer + section_start[i].sh_name; // section name
        printf("[%2d] %21s %7x\t%7x\t   %7x\t  %10s\n",
               i, section_name, section_start[i].sh_addr, section_start[i].sh_offset,
               section_start[i].sh_size, sectionType(section_start[i].sh_type));
    }
    if(s->debug_mode == 1){
        printf("Debug: Section header string table index: %d\n",header->e_shstrndx);
    }
}

void SymbolPrinter(Elf32_Sym symbol, char* name, char* pointer,int index){
    printf("%2d: %09x\t %9d\t %9s\t %9s\n",
           index, symbol.st_value, symbol.st_shndx, name,
           pointer + symbol.st_name);
}

char* getSymbolName(Elf32_Sym symbol,Elf32_Shdr *section, char* name, char* pointer){
    if(symbol.st_shndx == SHN_UNDEF)
        name = "UND";
    else if(symbol.st_shndx == SHN_ABS)
        name = "ABS";
    else
        name = pointer + section[symbol.st_shndx].sh_name;

    return name;
}

void PrintSymbols(state* s){
    if(s->mapped == 0) {
        perror("File is not mapped\n");
        exit(0);
    }
    char* section_name;
    char *symbol_table_names_pointer;
    Elf32_Shdr *run_header = (Elf32_Shdr *)(map_start + header->e_shoff);

    Elf32_Sym* symbol_table;
    Elf32_Shdr* symbol_table_names;
    Elf32_Shdr* section_start =  map_start + header->e_shoff;
    Elf32_Shdr* section_name_start = section_start + header->e_shstrndx;
    char* section_names_start_pointer = map_start + section_name_start->sh_offset;

    int i=0;
    while((i < header->e_shnum)) {
        if ((section_start[i].sh_type == SHT_SYMTAB) || (section_start[i].sh_type == SHT_DYNSYM)) {
            symbol_table = map_start + section_start[i].sh_offset; // get the symbol table of this section
            symbol_table_names = section_start + section_start[i].sh_link;
            symbol_table_names_pointer = map_start + symbol_table_names->sh_offset;
            if (s->debug_mode == 1){
                printf("Debug: Size of symbol table is: %x\n", run_header->sh_size);
                printf("Debug: Numbers of symbols: %d\n", run_header->sh_size / sizeof(Elf32_Sym));
            }
            printf("index\tvalue\tsection_index\tsection_name\tsymbol_name\n");
            for (int j = 0; j < section_start[i].sh_size / sizeof(Elf32_Sym); j++) {
                Elf32_Sym currentSymbol = symbol_table[j];

                section_name = getSymbolName(currentSymbol,section_start,section_name,section_names_start_pointer);
                SymbolPrinter(currentSymbol,section_name,symbol_table_names_pointer,j);

            }
            printf("\n");
        }
        run_header++;
        i++;
    }
}
void TablePrinter(Elf32_Rel relocation, Elf32_Sym* table, char* pointer){

    printf("%08x\t%08x\t%4d\t%08x\t%8s\n",
           relocation.r_offset, relocation.r_info,
           ELF32_R_TYPE(relocation.r_info),
           table[ELF32_R_SYM(relocation.r_info)].st_value, // get the symbol's value (relocation entry) and find it in the dynsym table
           (char *)(pointer + table[ELF32_R_SYM(relocation.r_info)].st_name));
}


void RelocationTables(state* s){
    if( currFd == -1) {
        perror("File is not mapped\n");
        exit(0);
    }
    Elf32_Shdr* section_start = (Elf32_Shdr *)(map_start + header->e_shoff);
    Elf32_Rel* relocationTables;
    Elf32_Sym* dynsymTable;
    Elf32_Shdr* dynsymNames;
    Elf32_Rel currentRelocation;
    for(int i = 0 ; i < header->e_shnum; i++) {

        if (section_start[i].sh_type == SHT_REL) { // is Relocation entries section
            int index = section_start[i].sh_link; // the section that this symbol belongs too
            dynsymTable = map_start + section_start[index].sh_offset; // pointer to the section table
            dynsymNames = section_start + section_start[index].sh_link;
            relocationTables = map_start + section_start[i].sh_offset; // get the current relocation table
            char* dynsym_names_pointer = map_start + dynsymNames->sh_offset;

            printf("offset\t\tinfo\t\ttype\t\tvalue\t\tname\n");
            for (int j=0; j<section_start[i].sh_size / sizeof(Elf32_Rel); j++){
                currentRelocation = relocationTables[j];
                TablePrinter(currentRelocation,dynsymTable,dynsym_names_pointer);

            }
            printf("\n");
        }
    }}

void quit (state* s) {
    if (s->debug_mode == 1)
    {
        printf("quitting..\n");
    }
    free(s);
    exit(0);
}

int main(int argc, char **argv){
    state *s = malloc(sizeof(state));
    s->unit_size = 1;
    s->debug_mode = 0;
    s->mapped = 0;
    fun_desc menu[] = {{"Toggle Debut Mode" , toggleDebugMode} ,
                       {"Examine_ELF_File" , ExamineELFFile} ,
                       {"Print Section Names" ,PrintSectionNames} ,
                       {"Print Symbols" , PrintSymbols} ,
                       {"Relocation Tables" , RelocationTables} ,
                       {"quit" , quit},
                       { NULL , NULL}};

    int length = sizeof(menu) / sizeof(menu[0]);
    while(1){
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
