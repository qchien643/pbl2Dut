#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"
#include "repositories/RoomStorage.h"

#include <filesystem>

class CinemaDatabase {
public:
    enum class LoadResult { Loaded, NeedsEmptyConfirmation };

    explicit CinemaDatabase(std::filesystem::path directory);
    CinemaDatabase(const CinemaDatabase&) = delete;
    CinemaDatabase& operator=(const CinemaDatabase&) = delete;
    CinemaDatabase(CinemaDatabase&&) = delete;
    CinemaDatabase& operator=(CinemaDatabase&&) = delete;

    FilmRepository films;
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;
    RoomStorage roomStorage;

    LoadResult load(bool confirmEmpty = false);
    void validate() const;
    void save();
    void retryRecovery();
    bool isReady() const;
    bool isDirty() const;
    bool needsManualRecovery() const;
    const std::filesystem::path& directory() const;

private:
    friend struct CinemaDatabaseTestAccess;
    // Fault injection for filesystem failure tests; unused by the application.
    void (*beforeFileStep_)(const char* phase, std::size_t index) = nullptr;
    void fileStep(const char* phase, std::size_t index) const;
    void markSaved();

    std::filesystem::path directory_;
    bool ready_ = false;
    bool hadFiles_ = false;
    bool ownsStage_ = false;
    bool recoveryRequired_ = false;
};
