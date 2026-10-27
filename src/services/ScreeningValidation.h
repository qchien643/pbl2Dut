#pragma once

#include "entities/Screening.h"
#include "repositories/ScreeningRepository.h"

namespace service_validation {

void screening(const Screening& candidate, const ScreeningRepository& screenings);

}
