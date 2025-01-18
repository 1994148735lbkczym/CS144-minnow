#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Initialize a index for looping through the data string
  size_t data_index = 0;

  // CASE: if data string overlaps with data that has already been added to the reassembler/bytestream 
  //    -> skip bytes until we reach new information
  while ((int64_t) first_index <= recent_index && data_index < data.length()) {
    // Remove from saved -> lower storage cost
    saved_bytes.erase(first_index);
    // Update indexes to traverse data
    first_index++;
    data_index++;
  }
  
  // Update data + data_index for simplicity. Does not increase runtime since we are looping through all data bytes anyway O(n)
  data = data.substr(data_index);
  data_index = 0;

  // CASE: insert relevent data bytes into the bytestream only if there is capacity and our string has the first_index starting position
  if (data_index < data.length() && (int64_t)first_index == recent_index + 1) {
    // derive stream capacity to update index information
    size_t stream_capacity = output_.writer().available_capacity();
    // attempt to insert data; data not guarenteed to be all added
    output_.writer().push( data ); 

    // calculate update_size : min(capacity, stringSize), represents number of bytes added to stream
    uint64_t update_size = min(stream_capacity, data.length());

    // remove from storage -> reduce storage space
    for (uint64_t i = first_index; i < first_index + update_size; i++) {
      saved_bytes.erase(i);
    }

    //update counters using update_size var
    first_index += update_size;
    data_index += update_size;
    recent_index = first_index - 1; // first_index represents unadded
  }

  // CASE: All Data skipped or added to stream + lastSubstring flag raised
  if (data_index == data.length() && is_last_substring) {
    output_.writer().close();
    return;
  }

  // CASE: Could not add bytes (ie. holes) -> store as many as capacity allows
  if (data_index < data.length() && output_.writer().available_capacity() != 0) {
    // max_index stores max index (inclusive) of bytes that fit in our reassembler 
    uint64_t max_index = recent_index + output_.writer().available_capacity();
    // loop from beginning, storing foremost bytes first, loop ends at max_index
    while (data_index < data.length() && first_index <= max_index) {
      // check if we are storing the "lastByte" of our message
      bool isLastByte = data_index == (data.length() - 1) && is_last_substring;
      // store data in reassembler
      std::pair<char, bool> info = {data[data_index], isLastByte};
      saved_bytes[first_index] = info;

      // update counters
      first_index++;
      data_index++;
    }
  }


  // CASE: Update Bytestream with relevant saved bytes from Reassembler, starting from recent_index+1 or the first index missing
  //      -> useful for holes being filled
  while (saved_bytes.contains(recent_index + 1)) {
    // store saved bytes locally and push
    std::pair<char, bool> info = saved_bytes[recent_index + 1];
    bool isLast = info.second;
    string this_msg = string(1, info.first);
    output_.writer().push(this_msg);
    // remove from storage -> save storage space
    saved_bytes.erase(recent_index + 1);
    recent_index++;
    // if we add the last byte -> end connection and close
    if (isLast) {
      output_.writer().close();
      return;
    }
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return saved_bytes.size();
}
