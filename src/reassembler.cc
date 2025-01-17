#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  //debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );

  size_t data_index = 0;
  // case: overlap
  while ((int64_t) first_index <= recent_index && data_index < data.length()) {
    // skip and remove from saved
    saved_bytes.erase(first_index);
    first_index++;
    data_index++;
  }
  
  // update data + data_index
  data = data.substr(data_index);
  data_index = 0;

  // case: insert
  if (data_index < data.length() && (int64_t)first_index == recent_index + 1) {
    // insert data
    size_t stream_capacity = output_.writer().available_capacity();
    output_.writer().push( data ); 

    // calculate update size : min(capacity, stringSize)
    uint64_t update_size = min(stream_capacity, data.length());

    // remove from storage
    for (uint64_t i = first_index; i < first_index + update_size; i++) {
      saved_bytes.erase(i);
    }

    //update counters
    first_index += update_size;
    data_index += update_size;
    recent_index = first_index - 1; // first_index represents unadded

    
  }
  // if we were able to read all + we got last flag
  if (data_index == data.length() && is_last_substring) {
    output_.writer().close();
    return;
  }

  // case: gap data -> store valid bytes
  if (data_index < data.length() && output_.writer().available_capacity() != 0) {
    // max_index stores max index inclusive
    uint64_t max_index = recent_index + output_.writer().available_capacity();
    while (data_index < data.length() && first_index <= max_index) {
      // store data
      bool isLastByte = data_index == (data.length() - 1) && is_last_substring;
      std::pair<char, bool> info = {data[data_index], isLastByte};
      saved_bytes[first_index] = info;

      // update counters
      first_index++;
      data_index++;
    }
  }

  // if last string, close string, not if there is extra stored
  // if (is_last_substring) {
  //   output_.writer().close();
  //   return;
  // }


  // case: gap data -> pop from dict
  while (saved_bytes.contains(recent_index + 1)) {
    std::pair<char, bool> info = saved_bytes[recent_index + 1];
    bool isLast = info.second;
    string this_msg = string(1, info.first);
    output_.writer().push(this_msg);
    // remove from storage
    saved_bytes.erase(recent_index + 1);
    recent_index++;

    if (isLast) {
      output_.writer().close();
      return;
    }
  }

  // edit: find first unpopped index using reader.bytespopped (if 0 then 0, etc), and use that to find the first unacceptableindex
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  // debug( "unimplemented count_bytes_pending() called" );
  return saved_bytes.size();
}
