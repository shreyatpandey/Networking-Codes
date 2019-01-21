#include "header_server.h"

class Server
{
public:
void tcp_packet_info()
{
    size_t socket_tcp;
    struct sockaddr_in server_addr_tcp,client_addr_tcp;
    socklen_t client_addr_tcp_length = sizeof(client_addr_tcp); 
    socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    int opt=1;
    if (socket_tcp < 0)
   {
     cout<<"Error in opening socket"<<endl;
     exit(-1);
    }
    if (setsockopt(socket_tcp, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))                                                
    {
        perror("setsockopt");
        exit(-1);
    }
    bzero((char *)&server_addr_tcp,sizeof(server_addr_tcp));    
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(tcp_port_no_client);
    server_addr_tcp.sin_addr.s_addr = inet_addr(IP);

    int bind_tcp = bind(socket_tcp,(struct sockaddr *)&server_addr_tcp,sizeof(server_addr_tcp));
    if(bind_tcp < 0)
   {
     perror("Error in binding the socket");
     exit(-1);
   }
 
   if(listen(socket_tcp,3)<0)
   {
      perror("Error in listening");
      exit(-1);
    }
    int accept_socket = accept(socket_tcp, (struct sockaddr *)&client_addr_tcp,&client_addr_tcp_length);
    if(!accept_socket)
    {
      perror("Error in accepting socket");
      exit(-1);
     }
    cout<<"read_socket"<<endl;
     int read_socket = read(accept_socket,&size_file,255);
     if(!(read_socket))
     {
       perror("Error in reading socket");
       exit(-1);
      }
    
    close(accept_socket);
    close(socket_tcp);
} 
int updateTrackPacketsArray(int sequence_number)
{
    if(sequence_number>= 0 && sequence_number < number_of_packets)
   {
        if(track_packets[sequence_number] == 0)
        {
            track_packets[sequence_number] = 1;
            return 1;
        }
        else return 0;

    }
    return 0;
}
int check_all_pckt_rcvd()
{
   cout<<"total_count:"<<number_of_packets<<endl;
   int value = (total_count == number_of_packets)? 1:0;
   return value;
}
void send_end_s()
{
	cout<<"send_end_s"<<endl;
         int end_data = -1;

        for (int i = 0 ; i < 10 ; i++)
        {
          int n = sendto(socket_udp,&end_data,sizeof(int), 0,(struct sockaddr *) &client_addr,client_addr_length);
        if (n < 0) 
         {
           cout<<"Error in sending_value"<<endl;
           exit(-1);
         }
        }
        close(socket_udp);

}




void receive_packets()
{
    int check_receive_client = 0;
    packet_parameter receive_packet; //packet_parameter receive_packet
    long write_position;
   // client_addr_length = sizeof(client_addr);
    while (1)
    {
        cout<<"received_packet"<<endl;
        check_receive_client = recvfrom(socket_udp,&receive_packet,1500,0,(struct sockaddr *) &client_addr,&client_addr_length);
		cout<<"just_after_recieve_client"<<endl;
        if (!check_receive_client) 
        {
         cout<<"Error in recieving packet"<<endl;
         exit(-1);
         }
      // cout<<"receive_client_clear"<<endl;
         pthread_mutex_lock(&lock);
	//	cout<<"after_mutex"<<endl;      
        if (updateTrackPacketsArray(receive_packet.sequence_number))
        {
		//	cout<<"Before segmentation fault"<<endl;
        	track_packets[receive_packet.sequence_number] = 1;	
			//cout<<"After segment fault"<<endl;
                if(receive_packet.sequence_number > last_index)
                {
                  last_index = receive_packet.sequence_number;
                
                }
                write_position = (receive_packet.sequence_number)* payload_size;
                fseek( fp_s , write_position , SEEK_SET  );
                if(receive_packet.sequence_number == (number_of_packets - 1) )
                {
                    fwrite(&receive_packet.payload , last_packet_size , 1 , fp_s);
                    fflush(fp_s);
                 }
                else
                {
                    fwrite(&receive_packet.payload ,payload_size,1,fp_s);
                    fflush(fp_s);
                 }
                 total_count+=1;
        
        }
        pthread_mutex_unlock(&lock);
        if(check_all_pckt_rcvd() == 1)
        {
            cout<<"Last byte received"<<endl;
            send_end_s();
            break;
        }
       if(last_index >= 0.8 * number_of_packets)
       {
            startcallback = 1;
       }
    }

}

int loop_index;

int getNackSeqNum()
{

    if (track_packets.empty()) 
    {
       return -1;
    }

    for (int i = loop_index; i < number_of_packets ; i++)
    {
        if(track_packets[i] == 0)
        {
            if( i == number_of_packets - 1) 
            {
             loop_index = start_index;
            }
            else 
            {
              loop_index = i+1;
            }
            return i;
        }
    }
    loop_index = start_index;
    return -1;
}


void send_nack_to_client(int sequence_number)
{
   cout<<"send_negative_acknowledgement"<<endl;
	int nack_hold;
    nack_hold = sendto(socket_udp, &sequence_number, sizeof(int), 0,(struct sockaddr *) &client_addr,client_addr_length);
    if (nack_hold < 0) 
    {
     cout<<"Negative Acknowledgement send error:"<<endl;
     exit(-1);
     }
}
};



