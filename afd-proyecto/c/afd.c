/* -*- coding: utf-8 -*-
   AFD en C (portable: MSVC/MinGW/Linux).
   Lee Conf.txt y Cadenas.txt, imprime "acepta"/"NO acepta" por línea.

   Uso:
     afd.exe Conf.txt Cadenas.txt
   Si no das argumentos, usa ../tests/Conf.txt y ../tests/Cadenas.txt

   Autor: estudiante 🇨🇴
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STATES 128
#define MAX_ALPHA  128
#define MAX_NAME   64
#define MAX_LINE   512

static char state_names[MAX_STATES][MAX_NAME];
static int  nstates = 0;

static char alphabet[MAX_ALPHA];
static int  nalpha = 0;

static int start_state = -1;
static int accepts[MAX_STATES];

static int delta[MAX_STATES][MAX_ALPHA]; /* -1 si no hay transición */

/* ----------------- utilidades ----------------- */

static void trim(char *s){
    /* quita espacios alrededor y CR/LF finales */
    size_t len = strlen(s);
    while (len && (s[len-1]=='\n' || s[len-1]=='\r' || isspace((unsigned char)s[len-1]))) {
        s[--len] = 0;
    }
    size_t i = 0;
    while (isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s+i, strlen(s+i)+1);
}

static void remove_spaces(char *s){
    char *d = s;
    for (; *s; ++s){
        if (*s!=' ' && *s!='\t' && *s!='\r') *d++ = *s;
    }
    *d = 0;
}

static int starts_with_nocase(const char *s, const char *pref){
    while (*pref && *s){
        if (tolower((unsigned char)*s) != tolower((unsigned char)*pref)) return 0;
        ++s; ++pref;
    }
    return *pref==0;
}

static int find_state(const char *name){
    for (int i=0;i<nstates;i++){
        if (strcmp(state_names[i], name)==0) return i;
    }
    return -1;
}

static int alpha_index(char c){
    for (int i=0;i<nalpha;i++) if (alphabet[i]==c) return i;
    return -1;
}

/* parsea lista "a,b,c" y la mete en out[], retorna cantidad */
static int parse_list_states(const char *s, char out[][MAX_NAME], int maxn){
    char buf[MAX_LINE]; strncpy(buf,s,sizeof(buf)); buf[sizeof(buf)-1]=0;
    int cnt=0;
    char *tok = strtok(buf, ",");
    while (tok && cnt<maxn){
        char t[MAX_LINE]; strncpy(t,tok,sizeof(t)); t[sizeof(t)-1]=0;
        trim(t);
        if (*t){
            strncpy(out[cnt], t, MAX_NAME); out[cnt][MAX_NAME-1]=0;
            cnt++;
        }
        tok = strtok(NULL, ",");
    }
    return cnt;
}

static int parse_list_alpha(const char *s, char out[], int maxn){
    char buf[MAX_LINE]; strncpy(buf,s,sizeof(buf)); buf[sizeof(buf)-1]=0;
    int cnt=0;
    char *tok = strtok(buf, ",");
    while (tok && cnt<maxn){
        char t[MAX_LINE]; strncpy(t,tok,sizeof(t)); t[sizeof(t)-1]=0;
        trim(t);
        if (strlen(t)!=1){
            fprintf(stderr,"Símbolo inválido (1 char): '%s'\n", t);
            exit(2);
        }
        out[cnt++] = t[0];
        tok = strtok(NULL, ",");
    }
    return cnt;
}

/* ----------------- parseo de configuración ----------------- */

static void init_delta(void){
    for (int i=0;i<MAX_STATES;i++)
        for (int j=0;j<MAX_ALPHA;j++)
            delta[i][j] = -1;
}

