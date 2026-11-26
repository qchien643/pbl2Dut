#pragma once

#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "entities/Screening.h"
#include "entities/Seat.h"
#include "entities/Ticket.h"
#include "fileio/TextFile.h"
#include "repositories/IRepository.h"

#include <ostream>

namespace fileio {

extern const char* const filmHeader;
extern const char* const auditoriumHeader;
extern const char* const seatHeader;
extern const char* const screeningHeader;
extern const char* const ticketHeader;

Film readFilm(const Row& row);
Auditorium readAuditorium(const Row& row);
Seat readSeat(const Row& row, const IRepository<Auditorium>& rooms);
Screening readScreening(const Row& row, const IRepository<Film>& films,
                        const IRepository<Auditorium>& rooms);
Ticket readTicket(const Row& row, const IRepository<Screening>& screenings,
                  const IRepository<Seat>& seats);

void writeFilm(std::ostream& output, const Film& film);
void writeAuditorium(std::ostream& output, const Auditorium& room);
void writeSeat(std::ostream& output, const Seat& seat);
void writeScreening(std::ostream& output, const Screening& screening);
void writeTicket(std::ostream& output, const Ticket& ticket);

}
