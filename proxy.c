#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
void doProxy(int connfd);
void parse_url(char *url, char *host, char *uri);
void get_requesthdrs(rio_t *riop, char *host); 
void scan_hdr(char *buf, char *key, char *val);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
            Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, 0);
            printf("Accepted connection from (%s, %s)\n", hostname, port);
        doProxy(connfd);                                             
        Close(connfd);                                            
    }
}

/*This function transfers the client/server messages*/
void doProxy(int connfd)
{   
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE], host[MAXLINE], uri[MAXLINE];
    rio_t rio;
    // parse the request line
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) // read the request line
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s\r\n", method, url, version); 
    // GET http://www.cmu.edu/hub/index.html HTTP/1.1
    strcpy(version, "HTTP/1.0");//always forward 1.1
    parse_url(url, host, uri);
    printf("host: %s, uri: %s version: %s\r\n", host, uri, version);
    get_requesthdrs(&rio, host);
}

void parse_url(char *url, char *host, char *uri)
{   
    char *slash;
    char *doubleSlash;
    //host name is between '//' and first '/',http://www.cmu.edu/hub/index.html 
    doubleSlash = strstr(url, "//");  
    if (doubleSlash){
        slash = strstr(doubleSlash + 2, "/");// first '/' after '//'
        if (slash) {
            strcpy(uri, "/");
            strcat(uri, slash + 1);
            *slash = '\0';
        }
        // host = doubleSlash + 2;//bug reassign host in local scale
        strcpy(host, doubleSlash + 2);
    }
    
}

void get_requesthdrs(rio_t *riop, char *host)
{
    char buf[MAXLINE], key[MAXLINE], val[MAXLINE];
    
    int has_host = 0;
    int has_user_agent = 0;
    int has_connection = 0;
    int has_proxy_connection = 0;

    Rio_readlineb(riop, buf, MAXLINE);
    /*HOST/User-Agent/Connection/Proxy-Connection*/
    while(strcmp(buf, "\r\n")) {       
        //sscanf is error prone 
        //sscanf(buf, "%s %s\r\n", key, val);
        scan_hdr(buf, key, val);
        if (!strcmp(key, "Host")) {
            has_host = 1;
            // retain original one
        } else if (!strcmp(key, "User-Agent")){
            has_user_agent = 1;
            strcpy(val, user_agent_hdr);
        } else if (!strcmp(key, "Connection")){
            has_connection = 1;
            strcpy(val, "close\r\n");
        } else if (!strcmp(key, "Proxy-Connection")) {
            has_proxy_connection = 1;
            strcpy(val, "close\r\n");
        }
        sprintf(buf, "%s: %s", key, val);
        printf("%s", buf);
        Rio_readlineb(riop, buf, MAXLINE);
    }
    if (!has_host) {
        printf("HOST: %s\r\n", host);
    }
    if (!has_user_agent) {
        printf("User-Agent: %s", user_agent_hdr);
    } 
    if (!has_connection) {
        printf("Connection: %s\r\n", "close");
    } 
    if (!has_proxy_connection) {
        printf("Proxy-Connection: %s\r\n", "close");
    }
    printf("%s", "\r\n");//end of request
}

void scan_hdr(char *buf, char *key, char *val)
{
    char *colon;
    colon = index(buf, ':');
    *colon = '\0';
    strcpy(val, colon + 2);
    strcpy(key, buf);
}