static void cargar_config(const char *path){
    FILE *f = fopen(path, "r");
    if (!f) { perror("Conf.txt"); exit(1); }
    init_delta();
    memset(accepts, 0, sizeof(accepts));

    char line[MAX_LINE];
    int en_trans = 0;
    while (fgets(line, sizeof(line), f)){
        /* quitar comentario y espacios extremos */
        char *hash = strchr(line, '#');
        if (hash) *hash = 0;
        trim(line);
        if (!*line) continue;

        if (!en_trans){
            if (starts_with_nocase(line,"states:")){
                char *v = strchr(line,':'); v++;
                nstates = parse_list_states(v, state_names, MAX_STATES);
                if (nstates<=0){ fprintf(stderr,"states vacío\n"); exit(1); }
            } else if (starts_with_nocase(line,"alphabet:")){
                char *v = strchr(line,':'); v++;
                nalpha = parse_list_alpha(v, alphabet, MAX_ALPHA);
                if (nalpha<=0){ fprintf(stderr,"alphabet vacío\n"); exit(1); }
            } else if (starts_with_nocase(line,"start:")){
                char *v = strchr(line,':'); v++; trim(v);
                start_state = find_state(v);
                if (start_state<0){ fprintf(stderr,"start desconocido: %s\n", v); exit(1); }
            } else if (starts_with_nocase(line,"accepts:")){
                char *v = strchr(line,':'); v++;
                char names[MAX_STATES][MAX_NAME];
                int m = parse_list_states(v, names, MAX_STATES);
                for (int i=0;i<m;i++){
                    int id = find_state(names[i]);
                    if (id<0){ fprintf(stderr,"accepts desconocido: %s\n", names[i]); exit(1); }
                    accepts[id]=1;
                }
            } else if (starts_with_nocase(line,"transitions:")){
                en_trans = 1;
            } else {
                fprintf(stderr,"Línea no reconocida: %s\n", line);
                exit(1);
            }
        } else {
            /* trans: src,sym->dst (espacios opcionales) */
            remove_spaces(line);
            char *arrow = strstr(line, "->");
            if (!arrow){ fprintf(stderr,"Transición inválida: %s\n", line); exit(1); }
            *arrow = 0;
            char *left = line;
            char *right = arrow+2;

            char *comma = strchr(left, ',');
            if (!comma){ fprintf(stderr,"Transición sin coma: %s\n", left); exit(1); }
            *comma = 0;
            char *src = left;
            char *sym = comma+1;
            char *dst = right;

            if (strlen(sym)!=1){ fprintf(stderr,"Símbolo debe ser 1 char: %s\n", sym); exit(1); }
            int s = find_state(src);
            int d = find_state(dst);
            int a = alpha_index(sym[0]);
            if (s<0 || d<0 || a<0){
                fprintf(stderr,"Estado/símbolo desconocido en: %s,%s->%s\n", src,sym,dst);
                exit(1);
            }
            delta[s][a] = d;
        }
    }
    fclose(f);

    if (start_state<0){ fprintf(stderr,"Falta start\n"); exit(1); }
}

/* ----------------- simulación ----------------- */

static int aceptar_cadena(const char *s){
    int curr = start_state;
    for (const char *p=s; *p; ++p){
        int a = alpha_index(*p);
        if (a<0) return 0;              /* símbolo fuera del alfabeto */
        int nx = delta[curr][a];
        if (nx<0) return 0;             /* transición no definida */
        curr = nx;
    }
    return accepts[curr] ? 1 : 0;
}

/* ----------------- main ----------------- */

static void usar(const char *exe){
    fprintf(stderr,"Uso: %s Conf.txt Cadenas.txt\n", exe);
}

int main(int argc, char **argv){
    const char *conf = (argc>1)? argv[1] : "../tests/Conf.txt";
    const char *cads = (argc>2)? argv[2] : "../tests/Cadenas.txt";

    cargar_config(conf);

    FILE *f = fopen(cads, "r");
    if (!f){ perror("Cadenas.txt"); return 1; }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)){
        trim(line); /* puede quedar cadena vacía (ε) */
        int ok = aceptar_cadena(line);
        if (*line)
            printf("%s -> %s\n", line, ok? "acepta":"NO acepta");
        else
            printf("ε -> %s\n", ok? "acepta":"NO acepta");
    }
    fclose(f);
    return 0;
}
