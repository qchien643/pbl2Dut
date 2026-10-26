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
    src/entities/EntityValidation.cpp
    src/repositories/TicketRepository.cpp
    src/persistence/TxtPersistence.cpp
    src/services/CatalogService.cpp
    src/services/ScreeningService.cpp
    src/services/TicketService.cpp
    src/services/ServiceClock.cpp
    src/services/SeatMap.cpp
    src/services/ScreeningTime.cpp
    src/services/ScreeningValidation.cpp
    src/ApplicationData.cpp
)
target_include_directories(cinema_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(cinema_services PUBLIC cxx_std_17)

foreach(structure IN ITEMS LinkedList HashTable Matrix2D Entity Repository Service ApplicationData)
    add_executable(${structure}Test tests/${structure}Test.cpp)
    target_include_directories(${structure}Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(${structure}Test PRIVATE cxx_std_17)
    add_test(NAME ${structure} COMMAND ${structure}Test)
endforeach()
target_link_libraries(EntityTest PRIVATE cinema_services)
target_link_libraries(RepositoryTest PRIVATE cinema_services)
target_link_libraries(ServiceTest PRIVATE cinema_services)
target_link_libraries(ApplicationDataTest PRIVATE cinema_services)

option(BUILD_GUI "Build the Qt Widgets application and UI checks" ON)
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    add_library(cinema_ui src/ui/MainWindow.cpp)
    target_link_libraries(cinema_ui PUBLIC cinema_services Qt6::Widgets)
    add_executable(cinema src/main.cpp)
    target_link_libraries(cinema PRIVATE cinema_ui)
    target_compile_definitions(cinema PRIVATE PBL2_DEMO_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/demo")
    add_executable(MainWindowTest tests/MainWindowTest.cpp)
    target_link_libraries(MainWindowTest PRIVATE cinema_ui)
    add_test(NAME MainWindow COMMAND MainWindowTest)
    set_tests_properties(MainWindow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
endif()
