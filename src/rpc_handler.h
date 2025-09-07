#pragma once

#include "protocol_handler.h"

class RpcHandler : public ProtocolHandler {
 public:
  using ProtocolHandler::ProtocolHandler;

 protected:
  void HandleRead() override {}
  void HandleWrite() override {}
};
