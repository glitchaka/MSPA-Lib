#include "mspa/persistence/serialization.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data,std::size_t size){std::vector<std::uint8_t> bytes(data,data+size);(void)mspa::deserialize_agent(bytes,{4u*1024u*1024u});return 0;}
