#pragma once

#include "entities/EntityId.h"

#include <QMainWindow>

class ApplicationData;
class CatalogService;
class QCloseEvent;
class QComboBox;
class QCheckBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;
class ScreeningService;
class TicketService;

bool loadApplicationData(ApplicationData& data, QWidget* parent = nullptr);

class MainWindow : public QMainWindow {
public:
    MainWindow(ApplicationData& data, CatalogService& catalog,
               ScreeningService& screenings, TicketService& tickets);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupSaleTab();
    void setupFilmTab();
    void setupRoomTab();
    void setupScreeningTab();
    void setupReportTab();

    void refreshFilms(EntityId selectId = 0);
    void editFilm(bool creating);
    void deleteFilm();
    void refreshRooms(EntityId selectId = 0);
    void refreshRoomSeats();
    void editRoom(bool creating);
    void deleteRoom();
    void refreshReferenceCombos();
    void refreshScreenings(EntityId selectId = 0);
    void editScreening(bool creating);
    void deleteScreening();
    void refreshSaleScreenings(EntityId selectId = 0);
    void refreshSaleSeats();
    void selectSaleSeat(int row, int column);
    void sellTicket();
    void lookupTicket();
    void showTicketDetails(EntityId ticketId);
    void refreshRevenue(EntityId selectTicketId = 0);
    void updateStatus();
    EntityId selectedFilm() const;
    EntityId selectedRoom() const;
    EntityId selectedScreening() const;
    EntityId selectedSaleScreening() const;

    ApplicationData& data_;
    CatalogService& catalog_;
    ScreeningService& screenings_;
    TicketService& tickets_;
    QTabWidget* tabs_;

    QComboBox* filmSearchMode_;
    QLineEdit* filmSearch_;
    QTableWidget* filmTable_;
    QLabel* filmResults_;
    QPushButton* filmEdit_;
    QPushButton* filmRemove_;

    QTableWidget* roomTable_;
    QTableWidget* roomSeatTable_;
    QLabel* roomResults_;
    QPushButton* roomEdit_;
    QPushButton* roomRemove_;

    QCheckBox* screeningUseDate_;
    QDateEdit* screeningDate_;
    QLineEdit* screeningIdFilter_;
    QComboBox* screeningFilmFilter_;
    QComboBox* screeningRoomFilter_;
    QTableWidget* screeningTable_;
    QLabel* screeningResults_;
    QPushButton* screeningEdit_;
    QPushButton* screeningRemove_;

    QCheckBox* saleUseDate_;
    QDateEdit* saleDate_;
    QComboBox* saleFilmFilter_;
    QComboBox* saleRoomFilter_;
    QTableWidget* saleScreeningTable_;
    QTableWidget* saleSeatTable_;
    QLabel* saleDetails_;
    QLabel* saleResults_;
    QPushButton* sellButton_;
    EntityId selectedSaleSeat_ = 0;

    QLineEdit* ticketId_;
    QLabel* ticketResult_;
    QDateEdit* reportDate_;
    QLabel* reportCount_;
    QLabel* reportRevenue_;
    QTableWidget* dailyTicketTable_;
    QLabel* dailyTicketStatus_;

    QLabel* saveState_;
};
