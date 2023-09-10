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
#include <time.h>
#define GET 0
#define POST 1
#define PUT 2 
#define DELETE 3
#define HEAD 4

// int get_size(); 

int get_size(char *direc);
const char *get_type();
typedef struct REQUEST
{
    int method; 
    char *path;
    char *body;
} req;

typedef struct ERR 
{
    int code; 
} error;
error err;

enum type { dir, file, png, jpg, jpeg, gif };
enum type flag;

/* 
GET CODES
0 -> driec
1 -> files
2 -> jpg
3 -> png
4 -> jpeg 
5 -> gif

-1 -> dir or file doesn't exist
*/


void removeChar(char* s, char c)
{
 
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != c)
            s[j++] = s[i];
 
    s[j] = '\0';
}


int get_file_size(char *direc) 
{
    FILE *fp; 
    fp = fopen(direc, "r"); 
    if (!fp)
    {
        return -1; 
    } else
    {
        flag = 1;
        int size; 
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        fclose(fp);
        return size; 
    }
}

int get_image_size(char *s) 
{
    FILE *fp = fopen(s, "rb");
    if (!fp) 
    {
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return size; 
}

int get_size(char *direc)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(direc);
    if (d) 
    {
        int i = 0; 
        while ((dir = readdir(d)) != NULL) 
        {
            i++;
        }
        closedir(d);
        flag = 0;
        return i; 
    } else if (strstr(direc, ".jpg") != NULL)
    {
        flag = 2; 
        return get_image_size(direc);
    } else if (strstr(direc, ".png") != NULL) 
    {
        flag = 3;
        return get_image_size(direc);
    } else if (strstr(direc, ".jpeg") != NULL) 
    {
        flag = 4;
        return get_image_size(direc);
    } else if (strstr(direc, ".gif") != NULL) 
    {
        flag = 5;
        return get_image_size(direc);
    } else 
    {
        int size = get_file_size(direc);
        if (size == -1) 
        {
            return -1;
        } else 
        {
            return size; 
        } 
    }
}


char *get_file_contents(char *direc, int size) 
{
    char *buffer = malloc(size + 1);
    FILE *fp; 
    fp = fopen(direc, "r"); 
    fread(buffer, size + 1, 1, fp); 
    buffer[size] = '\0'; 
    fclose(fp); 
    return buffer;
}

char *read_dir(char *direc) 
{
    DIR *d;
    struct dirent *dir;
    d = opendir(direc);
    int size = get_size(direc); 
    if (size == -1) 
    {
        char *info = "error: no such file or dir";
        err.code = size;
        return info;
    }

    if (size > 0 && flag == 1) 
    {
        char *c = get_file_contents(direc, size); 
        return c;
    } else if (size > 0 && (flag == 2 || flag == 3 || flag == 4 || flag == 5))
    {
        return NULL;
    } else if ((size > 0) && flag == 0)
    {

        char *files = malloc(256 * size);
        int i = 0; 
        if (d) 
        {
            while ((dir = readdir(d)) != NULL) 
            {
                if (strcmp(dir->d_name, ".." ) == 0 || strcmp(dir->d_name, ".") == 0) 
                {
                    continue;
                }
                for (int j = 0; j < strlen(dir->d_name); j++)
                {
                    files[i] = dir->d_name[j]; 
                    i++;
                }
                files[i] = ' ';
                files[i + 1] = ' ';
                i+= 2;
            }
            files[i] = '\0';
            closedir(d);
        } else 
        {
            printf("is not a dir\n"); 
            exit(1); 
        }

        return files;
    } 
    return NULL;
}

char *info(char *direc) 
{
    int c = access(direc, F_OK); 
    if (c != 0) 
    {
        char *info = "no such file\n"; 
        err.code = -1; 
        return info;
    }

    struct stat stats;
    struct tm dt;

    if (stat(direc, &stats) == -1) 
    {
        char *info = "no such file\n"; 
        err.code = -1; 
        return info;
    } else 
    {
        char *buffer;
        buffer = malloc(sizeof(struct stat)); 
        char temp[500] = "File access: ";
        if (stats.st_mode & R_OK) 
        {
            strcat(temp, "read");
        } 
        if (stats.st_mode & W_OK)
        {
            strcat(temp, " write");
        } 
        if (stats.st_mode & X_OK)
        {
            strcat(temp, " execute");
        }
        strcat(temp, "\n");
        {
            char tempg[80];
            snprintf(tempg, sizeof(tempg), "File size: %ld\n", stats.st_size);
            strcat(temp, tempg);
        }
        {
            char time[150];
            dt = *(gmtime(&stats.st_ctime));
            snprintf(time, sizeof(time), "Created on: %d-%d-%d %d:%d:%d\n", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
            dt.tm_hour, dt.tm_min, dt.tm_sec); 
            strcat(temp, time);
        }
        {
            char time[150];
            dt = *(gmtime(&stats.st_mtime));
            snprintf(time, sizeof(time), "Modified on: %d-%d-%d %d:%d:%d\n", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
            dt.tm_hour, dt.tm_min, dt.tm_sec);
            strcat(temp, time);
        }
        strcpy(buffer, temp);
        return buffer;
    }
    return NULL;
}


char *download_file(char *r) 
{
    int size = get_file_size(r); 
    if (size == -1) 
    {
        err.code = -1; 
        char *info  = "no such file\n"; 
        return info;
    } else 
    {        
        char *buff = get_file_contents(r, size);
        return buff; 
    }
}




