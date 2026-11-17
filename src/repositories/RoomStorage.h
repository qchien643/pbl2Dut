#pragma once

#include "repositories/IRoomStorage.h"
#include "repositories/Repository.h"
#include "repositories/TicketRepository.h"

class RoomStorage final : public IRoomStorage
{
public:
    RoomStorage(Repository<Auditorium>& rooms, Repository<Seat>& seats, TicketRepository& tickets);
    void add(const Auditorium& room, const LinkedList<Seat>& seats) override;
    void update(const Auditorium& room, const LinkedList<Seat>& seats) override;

private:
    void save(const Auditorium& room, const LinkedList<Seat>& seats, bool creating);

    Repository<Auditorium>& rooms_;
    Repository<Seat>& seats_;
    TicketRepository& tickets_;
};
