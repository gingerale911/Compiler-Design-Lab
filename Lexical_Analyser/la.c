#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "preprocessor.h"

#define TABLE_SIZE 50






struct Token {
    char type[30];
    char lexeme[100];
    int row;
    int col;
};



static char *keywords[] = {
    "int","float","double","char","if","else",
    "for","while","return","void","main","break",
    "continue","long","short","bool"
};

static int kwCount = 16;




static int tokenIndex = 0;
int symIndex = 0;

char Dbuf[30];  


struct Symbol {
    int index;
    char lexeme[30];
    char type[30];
    int size;
    char returntype[30];
    struct Symbol *next;
};

struct Symbol *table[TABLE_SIZE];




int isKeyword(char *str){
    for(int i=0;i<kwCount;i++)
        if(!strcmp(str,keywords[i]))
            return 1;
    return 0;
}

int isDatatype(char *str){
    return (!strcmp(str,"int") || !strcmp(str,"float") ||
            !strcmp(str,"double") || !strcmp(str,"char") ||
            !strcmp(str,"void") || !strcmp(str,"long") ||
            !strcmp(str,"short") || !strcmp(str,"bool"));
}

int isArithmetic(char ch){
    return (ch=='+'||ch=='-'||ch=='*'||ch=='/'||ch=='%');
}

int isRelational(char ch){
    return (ch=='<'||ch=='>'||ch=='='||ch=='!');
}

int isLogical(char ch){
    return (ch=='&'||ch=='|');
}

int isSpecial(char ch){
    return (ch=='('||ch==')'||ch=='{'||ch=='}'||
            ch=='['||ch==']'||ch==';'||ch==',');
}

void printToken(FILE *out, struct Token t){

    tokenIndex++;

    fprintf(out,"<%s,%d,%d,%d> ",
            t.type,
            t.row,
            t.col,
            tokenIndex);

    printf("<%s,%d,%d,%d> ",
           t.type,
           t.row,
           t.col,
           tokenIndex);
}

int hash(char *s){

    int sum = 0;

    for(int i=0;s[i];i++)
        sum += s[i];

    return sum % TABLE_SIZE;
}



struct Symbol* search(char *lex){

    int h = hash(lex);

    struct Symbol *cur = table[h];

    while(cur){

        if(!strcmp(cur->lexeme,lex))
            return cur;

        cur = cur->next;
    }

    return NULL;
}


void insert(char *lex, char *type, int size, char *ret){

    if(search(lex)) return;

    struct Symbol *s =
        (struct Symbol*)malloc(sizeof(struct Symbol));

    s->index = symIndex++;

    strcpy(s->lexeme,lex);
    strcpy(s->type,type);
    strcpy(s->returntype,ret);

    s->size = size;

    s->next = NULL;

    int h = hash(lex);

    if(!table[h])
        table[h] = s;
    else{

        struct Symbol *c = table[h];

        while(c->next)
            c = c->next;

        c->next = s;
    }
}





void printTable(FILE *out){

    fprintf(out,"\n\nSymbol Table:\n");
    fprintf(out,"Index\tLexeme\tType\tSize\tReturnType\n");

    for(int i=0;i<TABLE_SIZE;i++){

        struct Symbol *c = table[i];

        while(c){

            fprintf(out,"%d\t%s\t%s\t%d\t%s\n",
                    c->index,
                    c->lexeme,
                    c->type,
                    c->size,
                    c->returntype);

            c = c->next;
        }
    }
}



int getSize(char *t){

    if(!strcmp(t,"int")) return 4;
    if(!strcmp(t,"float")) return 4;
    if(!strcmp(t,"double")) return 8;
    if(!strcmp(t,"char")) return 1;
    if(!strcmp(t,"bool")) return 1;

    return 0;
}



