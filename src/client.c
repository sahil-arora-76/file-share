
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <signal.h>
#define GET 0
#define POST 1
#define PUT 2 
#define DELETE 3
#define BUFFERSIZE 100 

typedef struct PATH
{
    int len; 
    char *path;
} path_t; 

path_t global_path;

void print(); 

struct connection 
{
    struct sockaddr_in addr; 
    int fd; 
}; 

typedef int (*fp)(char **, struct connection *);

char *help =  "----------------------------\n"
"A Simple File Transfer Made By notadevps\n"
"refer https://github.com/notadevps for more info\n" 
"built in commands -:\n" 
"sopen [file/folder name] - open a dir or file from server\n"
"exit - exit from shell\n"
"help - all built in commands\n"
"clear - clear the screen\n"
"sls - list the file/dir from the server\n"
"sload [path to download] - load (download) the file to your pc \n"
"fstats [file/folder name] - show the stats of a file or folder \n"
"----------------------------\n";


int connect_check(int argc, char **argv, struct connection *addr) 
{
    int sockfd; 
    int sock; 

    if (argc < 2) 
    {
        printf("mission ip and port to connect \n"); 
        exit(1);
    }
    char *ip =  argv[1]; 
    int port = atoi(argv[2]); 
    printf("Trying to reach out ip %s on port %d\n", ip, port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0 ) 
    {
        printf("failed to connect\n"); 
        exit(1);
    }
    addr->fd = sockfd; 
    addr->addr.sin_family = AF_INET;
    addr->addr.sin_port = htons(port); 
    addr->addr.sin_addr.s_addr = inet_addr(ip);

    sock = connect(sockfd, (struct sockaddr *)&(addr->addr), sizeof(addr->addr)); 
    if (sock < 0)
    {
        printf("failed to connect\n"); 
        exit(1);
    } else 
    {
        return sock;
    }
}

char *read_line() 
{
    int current_buffer_size = 100; 
    char *text = NULL; 
    text = (char*)malloc(sizeof(char) * current_buffer_size); 
    if (!text) 
    {
        printf("malloc error \n"); 
        exit(1);
    }
    char ch; 
    int size = 0; 
    while (ch != '\n')
    {
        if (size > (current_buffer_size - 1)) 
        {
            text = realloc(text, BUFFERSIZE); 
            if (!text) 
            {
                printf("realloc error\n");
                exit(1);
            } 
            current_buffer_size += BUFFERSIZE;
        }
        ch = getchar(); 
        text[size] = ch;
        size++;
    }
    text[size] = '\0';
    return text;
}

char **parse(char *str) 
{
    int buffer = 64; 
    char **arr = malloc(sizeof(char) * buffer); 
    char *key; 
    const char *delim = " \t\r\n\a"; 
    if (!arr) 
    {
        printf("malloc error\n");
        exit(1); 
    };  

    key = strtok(str, delim); 
    int i = 0; 
    while(key != NULL) 
    {
        arr[i] = key;
        i++; 
        if (i >= buffer) {
            buffer += 64;
            arr = realloc(arr, buffer * sizeof(char*));
            if (!arr) 
            {
                printf("allocation error\n"); 
                exit(1);
            }; 
        }
        key = strtok(NULL, delim);  
    }
    arr[i] = NULL;
    return arr;
}


void removeChar(char* s, char c)
{
 
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != c)
            s[j++] = s[i];
 
    s[j] = '\0';
}

void print() 
{
    printf(">~%s$ ", global_path.path); 
}

int sls(char **str, struct connection *c) 
{
    printf("%s\n", global_path.path);
    int rc; 
    char buffer[8000];

    char req[500]; 
    snprintf(req, sizeof(req), "GET %s HTTP/1.1 \r\n\r\n", global_path.path);
    
    rc = send(c->fd, &req[0], strlen(req), 0);
    if (rc < 0) 
    {
        printf("failed \n");
        exit(1);
    }
    read(c->fd, buffer, 8000);

    char *s = "\n\n";
    char *m = strstr(buffer, &s[0]); 
    memmove(&m[1], &m[2], strlen(m) - 1);
    printf("%s%s", "\033[1A", m);
    return 1;
}

int download_file(char *path, char *content, size_t size)
{
    FILE *fp; 
    fp = fopen(path, "w"); 
    if (!fp) 
    {
        return -1;
    }
    int c = fwrite(content, 1, size, fp); 
    return 1;
}

