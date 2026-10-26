#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"

#include <filesystem>

class ApplicationData {
public:
    enum class LoadResult { Loaded, NeedsEmptyConfirmation };

    explicit ApplicationData(std::filesystem::path directory);
    ApplicationData(const ApplicationData&) = delete;
    ApplicationData& operator=(const ApplicationData&) = delete;
    ApplicationData(ApplicationData&&) = delete;
    ApplicationData& operator=(ApplicationData&&) = delete;

    FilmRepository films;
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;

    LoadResult load(bool confirmEmpty = false);
    void validate() const;
    void save();
    void retryRecovery();
    bool isReady() const;
    bool isDirty() const;
    bool needsManualRecovery() const;
    const std::filesystem::path& directory() const;

private:
    friend struct ApplicationDataTestAccess;
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
