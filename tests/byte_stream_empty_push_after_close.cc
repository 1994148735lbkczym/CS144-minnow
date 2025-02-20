#include "byte_stream_test_harness.hh"

#include <exception>
#include <iostream>

using namespace std;

void all_zeroes( ByteStreamTestHarness& test )
{
  test.execute( BytesBuffered { 0 } );
  test.execute( AvailableCapacity { 15 } );
  test.execute( BytesPushed { 0 } );
  test.execute( BytesPopped { 0 } );
}

int main()
{
  try {
    {
      ByteStreamTestHarness test { "insert empty string after close", 15 };
      test.execute( Push { "hello world" } );
      test.execute( Close {} );
      test.execute( IsClosed { true });
      test.execute( Push { "" } );
      test.execute( HasError { false } );
      test.execute( IsClosed { true });
    }

    {
      ByteStreamTestHarness test { "insert empty string after close empty stream", 15 };
      test.execute( Close {} );
      test.execute( IsClosed { true });
      test.execute( Push { "" } );
      test.execute( HasError { false } );
      test.execute( IsFinished { true } );
      all_zeroes( test );
    }
  } catch ( const exception& e ) {
    cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
