#pragma once

#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"
#include "services/SeatMap.h"
#include "services/ServiceClock.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

struct SaleDetails
{
    Film film;
    Auditorium room;
    Screening screening;
    Seat seat;
    std::string endTime;
};

struct TicketDetails
{
    Ticket ticket;
    SaleDetails sale;
};

struct DailyRevenue
{
    std::size_t ticketCount = 0;
    std::int64_t amount = 0;
};

class TicketService
{
public:
    TicketService(const SeatRepository& seats, const ScreeningRepository& screenings,
                  TicketRepository& tickets, ServiceClock clock = localMinuteNow);

    SaleDetails preview(EntityId screeningId, EntityId seatId) const;
    EntityId sell(EntityId screeningId, EntityId seatId, bool cashConfirmed);
    TicketDetails find(EntityId ticketId) const;
    std::unique_ptr<SeatMap> seatMap(EntityId screeningId) const;
    DailyRevenue revenue(const std::string& saleDate) const;

    void listBySaleDate(const std::string& saleDate, LinkedList<TicketDetails>& result) const;

private:
    SaleDetails details(EntityId screeningId, EntityId seatId) const;
    void requireAvailable(const SaleDetails& sale, const std::string& now) const;
    static void validateSaleDate(const std::string& saleDate);

    const SeatRepository& seats_;
    const ScreeningRepository& screenings_;
    TicketRepository& tickets_;
    ServiceClock clock_;
};