void* handleFailures(void *a)
{
	Server s;
    while(true)
    {
        if(startcallback)
        {
           cout<<"handle_failure_called"<<endl;
            usleep(100);
            int actual_last_index = 0;
            if(last_index > 0.7 * number_of_packets)
            {
              actual_last_index = last_index + 0.3 * number_of_packets;
            }
            else
            {
                actual_last_index = last_index;
            }
            if(s.check_all_pckt_rcvd() == 1)
            {
                pthread_exit(0);
            }
            for(int i =  start_index; i<= actual_last_index && i < number_of_packets ; i++)
            {
              if(track_packets[i] == 1)
                {
                  start_index+=1;
                }
              else break;
            }

            int required_sequence_number = s.getNackSeqNum();

            if(required_sequence_number >= 0 && required_sequence_number< number_of_packets)
            {
              s.send_nack_to_client(required_sequence_number);
            }
        }
    }
}




int main()
{
Server s1;
//creation of UDP socket on server side
socket_udp = socket(AF_INET,SOCK_DGRAM,0);
if(socket_udp < 0)
{
  perror("Error in creating socket");
  exit(1);
}
unsigned long long socket_buffer_size = 100000000;
//memset(&server_addr,0,sizeof(server_addr));
//memset(&client_addr,0,sizeof(client_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = inet_addr(IP);
server_addr.sin_port = htons(port_no);

//client_addr.sin_family = AF_INET;
//client_addr.sin_addr.s_addr - inet_addr(IP);
//client_addr.sin_port = htons(port_no);
size_t bind_udp = bind(socket_udp,(struct sockaddr *)&server_addr,sizeof(server_addr));
if(bind_udp < 0)
{
  perror("Error in binding the socket");
  exit(-1);
}
if((setsockopt(socket_udp,SOL_SOCKET,SO_SNDBUF,&socket_buffer_size, sizeof(unsigned long long))) == -1)
{
	cout<<"Error in send socket"<<endl;
	exit(-1);
}
if((setsockopt(socket_udp,SOL_SOCKET,SO_RCVBUF,&socket_buffer_size, sizeof(unsigned long long))) == -1)
{
  cout<<"Error in setting socket"<<endl;
  exit(-1);    
}
    s1.tcp_packet_info();
    filedata = (char *) malloc (size_file);
    fp_s = fopen(path.c_str() , "w+"); //need changes
   

    if(size_file % payload_size != 0)
    {
        number_of_packets = size_file/payload_size + 1;
      // cout<<"number_of_packets:"<<number_of_packets<<endl;
        last_packet_size = size_file%payload_size;
    }
    else
    {
        number_of_packets = size_file/payload_size;
 	//	cout<<"number_of_packets:"<<number_of_packets<<endl;
        last_packet_size = payload_size;
    }
    
    if((thread_create_error = pthread_create(&packet_nack, NULL, handleFailures, NULL )))
    {
        cout<<"Eror in creating thread"<<endl;
        pthread_exit(0);
    }
  //  track_packets = (int *)calloc(number_of_packets, sizeof (int));
	track_packets.resize(number_of_packets);
    s1.receive_packets();
    pthread_join(packet_nack, NULL);
        
    client_addr_length = sizeof(struct sockaddr_in);

    fclose(fp_s);
    close(socket_udp);
    return 0;
}

