#include "socket.hh"

using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

int main()
{
  // construct an Internet or user datagram here, and send using the RawSocket as in the Jan. 10 lecture
  string packet;

  packet += char ( 0b0100'0101 );
  packet += string(7, 0);

  packet += char(64);
  // protocol number 5->Raw, 17->UDP
  packet += char(5);
  //packet += char(17);
  packet += string(6, 0);

  // 10.144.0.54
  packet += char(10);
  packet += char(144);
  packet += char(0);
  packet += char(54);

  // UDP INFO
  // packet += char( 0 );
  // packet += char( 1 );

  // packet += char( 4 ); // specify dst port 1024 
  // packet += char( 0 );

  // string user_payload = "Hellooo";

  // packet += char ( 0 );
  // packet += char (user_payload.length() + 8);

  // packet += string(2, 0);


  packet += "Hellooo";

  printf("sent\n");


  RawSocket {}.sendto( Address { "10.144.0.54" }, packet);
  return 0;
}
