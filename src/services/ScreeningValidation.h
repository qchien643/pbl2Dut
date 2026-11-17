#pragma once

#include "entities/Screening.h"
#include "repositories/IRepository.h"

namespace service_validation {

void screening(const Screening& candidate, const IRepository<Screening>& screenings);

}
