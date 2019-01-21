#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<fstream>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<fcntl.h>
#include<netdb.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
using namespace std;

#define PORT_NO 24882
#define IP "10.1.1.2"
#define TCP_PORT_NO_CLIENT 2345
#define SERVER_PORT_NO "25882"
#define PAYLOAD_SIZE 1450
#define PACKET_SIZE 1454


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t packet_retransmit;

//file_characteristics
string path = "/tmp/input_file.txt";
ifstream input_file;
unsigned long long size_file = 0;
int file_point_holder;
char *data;

//UDP socket
size_t socket_udp;
struct sockaddr_in addr;
socklen_t length_udp_socket;

//tcp socket
socklen_t formlen;
struct hostent *server_tcp;

struct packet_parameter
{
  int sequence_number;
  char payload[PAYLOAD_SIZE + 1];
};
int number_of_packets_transmit = 0;
int sequence_number_hold = 0;
int off_set = 0;
size_t thread_error;
