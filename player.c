#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 1024


int main(int argc, char * argv[])
{
    struct sockaddr_in server; // server address
    int clientsock; // client socket descriptor
    char client_buf[BUF_SIZE];
    char server_reply[BUF_SIZE];
    char str[BUF_SIZE];
    int port = atoi(argv[3]); // define the port number 
    int res, ret;
    fd_set rfds;
    struct timeval tv;
    
    tv.tv_sec = 30; // set the timeout for 30 sec
    tv.tv_usec = 0;
    
    // // define the server address
    if (argc < 3){
        printf("Invalid arguments\nPlease enter 3 correct arguments\n(<Game Type> <Server Name> <Port Number>)\n");
        exit(1);
    }

    if(strcmp(argv[1], "numbers") != 0) // check the game type
  {
    printf("The game <%s> does not exist\n", argv[2]);
    exit(0);
  }
    
    server.sin_addr.s_addr = inet_addr("192.168.1.116"); // define IP address
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    // create socket
    clientsock = socket(AF_INET, SOCK_STREAM, 0);
    
    if(clientsock < 0)
    {
        perror("Could not create socket");
    }
    printf("Socket created\n");
    
    // connect (to remote server)
    res = connect(clientsock, (struct sockaddr *) &server, sizeof(server));
    if(res == -1){
        perror("Connection failed");
        exit(1);
    }
    
    // Keep communicating with server
    while(1){
        memset(client_buf, '\0', sizeof(client_buf));
        memset(server_reply, '\0', sizeof(server_reply));
        if (recv(clientsock, server_reply, BUF_SIZE, 0) == -1)
        {
            perror("ERROR");
            exit(1);
        }  
        if((strncmp(server_reply, "TEXT ", 5)) == 0) 
        {
            // if client receives TEXT message
            strcpy(str, server_reply + 5);
            printf("%s", str);
        }
        else if((strcmp(server_reply, "GO")) == 0)
        {
            // if client receives GO message
            printf("Enter a number: ");
            fflush(stdout);

            FD_ZERO(&rfds);
            FD_SET(0, &rfds);

            // set the timeout
            ret = select(1, &rfds, NULL, NULL, &tv);

            if(ret < 0){
                perror("select()");
            }
            else if(ret > 0){
                // if the client respond within 30 sec
                fgets(str, BUF_SIZE, stdin);
            }
            else if (ret == 0) {
                // if the client does not respond for more than 30 sec
                printf("\ntimeout\n");
                close(clientsock);
                exit(1);
            }

            if (isdigit(str[0])) // check if it's a number
            {
                // send MOVE message
                sprintf(client_buf, "MOVE %s", str);
            }
            else if (strncmp(str, "quit", 4) == 0) // check if it's quit
            {
                // send QUIT message
                sprintf(client_buf, "QUIT");
            }
            else 
            {
                // if the input is invalid, always send 10 instead
                strcpy(client_buf, "MOVE 10");
            }

            if (send(clientsock, client_buf, BUF_SIZE, 0) < 0){
                perror("Send failed");
             }
        }
        else if((strcmp(server_reply, "END")) == 0)
        {
            // if client receives END message
            printf("Game ended\n");
            close(clientsock); // close client socket
            exit(0);
        }
        else{
            // protocol infingement
            close(clientsock); // close client socket
            exit(1);
        }
        
    }

    // close connection
    close(clientsock);
    return 0;
}
