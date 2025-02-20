#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
  , clock_( 0 )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // CASE 1: DST_IP is known; then package and transmit
  /*
    Check Map
    Package
    Transmit
  */
  // INIT Map Key
  uint32_t key = next_hop.ipv4_numeric();
  // INIT Ethernet Frame
  EthernetFrame toSend;
  toSend.header.type = toSend.header.TYPE_IPv4;
  toSend.header.src = ethernet_address_;
  toSend.payload = serialize( dgram );
  if (ip_cache_.find(key) != ip_cache_.end() && !ip_cache_[key].isARP) {
    toSend.header.dst = ip_cache_[key].eth;
    transmit(toSend);
    // Update Cache
    ip_cache_[key].last_dg_sent = clock_;
    return;
  }
  // CASE 2: DST_IP is unkown; transmit an ARP and queue the datagram
  /*
    Check Map
    Check ARP Recency
    Queue Somewhere?
  */
 // If ARP has been sent and is recent enough
  if (ip_cache_.find(key) != ip_cache_.end() && ip_cache_[key].isARP) {
    if (ip_cache_[key].last_dg_sent >= clock_ - 5) {
      return;
    } 
  }
  // Assume we need to resend / send for the first time
  ARPMessage arp;
  arp.target_ip_address = next_hop.ipv4_numeric();
  arp.sender_ethernet_address = ethernet_address_;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.opcode = arp.OPCODE_REQUEST;
  EthernetFrame arp_eth;
  arp_eth.header.type = arp_eth.header.TYPE_ARP;
  arp_eth.header.src = ethernet_address_;
  arp_eth.header.dst = ETHERNET_BROADCAST;
  arp_eth.payload = serialize ( arp );
  transmit( arp_eth );
  ip_cache_[key].last_dg_sent = clock_;
  ip_cache_[key].ARP_msgs.push(toSend);
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  // CASE 1: Recieve an IPV4; parse and push to datagrams_recieved queue (given)
  if (frame.header.type == frame.header.TYPE_IPv4) {
    InternetDatagram internet_datagram;
    if ( parse( internet_datagram, frame.payload ) ) {
      datagrams_received_.push(internet_datagram);
    }
    // Return if success or failiure
    return;
  }
  // CASE 2A/B: Recieve an ARP Message; parse and remember the IP for 30 seconds
  /*
    Check if Reply/Recieve (How?)
    Update IP
      Update Dictionary Entry
      Re-Add entry to queue (potentially stores duplicates)
    If Recieve:
      Send the queued messages?
  */
  ARPMessage arp;
  if ( parse( arp, frame.payload ) ) {
    // Save arp information
    uint32_t key = arp.sender_ip_address;
    ip_cache_[key].eth = arp.sender_ethernet_address;
    ip_cache_[key].isARP = false;
    ip_cache_[key].last_dg_sent = clock_;
    // If Reply
    if (arp.opcode == arp.OPCODE_REQUEST && arp.target_ip_address == ip_address_.ipv4_numeric()) {
      // SEND: Arp Reply
      ARPMessage reply;
      reply.target_ip_address = arp.sender_ip_address;
      reply.target_ethernet_address = arp.sender_ethernet_address;
      reply.sender_ip_address = ip_address_.ipv4_numeric();
      reply.sender_ethernet_address = ethernet_address_;
      reply.opcode = arp.OPCODE_REPLY;
      EthernetFrame arp_eth;
      arp_eth.header.type = arp_eth.header.TYPE_ARP;
      arp_eth.header.src = ethernet_address_;
      arp_eth.header.dst = arp.sender_ethernet_address;
      arp_eth.payload = serialize ( reply );
      transmit( arp_eth );
    } else {
      // Send stalled messages
      while (ip_cache_[key].ARP_msgs.size() != 0) {
        EthernetFrame msg = ip_cache_[key].ARP_msgs.front();
        ip_cache_[key].ARP_msgs.pop();
        msg.header.dst = arp.sender_ethernet_address;
        transmit( msg );
      }
    }
  }
  debug( "unimplemented recv_frame called" );
  (void)frame;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // update time
  clock_ += ms_since_last_tick;

  // Expire any expired IP(s); >30 seconds storage time
  // Keep popping from queue until lead entry has insertion time >= curTime - 30
  while (ip_time_queue_.size() != 0 && ip_time_queue_.front().first < clock_ - 30) {
    std::pair<size_t, uint32_t> front_elem = ip_time_queue_.front();
    ip_time_queue_.pop();
    // Expire IPs if they are not ARP
    if (front_elem.first ==  ip_cache_[front_elem.second].last_dg_sent && !ip_cache_[front_elem.second].isARP) {
      ip_cache_.erase(front_elem.second);
    }
  }
}
