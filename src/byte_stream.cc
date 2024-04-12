#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  // (void)data;

  if(is_closed_) return;
  if(data.empty() || available_capacity() == 0) return;
  if(data.size() > available_capacity()) data = data.substr(0, available_capacity());
  bytes_buffered_ += data.size();
  bytes_pushed_ += data.size();
  buf.push(std::move(data));
  if(!buf.empty() && view_front.empty()) view_front = buf.front();
  return;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return buf.empty() && is_closed_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}

string_view Reader::peek() const
{
  // Your code here.
  return view_front;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  // (void)len;
  len = min(len, bytes_buffered_);
  bytes_popped_ += len;
  while(len > 0){
    if(len >= view_front.size()){
      len -= view_front.size();
      bytes_buffered_ -= view_front.size();
      buf.pop();
      view_front = buf.empty()? ""sv : buf.front();
    }else{
      view_front.remove_prefix(len);
      bytes_buffered_ -= len;
      len = 0;
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytes_buffered_;
}
