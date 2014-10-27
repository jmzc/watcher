#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


static const char TUXEDO_IP[] = "10.68.14.236";
static const int  TUXEDO_PORT = 4100;

int main( int argc, char **argv )
{
	
    if (argc != 2)    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <file>\n", argv[0]);
       exit(1);
    }
	
    if (watch(argv[1]))
    {
     notify(argv[1]);
    }
    exit(0);
	
}

int watch(char *f)  
{
  int length, i = 0 ,r = 0;
  int fd;
  int wd;
  char buffer[BUF_LEN];


  fd = inotify_init();

  if ( fd < 0 ) 
  {
    perror( "inotify_init" );
    return -1;
  }


  wd = inotify_add_watch( fd, f, IN_MODIFY);
  length = read( fd, buffer, BUF_LEN );  

  if ( length < 0 ) 
  {
    perror( "inotify_read" );
  }  

  while ( i < length ) 
  {
    struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
    if ( event->mask & IN_MODIFY ) 
    {
	r=1;
	break;
    }
    i += EVENT_SIZE + event->len;
  }

  ( void ) inotify_rm_watch( fd, wd );
  ( void ) close( fd );


  return r;
}

int notify(char *f)
{
	
    int sockfd = 0, n = 0;
    char *mail = malloc(2048);
    char buffer[1024];
    struct sockaddr_in serv_addr;

    memset(buffer, '0',sizeof(buffer));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TUXEDO_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(TUXEDO_IP);   /* Server IP address */

    if(inet_pton(AF_INET, TUXEDO_IP, &serv_addr.sin_addr)<=0)
    {
        perror("\n inet_pton error occured\n");
        return 1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       perror("\n Error : connect failed \n");
       return 1;
    }
     

    char hostname[1024];
    gethostname(hostname,1024);

    memset(&mail, '0', sizeof(mail));	
    asprintf(&mail, "198|0|mimovistar@prosodie.es|Error en %s|Se ha producido un error en fichero %s|%s", &hostname, f , "jmzc.prosodie@gmail.com");

    if (send(sockfd, mail, strlen(mail), 0) != strlen(mail))
        printf("send() sent a different number of bytes than expected");
	
    /*
    while ( (n = read(sockfd, buffer, sizeof(buffer)-1)) > 0)
    {
        buffer[n] = 0;
        if(fputs(buffer, stdout) == EOF)
        {
            perror("\n Error : fputs error\n");
        }
    }

    if(n < 0)
    {
        perror("\n read error \n");
    }
    */

    free(mail);

    close(sockfd);
    return 0;

}
