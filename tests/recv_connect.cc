#include "byte_stream_test_harness.hh"
#include "reassembler_test_harness.hh"
#include "receiver_test_harness.hh"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>

using namespace std;

int main()
{
  try {
    {
      TCPReceiverTestHarness test { "connect 1", 4000 };
      test.execute( ExpectWindow { 4000 } );
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_syn().with_seqno( 0 ) );
      test.execute( ExpectAckno { Wrap32 { 1 } } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "connect 2", 5435 };
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_syn().with_seqno( 89347598 ) );
      test.execute( ExpectAckno { Wrap32 { 89347599 } } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "connect 3", 5435 };
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_seqno( 893475 ).without_ackno() );
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "connect 4", 5435 };
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_fin().with_seqno( 893475 ).without_ackno() );
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "connect 5", 5435 };
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_fin().with_seqno( 893475 ).without_ackno() );
      test.execute( ExpectAckno { optional<Wrap32> {} } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
      test.execute( SegmentArrives {}.with_syn().with_seqno( 89347598 ) );
      test.execute( ExpectAckno { Wrap32 { 89347599 } } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "connect 6", 4000 };
      test.execute( SegmentArrives {}.with_syn().with_seqno( 5 ).with_fin() );
      test.execute( IsClosed { true } );
      test.execute( ExpectAckno { Wrap32 { 7 } } );
      test.execute( BytesPending { 0 } );
      test.execute( BytesPushed { 0 } );
    }

    {
      TCPReceiverTestHarness test { "window size zero", 0 };
      test.execute( ExpectWindow { 0 } );
    }

    {
      TCPReceiverTestHarness test { "window size 50", 50 };
      test.execute( ExpectWindow { 50 } );
    }

    {
      TCPReceiverTestHarness test { "window size at max", UINT16_MAX };
      test.execute( ExpectWindow { UINT16_MAX } );
    }

    {
      TCPReceiverTestHarness test { "window size at max+1", UINT16_MAX + 1 };
      test.execute( ExpectWindow { UINT16_MAX } );
    }

    {
      TCPReceiverTestHarness test { "window size at max+5", UINT16_MAX + 5 };
      test.execute( ExpectWindow { UINT16_MAX } );
    }

    {
      TCPReceiverTestHarness test { "window size at 10M", 10'000'000 };
      test.execute( ExpectWindow { UINT16_MAX } );
    }

    {
        TCPReceiverTestHarness test { "overlapping payloads", 1000 };

        // sending a SYN to initialize the connection
        test.execute(SegmentArrives {}.with_syn().with_seqno(0));
        test.execute(ExpectAckno { Wrap32 { 1 } });

        // Send a small initial payload
        test.execute(SegmentArrives {}.with_seqno(1).with_data("Hel"));
        test.execute(ExpectAckno { Wrap32 { 4 } }); 

        // Send an overlapping payload with additional data
        test.execute(SegmentArrives {}.with_seqno(3).with_data("loo Werld!"));
        test.execute(ExpectAckno { Wrap32 { 13 } });

        // Verify the stream content and ensure no duplicates
        test.execute(BytesPushed { 12 }); 
        test.execute(BytesPending { 0 });
       
    }

  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return 1;
  }

  return EXIT_SUCCESS;
}
