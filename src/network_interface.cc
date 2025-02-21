#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

#define five_sec 5000
#define thirty_sec 30000

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
  // INIT Vars
  uint32_t key = next_hop.ipv4_numeric();
  EthernetFrame toSend;
  toSend.header.type = toSend.header.TYPE_IPv4;
  toSend.header.src = ethernet_address_;
  toSend.payload = serialize( dgram );
  if ( ip_cache_.find( key ) != ip_cache_.end() && !ip_cache_[key].isARP ) {
    toSend.header.dst = ip_cache_[key].eth;
    transmit( toSend );
    return;
  }
  // CASE 2: DST_IP is unkown; transmit an ARP and queue the datagram
  /*
    Check Map
    Check ARP Recency
    Queue Somewhere? (N/A)
  */
  // If ARP has been sent and is recent enough
  if ( ip_cache_.find( key ) != ip_cache_.end() && ip_cache_[key].isARP ) {
    if ( ip_cache_[key].last_arp_sent + five_sec >= clock_ ) {
      std::pair<size_t, EthernetFrame> queued_dg = { clock_, toSend };
      ip_cache_[key].ARP_msgs.push( queued_dg );
      return;
    }
  }
  // Assume we need to resend / send for the first time
  ARPMessage arp; // INIT Arp Message
  arp.target_ip_address = next_hop.ipv4_numeric();
  arp.sender_ethernet_address = ethernet_address_;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.opcode = arp.OPCODE_REQUEST;
  EthernetFrame arp_eth; // INIT Eth Frame
  arp_eth.header.type = arp_eth.header.TYPE_ARP;
  arp_eth.header.src = ethernet_address_;
  arp_eth.header.dst = ETHERNET_BROADCAST;
  arp_eth.payload = serialize( arp );
  transmit( arp_eth );
  ip_cache_[key].last_arp_sent = clock_; // Update State
  std::pair<size_t, EthernetFrame> queued_dg = { clock_, toSend };
  ip_cache_[key].ARP_msgs.push( queued_dg );
  ip_cache_[key].isARP = true;
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  // CASE 1: Recieve an IPV4; parse and push to datagrams_recieved queue (given)
  if ( frame.header.type == frame.header.TYPE_IPv4 && frame.header.dst == ethernet_address_ ) {
    InternetDatagram internet_datagram;
    if ( parse( internet_datagram, frame.payload ) ) {
      // if (internet_datagram.header.dst == ip_address_.ipv4_numeric()) {
      datagrams_received_.push( internet_datagram );
      //}
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
  if ( parse( arp, frame.payload ) ) { // Attempt Parse
    // Save arp information regardless of type
    uint32_t key = arp.sender_ip_address;
    ip_cache_[key].eth = arp.sender_ethernet_address;
    ip_cache_[key].isARP = false;
    ip_cache_[key].last_arp_sent = clock_;
    pair<int, uint32_t> queue_entry { clock_, key };
    ip_time_queue_.push( queue_entry );
    // If Request && We are target
    if ( arp.opcode == arp.OPCODE_REQUEST && arp.target_ip_address == ip_address_.ipv4_numeric() ) {
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
      arp_eth.payload = serialize( reply );
      transmit( arp_eth );
    }
    // Send stalled messages
    // CORRECT SECTION BELOW
    while ( ip_cache_[key].ARP_msgs.size() != 0 ) {
      std::pair<size_t, EthernetFrame> queued_dg = ip_cache_[key].ARP_msgs.front();
      // EthernetFrame msg = ip_cache_[key].ARP_msgs.front();
      ip_cache_[key].ARP_msgs.pop();
      // Only want to send up to date messages
      if ( queued_dg.first + five_sec > clock_ ) {
        queued_dg.second.header.dst = arp.sender_ethernet_address;
        transmit( queued_dg.second );
      }
    }
    // CORRECT SECTION ABOVE
    // Incorrect Section  (included for my custom test case) UNCOMMENT SECTION BELOW
    // } else {
    // // Send stalled messages
    //   while (ip_cache_[key].ARP_msgs.size() != 0) {
    //     std::pair<size_t, EthernetFrame> queued_dg = ip_cache_[key].ARP_msgs.front();
    //     // EthernetFrame msg = ip_cache_[key].ARP_msgs.front();
    //     ip_cache_[key].ARP_msgs.pop();
    //     // Only want to send up to date messages
    //     if (queued_dg.first + five_sec > clock_) {
    //       queued_dg.second.header.dst = arp.sender_ethernet_address;
    //     transmit( queued_dg.second );
    //     }
    //   }
    // }
    // Incorrect Section UNCOMMENT SECTION ABOVE
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // update time
  clock_ += ms_since_last_tick;

  // Expire any expired IP(s); >30 seconds storage time
  // Keep popping from queue until lead entry has insertion time >= curTime - 30
  while ( ip_time_queue_.size() != 0 && ip_time_queue_.front().first + thirty_sec < clock_ ) {
    std::pair<size_t, uint32_t> front_elem = ip_time_queue_.front();
    ip_time_queue_.pop();
    // Expire IPs if they are not ARP
    if ( front_elem.first == ip_cache_[front_elem.second].last_arp_sent && !ip_cache_[front_elem.second].isARP ) {
      // if (!ip_cache_[front_elem.second].isARP) {
      ip_cache_.erase( front_elem.second );
    }
  }
}