void getNextToken(FILE *in, FILE *out){

    char ch,next;

    int row = 1, col = 0;
    int newlineCount = 0;

    while((ch=fgetc(in))!=EOF){

        col++;

        if(ch!='\n' && !isspace(ch))
            newlineCount = 0;


            

        if(ch=='\n'){

            newlineCount++;

            if(newlineCount == 1){
                printf("\n");
                fprintf(out,"\n");
            }

            row++;
            col = 0;
            continue;
        }

        if(isspace(ch)) continue;

        

        if(ch == '"'){

            struct Token t;
            char buf[100];

            int i = 0;
            int start = col;

            while((ch = fgetc(in)) != EOF && ch != '"'){
                buf[i++] = ch;
                col++;
            }

            buf[i] = '\0';

            strcpy(t.lexeme, buf);
            strcpy(t.type, "string");

            t.row = row;
            t.col = start;

            printToken(out, t);
            continue;
        }
        

        if(isalpha(ch)||ch=='_'){

            struct Token t;
            char buf[50];

            int i=0,start=col;

            buf[i++] = ch;

            while(isalnum(ch=fgetc(in))||ch=='_'){
                buf[i++] = ch;
                col++;
            }

            buf[i] = '\0';

            ungetc(ch,in);

            strcpy(t.lexeme,buf);

            if(isKeyword(buf)){

                strcpy(t.type,"keyword");

                if(isDatatype(buf)){
                    strcpy(Dbuf,buf);
                }
            }
            else{

                strcpy(t.type,"id");

                char nc = fgetc(in);
                ungetc(nc,in);

                if(nc=='('){

                    insert(buf, "FUNC", -1, Dbuf);
                }

                else if(nc=='['){

                    int n=0,c;

                    fgetc(in);

                    while(isdigit(c=fgetc(in)))
                        n = n*10 + (c-'0');

                    if(c!=']')
                        ungetc(c,in);

                    int total = n * getSize(Dbuf);

                    insert(buf, Dbuf, total, "-");
                }

                else{

                    insert(buf, Dbuf,
                           getSize(Dbuf),
                           "-");
                }
            }
            t.row = row;
            t.col = start;

            printToken(out,t);
            continue;
        }
        if(isdigit(ch)){

            struct Token t;
            char buf[50];

            int i=0,start=col;

            buf[i++] = ch;

            while(isdigit(ch=fgetc(in))){
                buf[i++] = ch;
                col++;
            }

            buf[i] = '\0';

            ungetc(ch,in);

            strcpy(t.lexeme,buf);
            strcpy(t.type,"num");

            t.row = row;
            t.col = start;

            printToken(out,t);
            continue;
        } 
        if(isRelational(ch)){

            struct Token t;
            char buf[3];  int start = col;

            buf[0] = ch;
            buf[1] = '\0';

            next = fgetc(in);

            if(next=='='){

                buf[1]='=';
                buf[2]='\0';
                col++;
            }
            else ungetc(next,in);

            strcpy(t.lexeme,buf);
            strcpy(t.type,buf);

            t.row=row;
            t.col=start;

            printToken(out,t);
            continue;
        }

        

        if(isLogical(ch)){

            struct Token t;
            char buf[3];

            int start=col;

            buf[0]=ch;

            next=fgetc(in);

            if(next==ch){

                buf[1]=ch;
                buf[2]='\0';
                col++;
            }
            else{
                buf[1]='\0';
                ungetc(next,in);
            }

            strcpy(t.lexeme,buf);
            strcpy(t.type,buf);

            t.row=row;
            t.col=start;

            printToken(out,t);
            continue;
        }

        
        if(isArithmetic(ch)){

            struct Token t;

            char buf[2]={ch,'\0'};

            strcpy(t.lexeme,buf);
            strcpy(t.type,buf);

            t.row=row;
            t.col=col;

            printToken(out,t);
            continue;
        }
        

        if(isSpecial(ch)){

            struct Token t;

            char buf[2]={ch,'\0'};

            strcpy(t.lexeme,buf);
            strcpy(t.type,buf);

            t.row=row;
            t.col=col;

            printToken(out,t);

            if(ch == ';'){
                Dbuf[0] = '\0';
            }

            continue;
        }
    }
}


int main(){

    preprocessFile("oo.c","clean.c");

    FILE *in = fopen("clean.c","r");
    FILE *out = fopen("o.c","w");

    if(!in){
        printf("File error\n");
        return 1;
    }

    printf("Tokens:\n\n");
    fprintf(out,"Tokens:\n\n");

    tokenIndex = 0;

    getNextToken(in,out);

    fclose(in);

    printTable(stdout);
    printTable(out);

    fclose(out);

    return 0;
}
