#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <pthread.h>



void error(char *msg)
{
    perror(msg);
    exit(0);
}

struct input_file
{
    int fixed_bit;
    char symbol;
    std::string code_integer;

};
struct child_struct{

    std::string binary_rep;
    struct hostent* child_server;
    int child_port, bit;
    char symbols;
};
//Multi-thread socket
void *runner(void *arg){
    struct child_struct *data = (child_struct *)arg;

    int sockfd,n;
    struct sockaddr_in serv_addr;
    struct hostent *server_name;
    std::string binary_msg = data->binary_rep;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_name = data->child_server;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server_name->h_addr, (char *)&serv_addr.sin_addr.s_addr,server_name->h_length);
    serv_addr.sin_port = htons(data->child_port);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error((char*)("ERROR connecting"));
    //send String:binary
    n = write(sockfd,binary_msg.c_str(), sizeof(binary_msg));
    if (n < 0)
        error((char*)("ERROR writing to socket"));
    n = read(sockfd,&buffer,sizeof(buffer));
    if (n < 0)
        error((char*)("ERROR reading from socket"));
    data->symbols = *buffer;

    close(sockfd);
    return 0;
}


int main(int argc, char *argv[])
{
    int sockfd,portno,n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    std::string binary_message;
    int bit_length;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error((char*)("ERROR opening socket"));
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error((char*)("ERROR connecting"));

    n = read(sockfd,&bit_length,sizeof(bit_length));
    if (n < 0) 
         error((char*)("ERROR reading from socket"));
         
    //Get input and split into fixed bit 
    std::cin >> binary_message;
    std::vector< std::string > arr;
    for(int i = 0; i < binary_message.size(); i = i + bit_length){
        arr.push_back(binary_message.substr(i,bit_length));
    }

    int NTHREADS = arr.size();
    struct child_struct arg[NTHREADS];
    pthread_t tid1[arr.size()];
    for (int i = 0; i < NTHREADS; i++)
    {    
        arg[i].binary_rep = arr[i];
        arg[i].child_server = server;
        arg[i].child_port = portno;
        arg[i].bit = bit_length;
        if (pthread_create(&tid1[i], nullptr, runner, &arg[i]) != 0)
        {
            error((char*)("Problem encountered while creating first set of thread"));
        }
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_join(tid1[i], nullptr) != 0)
        {
        error((char*)("Problem encountered while joining first set of thread"));
        }
    }

    //Final message format
    std::string final_message = "";
    for(int i = 0; i<NTHREADS; i++){
        final_message += arg[i].symbols;
    }
    std::cout << "Decompressed message: " << final_message << std::endl;
    return 0;
}
