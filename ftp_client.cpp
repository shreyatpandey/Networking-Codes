#include "header_client.h"

class Client
{
public:
unsigned long long file_size(string filename)
{
  streampos fsize = 0;
  ifstream read_file(filename.c_str(),ios::in);
  fsize = read_file.tellg();
  read_file.seekg(0,ios::end);
  fsize  = read_file.tellg() - fsize;
  read_file.close();
  return fsize;
}

void store_file_content()
{
//	cout<<"Store_file_content"<<endl;
    pthread_mutex_lock(&lock);
    ifstream file_pointer(path.c_str());
    if(!(file_pointer.is_open()))
    {
      perror("Error in opening file");
      pthread_mutex_unlock(&lock);
      exit(-1);
    } 
    size_file = file_size(path.c_str());
    cout<<"File_Size:"<<size_file<<endl;
    file_point_holder = open(path.c_str(),O_RDONLY);
    data = static_cast<char*>( mmap((caddr_t)0,size_file, PROT_READ, MAP_SHARED, file_point_holder, 0));
    if (data == (caddr_t)(-1)) 
	{
        perror("mmap");
        pthread_mutex_unlock(&lock);
        exit(-1);
    }
    pthread_mutex_unlock(&lock);
}

void break_packet(packet_parameter packet)
{
  usleep(100); 
  size_t send_value = sendto(socket_udp,&packet,PACKET_SIZE,0,(struct sockaddr *)&addr,sizeof(addr));
//  cout<<"Packets are been transmitted"<<endl;	
  if(send_value < 0)
    {
     cout<<"Error in sending value from client side"<<endl;
     exit(-1);
    }

}


void tcp_packet_info()
{
//	cout<<"tcp_packet_info"<<endl;
    size_t sock_tcp, write_tcp; 
    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_tcp < 0)
   {
     cout<<"Error in opening socket"<<endl;
     exit(-1);
    }
    server_tcp = gethostbyname(SERVER_PORT_NO);
    struct sockaddr_in serv_addr_t;
    bzero((char *) &serv_addr_t, sizeof(serv_addr_t));
    serv_addr_t.sin_family = AF_INET;
    bcopy((char *)server_tcp->h_addr,(char *)&serv_addr_t.sin_addr.s_addr,server_tcp->h_length);
    serv_addr_t.sin_port = htons(TCP_PORT_NO_CLIENT);
 	serv_addr_t.sin_addr.s_addr = inet_addr(IP);
    if (connect(sock_tcp,(struct sockaddr *)&serv_addr_t,sizeof(serv_addr_t)) < 0)
   {
    cout<<"Error in connecting tcp socket"<<endl;
    exit(-1);
    }
    
    write_tcp = write(sock_tcp,(void *)&size_file,sizeof(size_file));
    if (write_tcp < 0)
    {
      cout<<"Error writin into tcp socket"<<endl;
      exit(-1);
     }
    close(sock_tcp);
 }
};

void* packet_retransmission(void* packet_input)
{
  Client c;
  while(true)
 {
  //	cout<<"packet_retransmission"<<endl;
    int packet_retransmit,seq;
    int size = PAYLOAD_SIZE;
    packet_retransmit  = recvfrom(socket_udp,&seq,sizeof(int),0,(struct sockaddr *)&addr,&length_udp_socket);
    if (packet_retransmit < 0)
     {
        cout<<"Error in UDP socket"<<endl;
        exit(-1);
      }
//		cout<<"Seq:"<<seq<<endl;         
        if(seq == -1)
        {
            cout<<"Last byte transmitted"<<endl;
            pthread_exit(0);
        }
        if((seq == (number_of_packets_transmit-1)) && (0 != size_file % PAYLOAD_SIZE))
        {    
			size = size_file % PAYLOAD_SIZE;
		}
        packet_parameter packet_new;
        memset(packet_new.payload,'\0',PAYLOAD_SIZE+1);
        packet_new.sequence_number = seq;
        memcpy(packet_new.payload,data+(seq*PAYLOAD_SIZE),size);
      //  packet_retransmission(&packet_new);
        c.break_packet(packet_new);
 //	cout<<"packet_retransmission_over"<<endl;
    }

 }




int main()
{
 Client c1;  
  
//creation of UDP-socket in client side

socket_udp = socket(AF_INET,SOCK_DGRAM,0);
if(socket_udp < 0)
{
  perror("Error in creating socket");
  exit(1);
}
memset(&addr,0,sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = inet_addr(IP);
addr.sin_port = htons(PORT_NO);
length_udp_socket = sizeof(addr);
uint64_t udp_buffer_size = 500000000;
if((setsockopt(socket_udp,SOL_SOCKET,SO_SNDBUF,&udp_buffer_size, sizeof(uint64_t))) == -1)
{
  cout<<"Error in setting socket"<<endl;
  exit(-1);
    
}

c1.store_file_content();
c1.tcp_packet_info();


if((thread_error=pthread_create(&packet_retransmit,NULL,packet_retransmission,NULL)))
{
        cout<<"Error in creating thread"<<endl;
        exit(-1);
}
    packet_parameter packet_initial;
    memset(packet_initial.payload,'\0',PAYLOAD_SIZE+1);
    if((size_file % PAYLOAD_SIZE) != 0)
        number_of_packets_transmit = (size_file/PAYLOAD_SIZE)+1;
    else
        number_of_packets_transmit = (size_file/PAYLOAD_SIZE);
    while(sequence_number_hold < number_of_packets_transmit)
      {
        packet_initial.sequence_number = sequence_number_hold;
        if((sequence_number_hold == (number_of_packets_transmit-1)) && ((size_file % PAYLOAD_SIZE) != 0))
           {
        	memcpy(packet_initial.payload,data+off_set,(size_file % PAYLOAD_SIZE));
        	}
        else{
        	memcpy(packet_initial.payload,data+off_set,PAYLOAD_SIZE);
        	}
        sequence_number_hold+=1;
        memcpy(packet_initial.payload,data+off_set,PAYLOAD_SIZE);
        off_set = off_set + PAYLOAD_SIZE;
        c1.break_packet(packet_initial);
    }
pthread_join(packet_retransmit,NULL);
munmap(data,size_file);

close(file_point_holder);
return 0;

}
