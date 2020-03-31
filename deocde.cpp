#include "decode.h"

char *get_mime_type(  char *name)
{
     char* dot;

    dot = strrchr(name, '.');	
   
    if (dot == (char*)0)
        return  (char*)"text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return  (char*)"text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return  (char*)"image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return  (char*)"image/gif";
    if (strcmp(dot, ".png") == 0)
        return  (char*)"image/png";
    if (strcmp(dot, ".css") == 0)
        return  (char*)"text/css";
    if (strcmp(dot, ".au") == 0)
        return  (char*)"audio/basic";
    if (strcmp( dot, ".wav") == 0)
        return  (char*)"audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return  (char*)"video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return  (char*)"video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return  (char*)"video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return  (char*)"model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return  (char*)"audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return  (char*)"audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return  (char*)"application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return  (char*)"application/x-ns-proxy-autoconfig";

    return  (char*)"text/plain; charset=utf-8";
}

int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';

    return(i);
}


 // 解码函数
void strdecode(char *to, char *from)
{
    for ( ; *from != '\0'; ++to, ++from) {

        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) { 

            *to = hexit(from[1])*16 + hexit(from[2]);
            from += 2;                      
        } else
            *to = *from;
    }
    *to = '\0';
}

//16进制转换成10进制
int hexit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}
//编码函数，写回浏览器时用到。
void strencode(char* to, size_t tosize, const char* from)
{
    int tolen;

    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
            *to = *from;
            ++to;
            ++tolen;
        } else {
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }
    }
    *to = '\0';
}
