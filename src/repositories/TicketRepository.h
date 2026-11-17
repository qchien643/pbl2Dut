#pragma once

#include "entities/Seat.h"
#include "entities/Ticket.h"
#include "repositories/Repository.h"
#include "repositories/ITicketRepository.h"

class TicketRepository final : public ITicketRepository
{
public:
    using value_type = Ticket;

    bool empty() const;
    std::size_t size() const;
    bool isDirty() const;
    void markSaved();
    EntityId nextId() const override;
    const Ticket* findById(EntityId id) const override;
    bool insert(const Ticket& ticket) override;
    void list(LinkedList<const Ticket*>& result) const override;

    LinkedList<Ticket>::ConstIterator begin() const;
    LinkedList<Ticket>::ConstIterator end() const;

private:
    friend class ApplicationData;
    friend class RoomStorage;
    void relinkSeats(const Repository<Seat>& seats) noexcept;
    Repository<Ticket> records_;
};

// tickets.update(id, ticket);
// tickets.erase(id);

// Điều này không phù hợp với nghiệp vụ:

// - Vé đã bán không nên sửa giá, thời gian bán hoặc suất chiếu.
// - Vé đã bán không nên xóa trực tiếp.
// - Việc xử lý hủy vé, hoàn tiền nếu có phải nằm trong service chuyên biệt.
