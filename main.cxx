#include <iostream>

#include <boost/asio/spawn.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/system/system_error.hpp>

namespace asio = boost::asio;

constexpr std::uint16_t portNumber = 1234;

/**
 * Mogą istnieć różne kanały. Każdy może transmitować done o określonym
 * priorytecie. Każdy strumień danych ma określony priorytet.
 *
 * Ta funkcja docelowo będzie zastąpiona logiką przygotowującą kanały
 * i wybierającą odpowiedni w zależności od typu danych który chcemy
 * transmitować. Tutaj będzie również mechanizm przełączania kanałów.
 */
asio::ip::tcp::socket getBaseStationStream(
    asio::io_service &io_service,
    asio::yield_context yield)
{
    using namespace asio::ip;
    tcp::endpoint e(tcp::v4(), portNumber);
    tcp::acceptor acc(io_service, e);

    tcp::socket result(io_service);
    acc.async_accept(result, yield);
    return std::move(result);
}

/**
 * Szablon...
 */
void handleNextData(asio::ip::tcp::socket &incomingStream, asio::yield_context yield)
{
    using namespace asio;
    std::uint8_t packSize;
    async_read(incomingStream, buffer(&packSize, 1), yield);
    std::cout << "Odebrano paczke wielkości: >" << unsigned(packSize) << "<\n" << std::flush;
    for (unsigned i = 0; i < packSize; ++i)
    {
        std::uint8_t idAndSize[2];
        async_read(incomingStream, buffer(idAndSize, 2), yield);
        std::cout << "Wiadomość [" << i << "] dla >" << unsigned(idAndSize[0]) << "< o długości >" << unsigned(idAndSize[1]) << "<:" << std::flush;
        std::vector<uint8_t> data(idAndSize[1]);
        async_read(incomingStream, buffer(data), yield);
        for (unsigned i = 0; i < idAndSize[1]; ++i)
        {
            if (i % 16 == 0)
            {
                std::cout << "\n  ";
            }
            std::cout << std::hex << std::setw(2) << std::setfill('0') << unsigned(data[i]) << ' ';
        }
        std::cout << "\n" << std::flush;
    }
}

int main()
{
    asio::io_service io_service;
    asio::signal_set signal_set(io_service, SIGINT);
    signal_set.async_wait([&](const boost::system::error_code & error, int /*signal_number*/)
    {
        if (error != asio::error::operation_aborted)
        {
            io_service.stop();
        }
    });
    asio::spawn(io_service, [&](asio::yield_context yield)
    {
        std::clog << "Demultiplekser uruchomiony\n" << std::flush;
        auto baseStationStream = getBaseStationStream(io_service, yield);
        for (;;)
        {
            try
            {
                handleNextData(baseStationStream, yield);
            }
            catch (const boost::system::system_error &e)
            {
                if (e.code() == asio::error::eof)
                {
                    std::clog << "Połączenie zostało zakończone przez zdalnego hosta: " << e.what() << "\n";
                    signal_set.cancel();
                    break;
                }
                else
                {
                    throw;
                }
            }
            catch (const std::exception &e)
            {
                std::clog << "Nie udało się udczytać pakietu:\n" << e.what() << "\n" << std::flush;
            }
        }
    });

    io_service.run();
    std::clog << "Demultiplexer zakończył prace.\n";

    return 0;

}
