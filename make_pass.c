#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc,char** argv)
{
    printf(argv[1]);
    putchar('\n');
    int i;
    char * targ = (char*)calloc(strlen(argv[1])+1,sizeof(char));
    char * targ2 = (char*)calloc(strlen(argv[1])+1,sizeof(char));
    char c;
    
    for(i=0;i<strlen(argv[1]);i++){
	*(targ+i) =  (~(( ( *(argv[1]+i)   & 0x0f) << 4) | ( (*(argv[1]+i) & 0xf0 )>> 4)))^i;
    }
    printf("char __xoredpass[%d]={",strlen(argv[1])+1);
    for(i=0;i<strlen(argv[1]);i++){
	printf("0x%02x,",*(targ+i)&0xff);
    }
    printf("0x0}\n");
    
     for(i=0;i<strlen(argv[1]);i++){
	 c = *(targ+i);
	 c ^= i;
	 c = ~c;
	 c =  ((c& 0x0f) << 4) | ((c & 0xf0) >> 4);
	 *(targ2+i) = c;
	 
    }
     printf("%s\n",targ2);
     
    free(targ);
    free(targ2);
    
    
}
