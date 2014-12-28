#include <boost/asio/posix/stream_descriptor.hpp>

// Type Erasure...
typedef std::array<boost::asio::posix::stream_descriptor, 5> stream_descriptorArray;

stream_descriptorArray getVuartTranslation(boost::asio::io_service &io_service);
