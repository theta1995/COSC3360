#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <map>
#include <vector>
#include <string.h>


// 1. Server creates socket using socket system call
// 2. Assigns with IP and port number to bind them
// 3. Listen, waits for the client to approach the server to make a connetion
// 4. Accept (Connection is established between client and server)
void error(char *msg)
{
    perror(msg);
    exit(1);
}

struct input_file
{
    int fixed_bit;
    char symbol;
    std::string code_integer;
    std::string binary_representation;
};

void fireman(int)
{
    while(waitpid(-1, NULL, WNOHANG) > 0)
        std::cout << "A child has died." << std::endl;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int num_symbols,n;
    int greatest_base_10 = 0;
    std::string line;
    //check valid arguments
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    //Find fixed-bit
    std::cin >> num_symbols;
    std::cin.ignore();
    struct input_file msg[num_symbols];
    for (int i = 0; i < num_symbols; i++)
    {
        getline(std::cin, line);
        msg[i].symbol = line[0];
        msg[i].code_integer = line.substr(2);

        if (greatest_base_10 < stoi(msg[i].code_integer))
        {
            greatest_base_10 = stoi(msg[i].code_integer);
        }
    }
    int bit_length = ceil(log2(greatest_base_10 + 1));

    //Binary Conversion
    for(int i = 0; i < num_symbols; i++){
        msg[i].fixed_bit = bit_length;
        std::string binary;
        int decimal = stoi(msg[i].code_integer);
        while(decimal > 0){
            binary.append(std::to_string(decimal % 2));
            decimal /= 2;
        }
        reverse(binary.begin(), binary.end());
        while (binary.length() < bit_length)
        { // formating to match the fit-size bit length
            binary.insert(0, "0");
        }
            msg[i].binary_representation = binary;
    }
    //Map: String binary, Char symbol
    std::map<std::string, char> key;
    for(int i = 0; i<num_symbols; i++){
        key[msg[i].binary_representation] = msg[i].symbol;
    }

    //Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error((char*)("ERROR opening socket"));
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error((char*)("ERROR on binding"));

    listen(sockfd,5);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);

    if (newsockfd < 0) 
        error((char*)("ERROR on accept"));

    n = write(newsockfd,&bit_length,sizeof(bit_length));
    if (n < 0) error((char*)("ERROR writing to socket"));


    signal(SIGCHLD, fireman);
    while ((true))
    {  
        int child_socket = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
        if(child_socket<0) 
            error((char*)("ERROR accepting child"));

        if (fork() == 0)
        {
            char buffer[256];
            int z = read(child_socket,&buffer,sizeof(buffer));
            if(z<0) 
                error((char*)("ERROR reading from child threads"));
            char value = key[buffer];
            z = write(child_socket,&value,sizeof(value));
            if(z<0) 
                error((char*)("ERROR writing to child threads"));

            close(child_socket);
            _exit(0);
        }
    }
    wait(0);
    close(sockfd);

    return 0; 
}
