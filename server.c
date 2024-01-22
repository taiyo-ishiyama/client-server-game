#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/select.h>
#include <ctype.h>

#define BACKLOG 10
#define BUF_SIZE 1024
#define MAXLINE 30

int main(int argc, char *argv[])
{
  struct sockaddr_in server, client;
  int res, ret;
  int count = 0;
  int port = atoi(argv[1]); // define the port number
  int num_player = atoi(argv[3]); // define the number of player
  int serversock;
  fd_set rfds;
  struct timeval tv;
  int num_exit = 0; // number of exited player during the game
  int arr_exit[BUF_SIZE]; // array of exited player IDs
  
  tv.tv_sec = 30; // set the timeout for 30 seconds
  tv.tv_usec = 0;

  int *clientsock = malloc(num_player * sizeof(int)); // dynamically allocate the array to store the client sockets

  char client_buf[BUF_SIZE];
  char server_reply[BUF_SIZE];
  char str[BUF_SIZE];

  // check if the arguments are correct
  if (argc < 3) {
        printf("Invalid arguments\nPlease enter 3 correct arguments\n<Port Number> <Game Type> <Game arguments>\n");
        exit(1);
    }

  // check the game type
  if(strcmp(argv[2], "numbers") != 0)
  {
    printf("The game <%s> does not exist\n", argv[2]);
    exit(0);
  }

  // Create TCP socket
  serversock = socket(AF_INET, SOCK_STREAM, 0);
  if (serversock == -1)
  {
    printf("Creating socket failed\n");
    exit(1);
  }

  printf("Socket successfully created\n");

  // prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  // bind addr to socket
  res = bind(serversock, (struct sockaddr *)&server, sizeof(server));
  if (res < 0)
  {
    printf("Bind failed\n");
    exit(1);
  }
  printf("Bind was successfully completed\n");

  res = listen(serversock, num_player);
  if (res != 0)
  {
    printf("Listen failed\n");
    exit(1);
  }
  printf("Waiting for incoming connections...\n");


  // accept multiple clients using array
  int clientlen = sizeof(client);
  for (int i = 0; i < num_player; i++)
  {
    clientsock[i] = accept(serversock, NULL, NULL);
    if (clientsock[i] < 0)
    {
      perror("Accept failed");
      exit(1);
    }
    else
    {
      printf("Connection accepted\n");
      printf("(%d/%d players)\n", i + 1, num_player);
      strcpy(server_reply, "TEXT Welcome to the game\nWaiting for other players connecting\n");
      send(clientsock[i], server_reply, sizeof(server_reply), 0);
    }
  }

  printf("--------Game Start--------\n");

  int sum = 0;
  int turn = 0;
  while (1)
  {
    for (int i = 0; i < num_player; i++)
    {
      memset(client_buf, '\0', BUF_SIZE);
      memset(server_reply, '\0', BUF_SIZE);
      // skip the turn of the exited players
      while(1){
        int ans = 1;
        for (int j = 0; j < num_exit; j++)
        {
          if(arr_exit[j] == i)
          {
            ans = 0;
            break;
          }
        }
        if (ans == 1)
        {
          break;
        }
        i++;
      }

      // end the game if the number of player is only one
      if (num_player - num_exit == 1) 
      {
        strcpy(server_reply, "TEXT You won\n");
        if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0){
          perror("Send failed");
        }
        strcpy(server_reply, "END");
        if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0){
          perror("Send failed");
        }
        close(clientsock[i]); // close the client socket
        printf("---------Game End---------\n");
        close(serversock); // close server socket
        free(clientsock);
        exit(0);
      }

      // send the sum of added numbers
      sprintf(server_reply, "TEXT Sum is %d\n", sum);
      if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
      {
        perror("Send failed");
      }

      // send GO message
      strcpy(server_reply, "GO");
      if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
      {
        perror("Send failed");
      }

      FD_ZERO(&rfds);
      FD_SET(clientsock[i], &rfds);

      // set the timeout
      ret = select(clientsock[i] + 1, &rfds, NULL, NULL, &tv);

      if(ret < 0){
          perror("select()");
      }
      else if(ret > 0){
          // if the client respond within 30 sec
          if (recv(clientsock[i], client_buf, BUF_SIZE, 0) == -1)
          {
              perror("ERROR");
              exit(1);
          } 
      }
      else{
          // if the client does not respond for more than 30 sec
          strcpy(server_reply, "TEXT Timeout\n");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          strcpy(server_reply, "END");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          close(clientsock[i]);
          printf("timeout\n");
          printf("Player %d exited\n", i + 1);
          arr_exit[num_exit] = i; // add the player ID to the array
          num_exit++;
          continue; // going to the next palyer's turn
      }
      
      if(strncmp(client_buf, "MOVE", 4) == 0) // server receives MOVE message
      {
        strcpy(str, client_buf + 5);
        int temp = atoi(str);
        if (temp < 1 || temp > 9) // check if it's between 1 - 9
        {
          count++;
          if (count >= 5) // if player sends invalid input 5 times in a row
          {
            strcpy(server_reply, "END");
            if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
            {
              perror("Send failed");
            }
            close(clientsock[i]);
            printf("Player %d exited\n", i + 1);
            arr_exit[num_exit] = i;
            num_exit++;
            count = 0;
            continue;
          }
          // send error message
          strcpy(server_reply, "TEXT ERROR Enter a one-digt number (1-9)\n");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          i--;	  
          continue; // do the same player's turn again
        }
        printf("%d is added by player %d\n", temp, i + 1);
        sum += temp; // calculate the sum
        count = 0;
      }
      else {
        strcpy(str, client_buf);
        if (strncmp(str, "QUIT", 4) == 0) // if server receives QUIT message
        {
          // send END message
          strcpy(server_reply, "END");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          close(clientsock[i]);
          printf("Player %d exited\n", i + 1);
          arr_exit[num_exit] = i; // add the player ID to the array
          num_exit++;
          continue; // going to the next player's turn
        }
        else{
          // protocal infringements
          strcpy(server_reply, "TEXT Invalid input\n");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          strcpy(server_reply, "END");
          if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0)
          {
            perror("Send failed");
          }
          close(clientsock[i]);
          printf("Player %d exited\n", i + 1);
          arr_exit[num_exit] = i; // add the player ID to the array
          num_exit++;
          continue; // going to the next player's turn
        }
      }


      if (sum >= 30) // if game is end
      {
        // send announcing message to each player
        strcpy(server_reply, "TEXT You won\n");
        if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0){
          perror("Send failed");
        }
        strcpy(server_reply, "END");
        if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0){
          perror("Send failed");
        }
        close(clientsock[i]); // close the client socket who won
        for (int j = 0; j < num_player; j++)
        {
          if(i == j){
            continue;
          }
          strcpy(server_reply, "TEXT You lost\n");
          if (send(clientsock[j], server_reply, sizeof(server_reply), 0) < 0){
            perror("Send failed");
          }
          strcpy(server_reply, "END");
          if (send(clientsock[j], server_reply, sizeof(server_reply), 0) < 0){
            perror("Send failed");
          }
          close(clientsock[j]); // close other players socket
        }
        printf("---------Game End---------\n");
        close(serversock); // close server socket
        free(clientsock);
        exit(0);;
      }

      strcpy(server_reply, "TEXT Waiting for the ohter players' turn\n");
      if (send(clientsock[i], server_reply, sizeof(server_reply), 0) < 0){
          perror("Send failed");
      }
    }
  }

  return 0;
}