int sstat(char **str, struct connection *c)
{
    char buffer[8000]; 
    char req[500];
    char *s; 
    char *m; 

    snprintf(req, sizeof(req), "HEAD %s HTTP/1.1 \r\n\r\n", global_path.path); 
    send(c->fd, req, strlen(req), 0); 
    read(c->fd, buffer, 8000); 
    if (strstr(buffer, "no such file") != NULL) 
    {
        printf("no such file\n"); 
        return 1;
    }
    s = "\n\n";
    m = strstr(buffer, &s[0]); 
    memmove(&m[1], &m[2], strlen(m) - 1);
    printf("%s%s", "\033[1A", m);
    return 1;
}

int sload(char **str, struct connection *c) 
{
    char *path = str[1]; 
    int rc;
    if (!path) 
    {
        printf("usage\n %s <path-to-download>\n", str[0]); 
        return 1;
    }
    char buffer[8000];
    char req[500]; 
    char *s; 
    char *m; 

    snprintf(req, sizeof(req), "POST %s HTTP/1.1 \r\n\r\n", global_path.path); 
    send(c->fd, req, strlen(req), 0); 
    read(c->fd, buffer, 8000); 
    if (strstr(buffer, "no such file") != NULL) 
    {
        printf("no such file\n"); 
        return 1;
    }
    s = "\n\n";
    m = strstr(buffer, &s[0]); 
    rc = download_file(path, buffer, strlen(buffer));
    if (rc == -1) 
    {
        printf("%s doesn't exist\n", path); 
        return 1;
    } else 
    {
        printf("Done! Downloaded bytes=%ld\n", strlen(buffer));
        return 1;
    }
}

int sopen(char **str, struct connection *c)
{
    int rc; 
    char buffer[8000];

    if (!str[1]) 
    {
        printf("no dir or folder\n");
        return -1;
    }
    if (strcmp(str[1], "..") == 0) 
    {
        int flag = 0; 
        global_path.path[global_path.len - 1] = '~';
        int length = 0; 
        for (int i = (global_path.len - 1); i > 0; i--) 
        {
            if (global_path.path[i] == '/') 
            {
                flag = 1; 
            }
            if (flag == 0) 
            {
                global_path.path[i] = '~'; 
                length++;
            }
        }
        removeChar(global_path.path, '~');
        global_path.len -= length; 
        return 1;
    }
    char req[500];
    snprintf(req, sizeof(req), "GET %s%s%s HTTP/1.1 \r\n\r\n", global_path.path, str[1], "/");
    rc = send(c->fd, &req[0], strlen(req), 0);
    if (rc < 0) 
    {
        printf("failed \n");
        exit(1);
    }
    read(c->fd, buffer, 8000); 
    if (strstr(buffer, "error:") != NULL) 
    {
        printf("no such file or dir\n"); 
        return 1;
    }

    char *temp = str[1];
    strcat(temp, "/"); 
    int size = strlen(temp);
    if (global_path.len > 200) 
    {
        global_path.path = realloc(global_path.path,  size * 100);
    }
    int j = 0; 
    int i = global_path.len;
    
    while (j < size) 
    {
        global_path.path[i] = temp[j];
        i++;
        j++;
    }
    global_path.len += size; 
    return 1;
}

void gen_help() 
{
    printf("%s", help);
}

char *defaults[] = { "sls", "sopen", "exit", "clear", "help", "sdownload", "sload", "sstat" };

int execute(char **str, struct connection *addr) 
{

    if (strcmp(defaults[0], str[0]) == 0) 
    {
        sls(str, addr);
        return 1;
    } else if (strcmp(defaults[1], str[0] ) == 0)
    {
        sopen(str, addr);
        return 1;
    } else if (strcmp(defaults[2], str[0]) == 0) 
    {
        exit(0);
    } else if (strcmp(defaults[3], str[0]) == 0)
    {
        system("clear");
        return 1;
    } else if (strcmp(defaults[4], str[0]) == 0) 
    {
        gen_help();
        return 1;
    } else if (strcmp(defaults[5], str[0]) == 0 || strcmp(defaults[6], str[0]) == 0)
    {
        sload(str, addr);
        return 1;
    } else if (strcmp(defaults[7], str[0]) == 0) 
    {
        sstat(str, addr);
        return 1;
    } else 
    {
        printf("command not found. use 'help' for more info\n");
        return 1;
    }
}

void signal_callback_handler(int s) 
{
    system("clear");
    free(global_path.path);
    exit(1);
}


int main(int agrc, char **argv)
{
    struct connection client_addr; 
    global_path.path = malloc(200); 
    global_path.path[0] = '/';
    global_path.len = 1; 
    signal(SIGINT, signal_callback_handler);

    connect_check(agrc, argv, &client_addr);
    system("clear");
    char *str = NULL; 
    char **parsed = NULL;
    do 
    {
        print();
        str = read_line(); 
        str[strlen(str) - 1] = '\0';
        parsed = parse(str);
        execute(parsed, &client_addr); 
        free(str); 
        free(parsed);
    } while(1);
    free(global_path.path);

}