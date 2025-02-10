#include "byte_stream_test_harness.hh"
#include "random.hh"
#include "reassembler_test_harness.hh"
#include "receiver_test_harness.hh"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

int main()
{
  try {
    auto rd = get_random_engine();

    {
      const uint32_t isn = uniform_int_distribution<uint32_t> { 0, UINT32_MAX }( rd );
      TCPReceiverTestHarness test { "close 1", 4000 };
      test.execute( HasAckno { false } );
      test.execute( SegmentArrives {}.with_syn().with_seqno( isn + 0 ) );
      test.execute( IsClosed { false } );
      test.execute( SegmentArrives {}.with_fin().with_seqno( isn + 1 ) );
      test.execute( ExpectAckno { Wrap32 { isn + 2 } } );
      test.execute( BytesPending { 0 } );
      test.execute( Peek { "" } );
      test.execute( BytesPushed { 0 } );
      test.execute( IsClosed { true } );
    }

    {
      const uint32_t isn = uniform_int_distribution<uint32_t> { 0, UINT32_MAX }( rd );
      TCPReceiverTestHarness test { "close 2", 4000 };
      test.execute( HasAckno { false } );
      test.execute( SegmentArrives {}.with_syn().with_seqno( isn + 0 ) );
      test.execute( IsClosed { false } );
      test.execute( SegmentArrives {}.with_fin().with_seqno( isn + 1 ).with_data( "a" ) );
      test.execute( IsClosed { true } );
      test.execute( ExpectAckno { Wrap32 { isn + 3 } } );
      test.execute( BytesPending { 0 } );
      test.execute( ReadAll { "a" } );
      test.execute( BytesPushed { 1 } );
      test.execute( IsClosed { true } );
      test.execute( IsFinished { true } );
    }

    {
      const uint32_t isn = uniform_int_distribution<uint32_t> { 0, UINT32_MAX }( rd );
      TCPReceiverTestHarness test { "fin only segment", 2358 };
      // Test: FIN-only segment edge-case.
      // 1. Start connection with SYN.
      // 2. Receive three bytes of contiguous data.
      // 3. Receive a FIN-only segment.
      // Expect: The ACK after the FIN should be ISN + 5.
      //
      // Explanation:
      //   - SYN consumes 1 sequence number.
      //   - "abc" is 3 bytes (consuming 3 sequence numbers).
      //   - FIN consumes 1 sequence number.
      // So, if ISN is the initial sequence number, the final ACK should be:
      //     ISN + 1 (SYN) + 3 (data) + 1 (FIN) = ISN + 5.
      test.execute( SegmentArrives {}.with_syn().with_seqno( isn ) );
      test.execute( ExpectAckno { Wrap32 { isn + 1 } } );

      // Deliver contiguous data "abc" (which occupies bytes isn+1 through isn+3).
      test.execute( SegmentArrives {}.with_seqno( isn + 1 ).with_data( "abc" ) );
      test.execute( ReadAll { "abc" } );
      test.execute( BytesPushed { 3 } );

      // Now, send a FIN-only segment.
      // The FIN should arrive with seqno isn + 4 (after the data).
      test.execute( SegmentArrives {}.with_seqno( isn + 4 ).with_fin() );
      // Correct behavior: the ACK should be isn + 5.
      test.execute( ExpectAckno { Wrap32 { isn + 5 } } );
      test.execute( BytesPending { 0 } );
    }

  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return 1;
  }

  return EXIT_SUCCESS;
}
