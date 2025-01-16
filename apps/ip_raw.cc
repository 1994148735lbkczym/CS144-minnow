#include "socket.hh"
#include <string>
#include <iostream>

using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

void send_internet_datagram(const string& payload)
{
  // construct an Internet or user datagram here, and send using the RawSocket as in the Jan. 10 lecture
  string packet;

  packet += char ( 0b0100'0101 );
  packet += string(7, 0);

  packet += char(64);
  // protocol number 5->Raw, 17->UDP
  //packet += char(5);
  packet += char(17);
  packet += string(6, 0);

  // 10.144.0.54
  packet += char(10);
  packet += char(144);
  packet += char(0);
  packet += char(164);

  // UDP INFO
  packet += char( 0 );
  packet += char( 1 );

  packet += char( 4 ); // specify dst port 1024 
  packet += char( 0 );

  packet += char ( 0 );
  packet += char (payload.length() + 8); // package size?

  packet += string(2, 0);


  packet += payload;

  // printf("sent\n");

  string address = "10.144.0.164";

  RawSocket {}.sendto( Address { address }, packet);
}

void send_icmp_message( const string& payload )
{
  send_internet_datagram( "\x08" + payload );
}

void program_body()
{
  string payload;
  while ( cin.good() ) {
	getline( cin, payload );
	send_icmp_message( payload + "\n" );
  }
}

int main()
{
  try {
	program_body();
  } catch ( const exception& e ) {
	cerr << e.what() << "\n";
	return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
