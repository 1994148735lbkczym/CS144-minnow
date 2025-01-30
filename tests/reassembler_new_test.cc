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
      ReassemblerTestHarness test { "empty string pushed before byte pushed at "syn bit" location", 4 };
      test.execute( Insert { "", 0 } );
      test.execute( Insert { "a", 18446744073709551615 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 0 ) );
  } catch ( const exception& e ) {
    cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
return EXIT_SUCCESS;
}
