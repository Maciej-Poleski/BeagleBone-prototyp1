#include <iostream>

#include <boost/asio/spawn.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "vuart.hxx"

constexpr std::uint16_t portNumber = 1234;

/**
* Mogą istnieć różne kanały. Każdy może transmitować done o określonym
* priorytecie. Każdy strumień danych ma określony priorytet.
*
* Ta funkcja docelowo będzie zastąpiona logiką przygotowującą kanały
* i wybierającą odpowiedni w zależności od typu danych który chcemy
* transmitować. Tutaj będzie również mechanizm przełączania kanałów.
*/
boost::asio::ip::tcp::socket getBaseStationStream(
        boost::asio::io_service &io_service,
        boost::asio::yield_context &yield)
{
    using namespace boost::asio::ip;
    tcp::endpoint e(tcp::v4(), portNumber);
    tcp::acceptor acc(io_service, e);

    tcp::socket result(io_service);
    acc.async_accept(result, yield);
    return std::move(result);
}

/**
* Szablon...
*
* Potrzeba lepszej nazwy dla translation
*/
void handleNextData(boost::asio::ip::tcp::socket &incomingStream, stream_descriptorArray &translation, boost::asio::yield_context &yield)
{
    using namespace boost::asio;
    std::uint8_t packSize;
    async_read(incomingStream, buffer(&packSize, 1), yield);
    for (unsigned i = 0; i < packSize; ++i)
    {
        std::uint8_t idAndSize[2];
        async_read(incomingStream, buffer(idAndSize, 2), yield);
        std::vector<uint8_t> data(idAndSize[1]);
        async_read(incomingStream, buffer(data), yield);
        if (idAndSize[0] >= translation.size())
        {
            std::cerr << "Wysłano dane do >" << unsigned(idAndSize[0]) << "<, ale ten adres jest nieznany\n";
        }
        else
        {
            async_write(translation[idAndSize[0]], buffer(data), yield); // Po co czekać?
        }
    }
}

int main()
{
    boost::asio::io_service io_service;
    boost::asio::signal_set signal_set(io_service, SIGINT);
    signal_set.async_wait([&](const boost::system::error_code &error, int /*signal_number*/) {
        if (error != boost::asio::error::operation_aborted)
        {
            io_service.stop();
        }
    });
    boost::asio::spawn(io_service, [&](boost::asio::yield_context yield) {
        auto vuartTranslation = getVuartTranslation(io_service);
        std::clog << "Demultiplekser uruchomiony\n" << std::flush;
        auto baseStationStream = getBaseStationStream(io_service, yield);
        for (; ;)
        {
            try
            {
                handleNextData(baseStationStream, vuartTranslation, yield);
            }
            catch (const boost::system::system_error &e)
            {
                if (e.code() == boost::asio::error::eof)
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
