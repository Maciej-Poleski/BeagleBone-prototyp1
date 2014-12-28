#include <cerrno>

#include <linux/i2c-dev.h>

#include "vuart.hxx"

constexpr const char *i2cBusDeviceName = "/dev/i2c-1";

static int getWheelFileDescriptor()
{
    static constexpr int wheelAddress = 0x04;
    int file = open(i2cBusDeviceName, O_RDONLY);
    if (file < 0)
    {
        throw std::runtime_error(std::string("Nie udało się uzyskać dostępu do szyny I2C: ") + strerror(errno));
    }
    if (ioctl(file, I2C_SLAVE, wheelAddress) == -1)
    {
        throw std::runtime_error(
                std::string("Nie udało się ustawić adresu slave (") + std::to_string(wheelAddress) + "): " +
                        strerror(errno));
    }
}

// Tak nie da się pracować, stream_descriptor nie jest DefaultConstructible, CopyConstructible, ...
stream_descriptorArray getVuartTranslation(boost::asio::io_service &io_service)
{
    using boost::asio::posix::stream_descriptor;
    return {
            stream_descriptor{io_service},
            stream_descriptor{io_service},
            stream_descriptor{io_service},
            stream_descriptor{io_service},
            stream_descriptor{io_service, getWheelFileDescriptor()},
    };
}