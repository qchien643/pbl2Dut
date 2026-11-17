#include "repositories/RoomStorage.h"

#include <stdexcept>

RoomStorage::RoomStorage(Repository<Auditorium>& rooms, Repository<Seat>& seats,
                         TicketRepository& tickets)
    : rooms_(rooms), seats_(seats), tickets_(tickets)
{
}

void RoomStorage::add(const Auditorium& room, const LinkedList<Seat>& seats)
{
    save(room, seats, true);
}

void RoomStorage::update(const Auditorium& room, const LinkedList<Seat>& seats)
{
    save(room, seats, false);
}

void RoomStorage::save(const Auditorium& room, const LinkedList<Seat>& newSeats, bool creating)
{
    // ponytail: stage the whole seat store for atomic writes; use per-room batches if data outgrows RAM.
    Repository<Seat> staged;
    for (const Seat& seat : seats_)
    {
        if (seat.auditoriumId() != room.id())
        {
            staged.insert(seat);
        }
    }
    for (const Seat& seat : newSeats)
    {
        if (!staged.insert(seat))
        {
            throw std::invalid_argument("Duplicate seat ID");
        }
    }
    // All allocating work finishes before changing the room or existing pointers.
    const bool saved = creating ? rooms_.insert(room) : rooms_.update(room.id(), room);
    if (!saved)
    {
        throw std::invalid_argument("Room could not be saved");
    }
    const Auditorium& storedRoom = *rooms_.findById(room.id());
    for (const Seat& seat : staged)
    {
        if (seat.auditoriumId() == room.id())
        {
            const Seat replacement(seat.id(), storedRoom, seat.rowNumber(), seat.seatNumber());
            staged.update(seat.id(), replacement);
        }
    }
    tickets_.relinkSeats(staged);
    seats_.replaceWith(staged);
}
