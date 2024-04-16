#include "tcp_sender.hh"
#include "byte_stream.hh"
#include "reassembler.hh"
#include "tcp_config.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <ctime>
#include <string_view>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  // return outstadings_;
  return outstadings_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return ret_times_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // (void)transmit;
  while((window_sz == 0 ? 1 : window_sz) > outstadings_){
    if(FINsented_) return;
    auto msg = make_empty_message();
    if(not SYNsented_){
      SYNsented_ = true;
      msg.SYN = true;
    }
    // if(input_.has_error()) msg.RST = true;
    // remaining is total msg's length this time can send
    uint64_t remaining = {(window_sz == 0 ? 1 : window_sz) - outstadings_};
    uint64_t len {min(TCPConfig::MAX_PAYLOAD_SIZE, remaining - msg.sequence_length())};
    auto &payload{msg.payload};
    // if buffered_bytes == 0 means read is over
    while(reader().bytes_buffered() != 0 and payload.size() < len){
      string_view view { reader().peek()};
      view = view.substr(0, len - payload.size());
      payload += view;
      input_.reader().pop(view.size());
    }
    //条件2 信息的总长要小于当前能发送的长度，等于不行，因为还要加入FIN
    if(not FINsented_ and msg.sequence_length() < remaining and reader().is_finished()){
      msg.FIN = true;
      FINsented_ = true;
    }

    if(msg.sequence_length() == 0) return;
    transmit(msg);
    if(not timer_.is_active()){
      timer_.start();
    }
    outstadings_ += msg.sequence_length();
    now_seqno_ += msg.sequence_length();
    outstanding_mes_.emplace(move(msg));
    
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here
  //RST设置位必须在初始化时就根据input进行设置，原因未知
  //在push函数种判断再设置RST位就失效了
  //有可能是在错误时不经过push过程？
  return {Wrap32::wrap(now_seqno_, isn_), false,{}, false, input_.has_error()};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  // (void)msg;
  if(input_.has_error()){
    return;
  }
  if(msg.RST){
    input_.set_error();
    return;
  }
  //一定要在前面！！ 因为可能无ack，但是不一定是无效信息，可能返回windowsz  
  window_sz = msg.window_size;
  //读一下TCPReceiverMessage的定义，接收方在某些情况，如初次接触或结束连接时是不会发ackno的
  if(not msg.ackno.has_value()) return;
  bool ack {false};
  const uint64_t rec_ackseqno {msg.ackno->unwrap(isn_, now_seqno_)};
  //一种容错，可能网络种残存有某些过去的tcp ack确认，它的ackseqno如果都大于了现在已经发送的seqno
  //需要被忽略
  if(rec_ackseqno > now_seqno_) return;
  while(not outstanding_mes_.empty()){
    const auto &r {outstanding_mes_.front()};
    if (acked_seqno + r.sequence_length() <= rec_ackseqno){
      outstadings_ -= r.sequence_length();
      acked_seqno += r.sequence_length();
      outstanding_mes_.pop();
      ack = true;
    }else{
      break;
    }
  }
  if(ack){
    ret_times_ = 0;
    timer_.reload(initial_RTO_ms_);
    if(outstanding_mes_.empty()) timer_.stop();
    else timer_.start();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // (void)ms_since_last_tick;
  // (void)transmit;
  // (void)initial_RTO_ms_;
  if(!timer_.is_active()) return;
  if(timer_.tick(ms_since_last_tick).is_expired()){
    if(outstanding_mes_.empty()) return;
    transmit(outstanding_mes_.front());
    if(window_sz != 0){
      ret_times_++;
      timer_.e_backoff();
    }
    timer_.reset();
  }


}
