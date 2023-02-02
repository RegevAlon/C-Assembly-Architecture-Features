#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char censor(char c) {
    if(c == '!')
        return '.';
    else
        return c;
}

char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    for (int i = 0 ; i < array_length ; i++){
        mapped_array[i] = f((array[i]));
    }
    return mapped_array;
}

char my_get(char c){
    char newC = fgetc(stdin);
    return newC;
}
/* Ignores c, reads and returns a character from stdin using fgetc. */


char cprt(char c){

    if ((c < 126) && (c > 20)){
        printf("%c\n" , c);

    } else{
        printf("%c\n" , '.');
    }
    return c;
}
/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */

char encrypt(char c){
    if((c<32) | (c>126))
        return c;
    else
        return c+3;
}
/* Gets a char c and returns its encrypted form by adding 3 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char decrypt(char c){
    if((c<32) | (c>126))
        return c;
    else
        return c-3;
}
/* Gets a char c and returns its decrypted form by reducing 3 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char xprt(char c){
    if ((c < 126) && (c > 20)){
        printf("%X\n" , c);

    } else{
        printf("%c\n" , '.');
    }
    return c;

}
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */

char quit(char c){
    if (c == 'q'){
        exit(1);
    }
    return c;
}

struct fun_desc {
    char *name;
    char (*fun)(char);
};
/* Gets a char c, and if the char is 'q' , ends the program with exit code 0. Otherwise returns c. */

int main(int argc, char **argv){
    char* carray = (char*)(malloc(5*sizeof(char)));
    int option = -1;
    struct fun_desc menu[] = {{"Get string", &my_get},{"Print string",&cprt},{"Print hex",&xprt},
                              {"Censor",&censor},{"Encrypt",&encrypt},{"Decrypt",&decrypt},{"Quit",&quit},{NULL,NULL}};
    int bounds = sizeof(menu) / (sizeof(struct fun_desc))-1;

    while(1){
        printf("Please choose a function:\n");
        for (int i=0; i<bounds; i++){
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option: ");
        scanf("%d", &option);
        if ((option<0) | (option>bounds-1)){
            printf("Not within bounds\n");
            exit(1);
        }
        else{
            printf("Within bounds\n");
            fgetc(stdin);
            char* temp = map(carray,5,menu[option].fun);
            free(carray);
            carray = temp;
            printf("DONE.\n\n");
        }
    }
}

