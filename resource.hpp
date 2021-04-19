#ifndef SIK_SKORELOWANE_SERWERY_RESOURCE_HPP
#define SIK_SKORELOWANE_SERWERY_RESOURCE_HPP

#include "tcp_server.hpp"

class resource {
  public:
    resource(basic_configuration const& config, std::string const& path);
};

#endif // SIK_SKORELOWANE_SERWERY_RESOURCE_HPP
