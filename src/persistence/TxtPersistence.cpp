#include "TxtPersistence.h"

#include <charconv>

namespace txt_persistence {

void validateUtf8(const std::string& text) {
    for (std::size_t i = 0; i < text.size();) {
        const unsigned char first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80) {
            continue;
        }
        int remaining = -1;
        if (first >= 0xc2 && first <= 0xdf) {
            remaining = 1;
        } else if (first >= 0xe0 && first <= 0xef) {
            remaining = 2;
        } else if (first >= 0xf0 && first <= 0xf4) {
            remaining = 3;
        }
        if (remaining < 0 || text.size() - i < static_cast<std::size_t>(remaining)) {
            throw std::invalid_argument("Invalid UTF-8");
        }
        std::uint32_t code = first & ((1u << (6 - remaining)) - 1);
        for (int j = 0; j < remaining; ++j) {
            const unsigned char byte = static_cast<unsigned char>(text[i++]);
            if ((byte & 0xc0) != 0x80) {
                throw std::invalid_argument("Invalid UTF-8 continuation");
            }
            code = (code << 6) | (byte & 0x3f);
        }
        if ((remaining == 1 && code < 0x80) || (remaining == 2 && code < 0x800)
            || (remaining == 3 && code < 0x10000) || code > 0x10ffff
            || (code >= 0xd800 && code <= 0xdfff)) {
            throw std::invalid_argument("Invalid UTF-8 code point");
        }
    }
}

std::int64_t number(const std::string& field) {
    std::int64_t result = 0;
    const std::from_chars_result parsed = std::from_chars(field.data(), field.data() + field.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != field.data() + field.size()) {
        throw std::invalid_argument("Expected a complete int64 field");
    }
    return result;
}

const char* header(const Film*) { return "film_id|title|duration_minutes"; }
const char* header(const Auditorium*) { return "auditorium_id|name|row_count|seats_per_row"; }
const char* header(const Seat*) { return "seat_id|auditorium_id|row_number|seat_number"; }
const char* header(const Screening*) { return "screening_id|film_id|auditorium_id|start_time|ticket_price"; }
const char* header(const Ticket*) { return "ticket_id|screening_id|seat_id|sold_at|unit_price"; }

Film parse(const std::string* f, const Film*) {
    return Film(number(f[0]), f[1], number(f[2]));
}
Auditorium parse(const std::string* f, const Auditorium*) {
    return Auditorium(number(f[0]), f[1], number(f[2]), number(f[3]));
}
void writeRecord(std::ostream& out, const Film& value) {
    out << value.id() << '|' << value.title() << '|' << value.durationMinutes();
}
void writeRecord(std::ostream& out, const Auditorium& value) {
    out << value.id() << '|' << value.name() << '|' << value.rowCount() << '|' << value.seatsPerRow();
}
void writeRecord(std::ostream& out, const Seat& value) {
    out << value.id() << '|' << value.auditoriumId() << '|' << value.rowNumber() << '|' << value.seatNumber();
}
void writeRecord(std::ostream& out, const Screening& value) {
    out << value.id() << '|' << value.filmId() << '|' << value.auditoriumId()
        << '|' << value.startTime() << '|' << value.ticketPrice();
}
void writeRecord(std::ostream& out, const Ticket& value) {
    out << value.id() << '|' << value.screeningId() << '|' << value.seatId()
        << '|' << value.soldAt() << '|' << value.unitPrice();
}

}
