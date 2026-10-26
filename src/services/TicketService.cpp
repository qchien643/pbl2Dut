#include "services/TicketService.h"

#include "entities/EntityValidation.h"
#include "services/ScreeningTime.h"

#include <limits>
#include <stdexcept>

TicketService::TicketService(const FilmRepository& films, const AuditoriumRepository& rooms,
                             const SeatRepository& seats,
                             const ScreeningRepository& screenings,
                             TicketRepository& tickets, ServiceClock clock)
    : films_(films), rooms_(rooms), seats_(seats), screenings_(screenings),
      tickets_(tickets), clock_(clock)
{
    if (clock_ == nullptr)
    {
        throw std::invalid_argument("Clock is required");
    }
}

SaleDetails TicketService::details(EntityId screeningId, EntityId seatId) const
{
    if (screeningId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Screening* screening = screenings_.findById(screeningId);
    if (screening == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    if (seatId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Seat* seat = seats_.findById(seatId);
    if (seat == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const Auditorium* room = rooms_.findById(screening->auditoriumId());
    if (room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    if (seat->auditoriumId() != room->id()
        || seat->rowNumber() > room->rowCount() || seat->seatNumber() > room->seatsPerRow())
    {
        throw std::invalid_argument("Seat does not belong to screening room");
    }
    const Film* film = films_.findById(screening->filmId());
    if (film == nullptr)
    {
        throw std::out_of_range("Record not found");
    }

    const std::int64_t endMinute = screening_time::endMinute(*screening, films_);
    const std::string endTime = screening_time::formatMinute(endMinute);
    SaleDetails result{*film, *room, *screening, *seat, endTime};
    return result;
}

void TicketService::requireAvailable(const SaleDetails& sale, const std::string& now) const
{
    entity_detail::dateTime(now);
    if (now >= sale.screening.startTime())
    {
        throw std::invalid_argument("Screening has already started");
    }
    for (const Ticket& ticket : tickets_)
    {
        if (ticket.screeningId() == sale.screening.id() && ticket.seatId() == sale.seat.id())
        {
            throw std::invalid_argument("Seat is already sold");
        }
    }
}

SaleDetails TicketService::preview(EntityId screeningId, EntityId seatId) const
{
    SaleDetails sale = details(screeningId, seatId);
    const std::string now = clock_();
    requireAvailable(sale, now);
    return sale;
}

EntityId TicketService::sell(EntityId screeningId, EntityId seatId, bool cashConfirmed)
{
    if (!cashConfirmed)
    {
        throw std::invalid_argument("Cash confirmation is required");
    }
    const SaleDetails sale = details(screeningId, seatId);
    const std::string now = clock_();
    requireAvailable(sale, now);
    const EntityId id = tickets_.nextId();
    const std::int64_t price = sale.screening.ticketPrice();
    const Ticket ticket(id, screeningId, seatId, now, price);
    tickets_.insert(ticket);
    return id;
}

TicketDetails TicketService::find(EntityId ticketId) const
{
    if (ticketId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Ticket* ticket = tickets_.findById(ticketId);
    if (ticket == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    SaleDetails sale = details(ticket->screeningId(), ticket->seatId());
    TicketDetails result{*ticket, sale};
    return result;
}

std::unique_ptr<SeatMap> TicketService::seatMap(EntityId screeningId) const
{
    if (screeningId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Screening* screening = screenings_.findById(screeningId);
    if (screening == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const Auditorium* room = rooms_.findById(screening->auditoriumId());
    if (room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    std::unique_ptr<SeatMap> result = makeSeatMap(*room, seats_);
    for (const Ticket& ticket : tickets_)
    {
        if (ticket.screeningId() != screeningId)
        {
            continue;
        }
        const Seat* seat = seats_.findById(ticket.seatId());
        if (seat == nullptr)
        {
            throw std::out_of_range("Record not found");
        }
        if (seat->auditoriumId() != screening->auditoriumId())
        {
            throw std::invalid_argument("Ticket seat belongs to another room");
        }
        const std::size_t row = static_cast<std::size_t>(seat->rowNumber() - 1);
        const std::size_t column = static_cast<std::size_t>(seat->seatNumber() - 1);
        SeatStatus& cell = result->at(row, column);
        if (cell.sold)
        {
            throw std::invalid_argument("Duplicate screening-seat ticket");
        }
        cell.sold = true;
    }
    return result;
}

void TicketService::validateSaleDate(const std::string& saleDate)
{
    screening_time::validateDate(saleDate);
}

DailyRevenue TicketService::revenue(const std::string& saleDate) const
{
    validateSaleDate(saleDate);
    DailyRevenue result;
    for (const Ticket& ticket : tickets_)
    {
        if (ticket.soldAt().compare(0, 10, saleDate) != 0)
        {
            continue;
        }
        if (ticket.unitPrice() > std::numeric_limits<std::int64_t>::max() - result.amount
            || result.ticketCount == std::numeric_limits<std::size_t>::max())
        {
            throw std::overflow_error("Revenue total overflow");
        }
        ++result.ticketCount;
        result.amount += ticket.unitPrice();
    }
    return result;
}

void TicketService::listBySaleDate(const std::string& saleDate,
                                  LinkedList<TicketDetails>& result) const
{
    validateSaleDate(saleDate);
    LinkedList<TicketDetails> matches;
    for (const Ticket& ticket : tickets_)
    {
        if (ticket.soldAt().compare(0, 10, saleDate) == 0)
        {
            const TicketDetails ticketDetails = find(ticket.id());
            matches.pushBack(ticketDetails);
        }
    }
    result.swap(matches);
}
