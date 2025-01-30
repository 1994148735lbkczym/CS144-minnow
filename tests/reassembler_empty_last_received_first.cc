#include "byte_stream_test_harness.hh"
#include "reassembler_test_harness.hh"

#include <exception>
#include <iostream>

using namespace std;

int main()
{
  try {
    {
      const size_t cap = { 1000 };
      ReassemblerTestHarness test { "empty last index received first", cap };

      test.execute( Insert { "", 4 }.is_last() );
      test.execute( Insert { "abc", 0 } );

      test.execute( BytesPushed( 3 ) );
      test.execute( ReadAll( "abc" ) );
    }
  } catch ( const exception& e ) {
    cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
