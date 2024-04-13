#include "tcp_receiver.hh"
#include "byte_stream.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <locale>
#include <optional>


using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.RST) reader().set_error();
  if(not isn.has_value()){
    if(not message.SYN) return;
    //SYN
    isn.emplace(message.seqno);
  }
  const uint64_t checkpoint {writer().bytes_pushed() + 1 /* SYN */};
  const uint64_t absolute_seq{message.seqno.unwrap(isn.value(), checkpoint)};
  const uint64_t stream_indx{absolute_seq - 1 + static_cast<uint64_t>(message.SYN)};
  reassembler_.insert(stream_indx, message.payload, message.FIN);

}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  uint16_t window_sz;
  uint64_t cap = writer().available_capacity();
  if(cap > UINT16_MAX) window_sz = static_cast<uint16_t>(UINT16_MAX);
  else window_sz = static_cast<uint16_t>(cap);
  if(isn.has_value()){
    const uint64_t ack = writer().bytes_pushed() + 1 /* SYN*/ + static_cast<uint64_t>(writer().is_closed());
    return {Wrap32::wrap(ack, isn.value()), window_sz, writer().has_error()};
  }
  return {nullopt, window_sz, writer().has_error()};
}