char *send_res(req *r) 
{
    char *homedir = getpwuid(getuid())->pw_dir;
    if (r->method == 0 && strcmp(r->path, "/") == 0) 
    {
        char *s = read_dir(homedir);
        return s; 
    } else if (r->method == 0) 
    {
        int size = strlen(r->path); 
        if (r->path[size - 1] == '/') 
        {
            r->path[size - 1] = '\0';
        }
        strcat(homedir, r->path);
        char *s = read_dir(homedir);
        return s;
    } else if (r->method == 1 && r->body == NULL) 
    {
        int size = strlen(r->path); 
        if (r->path[size - 1] == '/') 
        {
            r->path[size - 1] = '\0';
        }
        strcat(homedir, r->path);
        char *data = download_file(homedir);
        return data; 
    } else if (r->method == 4) 
    {
        int size = strlen(r->path);
        if (r->path[size - 1] == '/') 
        {
            r->path[size - 1] = '\0';
        }
        strcat(homedir, r->path);
        char *s = info(homedir);
        return s;
    } 
    return NULL;
}

int compare(char *s, req *r)
{
    if (strcmp(s, "GET") == 0) 
    {
        r->method = GET;
        return 1; 
    } else if (strcmp(s, "POST") == 0) 
    {
        r->method = POST;
        return 1;
    } else if (strcmp(s, "PUT") == 0) 
    {
        r->method = PUT;
        return 1;
    } else if (strcmp(s, "DELETE") == 0) 
    {
        r->method = DELETE;
        return 1; 
    } else if (strcmp(s,  "HEAD") == 0) 
    {
        r->method = HEAD; 
        return 1;
    } else 
    {
        r->method = -1;
        return -1;
    }
}

req parse(char *str, size_t length) 
{
    req r; 
    int i;
    char *m;

    /*method parsing*/
    int space = 32; 
    m = strstr(str, (char *)&space); 
    int index = m - str; 
    char *method  = malloc(index + 1);
    for (i = 0; i < index; i++) 
    {
        method[i] = str[i]; 
    }
    method[i] = '\0';
    compare(method, &r);
    free(method);
    /*path parsing*/ 
    str = str + index + 1;
    m = strstr(str, (char *)&space);
    index  = m - str;
    r.path = malloc(index + 1); 
    for (i = 0; i < index; i++) 
    {
        r.path[i] = str[i]; 
    }
    r.path[i] = '\0';

    r.body = NULL;
    return r;
}

void display_image(int new_sock, req *q)
{
    const char *image_type = get_type();
    if (image_type == NULL) 
    {
        printf("error\n"); 
        exit(1);
    }
    char *homedir = getpwuid(getuid())->pw_dir;
    strcat(homedir, q->path);
    char webpage[200];
    snprintf(webpage, sizeof(webpage),  "HTTP/1.1 200 Ok\r\n"
    "Content-Type: %s; charset=UTF-8\r\n\r\n", image_type);
    send(new_sock, webpage, strlen(webpage), 0);
    int p = open(homedir, O_RDONLY); 
    int size = get_image_size(homedir);
    sendfile(new_sock, p, NULL, size);
    flag = 0; 
    err.code = 0;
}

const char *get_type() 
{
    if (flag == 2)
    {
        return "image/jpg";
    } else if (flag == 3) 
    {
        return "image/png"; 
    } else if (flag == 4) 
    {
        return "image/jepg"; 
    } else if (flag == 5) 
    {
        return "image/gif";
    }
    return NULL;
}


int main(int argc, char **argv) 
{
    int sockfd; 
    int rc; 
    int yes = 1; 
    const void *val = &yes;
    struct sockaddr_in addr; 
    const int port = 4000; 
    int new_sock;
    if (!argv[1]) return -1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    {
        printf("failed: socket()"); 
        exit(1); 
    }
    
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(yes));
    if (rc < 0) 
    {
        printf("failed: setsockopt()"); 
        exit(1); 
    }
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = inet_addr(argv[1]); 
    
    rc = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0) 
    {
        printf("failed: bind()"); 
        exit(1); 
    }

    rc = listen(sockfd, 10); 
    if (rc < 0)
    {
        printf("failed: listen()"); 
        exit(1); 
    }
    int addrlen = sizeof(addr);
   
    new_sock = accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
    if (new_sock < 0) 
    {
        printf("failed: accpet()"); 
        exit(1);
    }
    while(1) 
    {
        char buffer[502];

        recv(new_sock, buffer, 502, 0);
        printf("%s", buffer);
        req p_sts = parse(buffer, strlen(buffer));
        if (strcmp(p_sts.path, "/exit") == 0)
        {
            break;
        }
        if (p_sts.method == -1) 
        {
            exit(1); 
        }
        if (strcmp(p_sts.path, "/favicon.ico") == 0) 
        {
            continue;
        }
        char *m = send_res(&p_sts);

        if(m == NULL && (flag == 2 || flag == 3 || flag == 4 || flag == 5))
        { 
            display_image(new_sock, &p_sts);
            continue;
        } else if (m == NULL) 
        {
            printf("failed\n"); 
            exit(1); 
        }
        char n_buffer[8000];
        snprintf(n_buffer, 8000, "HTTP/1.1 200 OK\n"
        "Content-Type: text\n"
        "Content-Length: %ld\n"
        "Accept-Ranges: bytes\n"
        "Connection: close\n"
        "\n"
        "%s\r\n", strlen(m), m); 
        rc = send(new_sock, n_buffer, 8000, 0); 
        if (rc < 0)
        {
            printf("sending failed\n"); 
            exit(1); 
        }
        if (err.code != -1)
        {
            free(m);
            free(p_sts.path);
            if (p_sts.body != NULL)
            {
                free(p_sts.body);
            }

        }
        err.code = 0; 
        flag = 0; 
    }
    
    close(sockfd);

    return 0; 
}
