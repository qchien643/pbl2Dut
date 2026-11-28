cmake_minimum_required(VERSION 3.16)
project(pbl2 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_library(cinema_services
    src/entities/Film.cpp
    src/entities/Auditorium.cpp
    src/entities/Seat.cpp
    src/entities/Screening.cpp
    src/entities/Ticket.cpp
    src/helper/FieldValidation.cpp
    src/helper/TextNormalization.cpp
    src/search/TextSearch.cpp
    src/repositories/FilmRepository.cpp
    src/repositories/TicketRepository.cpp
    src/repositories/RoomStorage.cpp
    src/fileio/TextFile.cpp
    src/fileio/EntityFile.cpp
    src/services/CatalogService.cpp
    src/services/ScreeningService.cpp
    src/services/TicketService.cpp
    src/helper/SeatMap.cpp
    src/helper/DateTimeHelper.cpp
    src/database/CinemaDatabase.cpp
)
target_include_directories(cinema_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(cinema_services PUBLIC cxx_std_17)

foreach(structure IN ITEMS LinkedList HashTable Matrix2D Entity Repository Service DependencyInjection CinemaDatabase FilmSearch Search)
    add_executable(${structure}Test tests/${structure}Test.cpp)
    target_include_directories(${structure}Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(${structure}Test PRIVATE cxx_std_17)
    add_test(NAME ${structure} COMMAND ${structure}Test)
endforeach()
target_link_libraries(EntityTest PRIVATE cinema_services)
target_link_libraries(RepositoryTest PRIVATE cinema_services)
target_link_libraries(ServiceTest PRIVATE cinema_services)
target_link_libraries(DependencyInjectionTest PRIVATE cinema_services)
target_link_libraries(CinemaDatabaseTest PRIVATE cinema_services)
target_link_libraries(FilmSearchTest PRIVATE cinema_services)
target_link_libraries(SearchTest PRIVATE cinema_services)
add_executable(FilmSearchBenchmark tools/FilmSearchBenchmark.cpp)
target_link_libraries(FilmSearchBenchmark PRIVATE cinema_services)
add_test(NAME SampleData COMMAND FilmSearchBenchmark ${CMAKE_CURRENT_SOURCE_DIR}/data/sample 1)
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME SampleGenerator COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_sample_data.py)
endif()

option(BUILD_GUI "Build the Qt Widgets application and UI checks" ON)
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    add_library(cinema_gui src/gui/CinemaWindow.cpp)
    target_link_libraries(cinema_gui PUBLIC cinema_services Qt6::Widgets)
    add_executable(cinema src/main.cpp)
    target_link_libraries(cinema PRIVATE cinema_gui)
    target_compile_definitions(cinema PRIVATE PBL2_DEMO_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/demo")
    add_executable(CinemaWindowTest tests/CinemaWindowTest.cpp)
    target_link_libraries(CinemaWindowTest PRIVATE cinema_gui)
    add_test(NAME CinemaWindow COMMAND CinemaWindowTest)
    set_tests_properties(CinemaWindow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
endif()
