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

#define port_no 24882
#define IP "10.1.1.2"
#define tcp_port_no_client 2345
#define server_port_no 25882
#define payload_size 1450
#define packet_size 1454

//thread characteristics
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t packet_nack;
size_t thread_create_error;

//file_characteristics
string path = "/tmp/output_file.txt";
unsigned long long size_file = 0;
int file_point_holder;
FILE* fp_s;
char *filedata;

//UDP socket characteristics
size_t socket_udp;
struct sockaddr_in server_addr,client_addr;
socklen_t client_addr_length;


//packet characteristics
struct packet_parameter
{
  int sequence_number;
  char payload[payload_size + 1];
};
int sequence_number = 0;
int last_index = 0;
int start_index = 0;
int number_of_packets = 0;
int total_count=0;
int startcallback = 0;
int last_packet_size;
vector<long long int>track_packets;
//int* track_packets;
