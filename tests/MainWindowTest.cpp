#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ApplicationData.h"
#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>

#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
T* child(QObject& parent, const char* name) {
    auto* found = parent.findChild<T*>(name);
    assert(found);
    return found;
}

void answerConfirmation(bool accept) {
    QTimer::singleShot(0, [accept] {
        auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        assert(box);
        child<QPushButton>(*box, accept ? "confirmYes" : "confirmCancel")->click();
    });
}

void dismissError() {
    QTimer::singleShot(0, [] {
        auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        assert(box && box->icon() == QMessageBox::Critical);
        box->accept();
    });
}

void fillDialog(const QString& title, const QString& duration, bool accept = true) {
    QTimer::singleShot(0, [=] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog && dialog->objectName() == "filmDialog");
        child<QLineEdit>(*dialog, "filmTitle")->setText(title);
        child<QLineEdit>(*dialog, "filmDuration")->setText(duration);
        if (accept) {
            child<QPushButton>(*dialog, "filmSubmit")->click();
        } else {
            dialog->reject();
        }
    });
}

void fillRoomDialog(const QString& name, const QString& rows, const QString& columns) {
    QTimer::singleShot(0, [=] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog && dialog->objectName() == "roomDialog");
        child<QPushButton>(*dialog, "roomSubmit")->click();
        assert(dialog->isVisible() && !child<QLabel>(*dialog, "roomError")->isHidden());
        child<QLineEdit>(*dialog, "roomName")->setText(name);
        child<QLineEdit>(*dialog, "roomRows")->setText(rows);
        child<QLineEdit>(*dialog, "roomColumns")->setText(columns);
        child<QPushButton>(*dialog, "roomSubmit")->click();
    });
}

void fillScreeningDialog(const QDateTime& start, const QString& price) {
    QTimer::singleShot(0, [=] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog && dialog->objectName() == "screeningDialog");
        child<QDateTimeEdit>(*dialog, "screeningStart")->setDateTime(start);
        child<QLineEdit>(*dialog, "screeningPrice")->setText(price);
        child<QPushButton>(*dialog, "screeningSubmit")->click();
    });
}

std::string uiClock() { return "2026-10-19 10:00"; }

std::filesystem::path path(const QTemporaryDir& directory) {
    return std::filesystem::u8path(directory.path().toUtf8().toStdString());
}

int rowForId(QTableWidget* table, EntityId id) {
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->data(Qt::DisplayRole).toLongLong() == id) return row;
    }
    return -1;
}

void testStartup() {
    QTemporaryDir directory;
    assert(directory.isValid());
    ApplicationData data(path(directory));
    answerConfirmation(false);
    assert(!loadApplicationData(data));
    assert(!data.isReady() && std::filesystem::is_empty(path(directory)));
    answerConfirmation(true);
    assert(loadApplicationData(data));
    assert(data.isReady() && std::filesystem::is_empty(path(directory)));
    data.save();

    std::ofstream(path(directory) / "films.txt") << "broken\n";
    ApplicationData broken(path(directory));
    dismissError();
    assert(!loadApplicationData(broken));
    assert(!broken.isReady());
    QFile file(directory.filePath("films.txt"));
    assert(file.open(QIODevice::ReadOnly) && file.readAll() == "broken\n");
}

void testFilmsAndClose() {
    QTemporaryDir directory;
    assert(directory.isValid());
    ApplicationData data(path(directory));
    data.load(true);
    CatalogService catalog(data.films, data.rooms, data.seats, data.screenings, data.tickets);
    ScreeningService screenings(data.films, data.rooms, data.screenings, data.tickets);
    TicketService tickets(data.seats, data.screenings, data.tickets);
    MainWindow window(data, catalog, screenings, tickets);
    window.show();
    QApplication::processEvents();
    auto* table = child<QTableWidget>(window, "filmTable");
    auto* add = child<QPushButton>(window, "addFilm");
    auto* edit = child<QPushButton>(window, "editFilm");
    auto* remove = child<QPushButton>(window, "deleteFilm");
    auto* search = child<QLineEdit>(window, "filmSearch");
    auto* mode = child<QComboBox>(window, "searchMode");
    child<QTabWidget>(window, "mainTabs")->setCurrentWidget(child<QWidget>(window, "filmTab"));
    assert(table->rowCount() == 0 && !edit->isEnabled() && !remove->isEnabled());
    search->setFocus();
    QApplication::processEvents();
    QKeyEvent tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(search, &tab);
    assert(QApplication::focusWidget() == add);
    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog);
        QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(dialog, &escape);
    });
    add->click();
    assert(data.films.empty());

    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog);
        auto* title = child<QLineEdit>(*dialog, "filmTitle");
        auto* duration = child<QLineEdit>(*dialog, "filmDuration");
        auto* submit = child<QPushButton>(*dialog, "filmSubmit");
        submit->click();
        assert(dialog->isVisible() && !child<QLabel>(*dialog, "filmError")->isHidden());
        title->setText(QString::fromUtf8(u8"Phim Việt"));
        duration->setText("0");
        submit->click();
        assert(dialog->isVisible() && title->text() == QString::fromUtf8(u8"Phim Việt"));
        duration->setText("9223372036854775808");
        submit->click();
        assert(dialog->isVisible());
        duration->setText("120");
        submit->click();
    });
    add->click();
    assert(table->rowCount() == 1 && data.films.findById(1)->title() == u8"Phim Việt");
    assert(data.isDirty() && window.isWindowModified());
    assert(std::filesystem::is_empty(path(directory)));
    fillDialog("Cancelled", "100", false);
    add->click();
    assert(data.films.size() == 1);
    fillDialog("Other", "90");
    add->click();
    assert(data.films.size() == 2);
    search->setText(QString::fromUtf8(u8"Việt"));
    assert(table->rowCount() == 1 && table->item(0, 0)->text() == "1");
    mode->setCurrentIndex(1);
    search->setText("invalid");
    assert(table->rowCount() == 0);
    search->setText("999");
    assert(table->rowCount() == 0);
    search->setText("1");
    assert(table->rowCount() == 1);
    table->selectRow(0);
    fillDialog(QString::fromUtf8(u8"Tên mới"), "110");
    edit->click();
    assert(data.films.findById(1)->durationMinutes() == 110);
    assert(data.films.findById(1)->title() == u8"Tên mới");

    // Historical relationships loaded through the core still constrain UI edits.
    catalog.addRoom("Room", 1, 1);
    data.screenings.insert(Screening(1, *data.films.findById(1), *data.rooms.findById(1),
                                      "2020-01-01 18:00", 80000));
    mode->setCurrentIndex(1);
    search->setText("1");
    table->selectRow(0);
    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog);
        child<QLineEdit>(*dialog, "filmTitle")->setText("Scheduled rename");
        child<QLineEdit>(*dialog, "filmDuration")->setText("111");
        child<QPushButton>(*dialog, "filmSubmit")->click();
        assert(dialog->isVisible() && !child<QLabel>(*dialog, "filmError")->isHidden());
        assert(child<QLineEdit>(*dialog, "filmTitle")->text() == "Scheduled rename");
        child<QLineEdit>(*dialog, "filmDuration")->setText("110");
        child<QPushButton>(*dialog, "filmSubmit")->click();
    });
    edit->click();
    assert(data.films.findById(1)->title() == "Scheduled rename");
    search->setText("1");
    table->selectRow(0);
    answerConfirmation(true);
    remove->click();
    assert(data.films.findById(1));
    assert(child<QLabel>(window, "filmResults")->text().contains(QString::fromUtf8(u8"Không thể xóa")));
    search->setText("2");
    table->selectRow(0);
    answerConfirmation(false);
    remove->click();
    assert(data.films.findById(2));
    answerConfirmation(true);
    remove->click();
    assert(!data.films.findById(2));

    mode->setCurrentIndex(0);
    search->clear();
    if (const auto screenshot = qEnvironmentVariable("PBL2_UI_SCREENSHOT"); !screenshot.isEmpty()) {
        assert(window.grab().save(screenshot + ".desktop.png"));
    }
    window.resize(560, 400);
    QApplication::processEvents();
    if (const auto screenshot = qEnvironmentVariable("PBL2_UI_SCREENSHOT"); !screenshot.isEmpty()) {
        assert(window.grab().save(screenshot));
    }
    answerConfirmation(false);
    assert(!window.close() && window.isVisible());

    // A real save failure must keep the window/RAM open, including via the exit button.
    std::filesystem::create_directory(path(directory) / ".cms-save");
    QTimer::singleShot(0, [] {
        auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        assert(box);
        dismissError();
        child<QPushButton>(*box, "confirmYes")->click();
    });
    child<QPushButton>(window, "exitButton")->click();
    assert(window.isVisible() && data.isDirty() && data.films.size() == 1);
    assert(!std::filesystem::exists(path(directory) / "films.txt"));
    std::filesystem::remove(path(directory) / ".cms-save");
    answerConfirmation(true);
    assert(window.close());
    assert(!window.isVisible() && !data.isDirty());
    ApplicationData reopened(path(directory));
    reopened.load();
    assert(reopened.films.size() == 1 && reopened.films.findById(1)->title() == "Scheduled rename");
    CatalogService nextCatalog(reopened.films, reopened.rooms, reopened.seats,
                               reopened.screenings, reopened.tickets);
    ScreeningService nextScreenings(reopened.films, reopened.rooms, reopened.screenings, reopened.tickets);
    TicketService nextTickets(reopened.seats, reopened.screenings, reopened.tickets);
    MainWindow next(reopened, nextCatalog, nextScreenings, nextTickets);
    next.show();
    answerConfirmation(true);
    child<QPushButton>(next, "exitButton")->click();
    assert(!next.isVisible() && std::filesystem::exists(path(directory) / ".cms-backup-2"));
}

void testCompleteWorkflow() {
    QTemporaryDir directory;
    assert(directory.isValid());
    ApplicationData data(path(directory));
    data.load(true);
    CatalogService catalog(data.films, data.rooms, data.seats, data.screenings, data.tickets);
    ScreeningService screenings(data.films, data.rooms, data.screenings, data.tickets, uiClock);
    TicketService tickets(data.seats, data.screenings, data.tickets, uiClock);
    assert(catalog.addFilm("Film", 120) == 1);
    MainWindow window(data, catalog, screenings, tickets);
    window.show();
    QApplication::processEvents();
    auto* tabs = child<QTabWidget>(window, "mainTabs");

    tabs->setCurrentWidget(child<QWidget>(window, "roomTab"));
    fillRoomDialog("Room", "2", "3");
    child<QPushButton>(window, "addRoom")->click();
    auto* roomTable = child<QTableWidget>(window, "roomTable");
    assert(roomTable->rowCount() == 1 && data.rooms.size() == 1 && data.seats.size() == 6);
    roomTable->selectRow(rowForId(roomTable, 1));
    auto* roomSeats = child<QTableWidget>(window, "roomSeatTable");
    assert(roomSeats->rowCount() == 2 && roomSeats->columnCount() == 3);
    assert(roomSeats->item(1, 2)->text() == "H2-G3");
    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(dialog && dialog->objectName() == "roomDialog");
        child<QLineEdit>(*dialog, "roomName")->setText("Room A");
        child<QLineEdit>(*dialog, "roomRows")->setText("2");
        child<QLineEdit>(*dialog, "roomColumns")->setText("2");
        child<QPushButton>(*dialog, "roomSubmit")->click();
    });
    child<QPushButton>(window, "editRoom")->click();
    assert(data.rooms.findById(1)->name() == "Room A" && data.seats.size() == 4);
    fillRoomDialog("Spare", "1", "1");
    child<QPushButton>(window, "addRoom")->click();
    assert(data.rooms.size() == 2 && data.seats.size() == 5);
    roomTable->selectRow(rowForId(roomTable, 2));
    answerConfirmation(true);
    child<QPushButton>(window, "deleteRoom")->click();
    assert(data.rooms.size() == 1 && data.seats.size() == 4);

    tabs->setCurrentWidget(child<QWidget>(window, "screeningTab"));
    fillScreeningDialog(QDateTime(QDate(2099, 1, 1), QTime(18, 0)), "80000");
    child<QPushButton>(window, "addScreening")->click();
    auto* screeningTable = child<QTableWidget>(window, "screeningTable");
    assert(screeningTable->rowCount() == 1 && data.screenings.size() == 1);
    assert(screeningTable->item(0, 4)->text() == "2099-01-01 20:00");
    screeningTable->selectRow(rowForId(screeningTable, 1));
    fillScreeningDialog(QDateTime(QDate(2099, 1, 1), QTime(18, 0)), "85000");
    child<QPushButton>(window, "editScreening")->click();
    assert(data.screenings.findById(1)->ticketPrice() == 85000);
    fillScreeningDialog(QDateTime(QDate(2099, 1, 1), QTime(20, 15)), "90000");
    child<QPushButton>(window, "addScreening")->click();
    assert(data.screenings.size() == 2);
    screeningTable->selectRow(rowForId(screeningTable, 2));
    answerConfirmation(true);
    child<QPushButton>(window, "deleteScreening")->click();
    assert(data.screenings.size() == 1);

    tabs->setCurrentWidget(child<QWidget>(window, "saleTab"));
    child<QDateEdit>(window, "reportDate")->setDate(QDate(2026, 10, 19));
    child<QCheckBox>(window, "saleUseDate")->setChecked(false);
    auto* saleShows = child<QTableWidget>(window, "saleScreeningTable");
    assert(saleShows->rowCount() == 1);
    saleShows->selectRow(0);
    auto* saleSeats = child<QTableWidget>(window, "saleSeatTable");
    assert(saleSeats->rowCount() == 2 && saleSeats->columnCount() == 2);
    saleSeats->cellClicked(0, 0);
    assert(child<QPushButton>(window, "sellTicket")->isEnabled());
    assert(child<QLabel>(window, "saleDetails")->text().contains("H1-G1"));
    answerConfirmation(false);
    child<QPushButton>(window, "sellTicket")->click();
    assert(data.tickets.empty() && child<QPushButton>(window, "sellTicket")->isEnabled());
    answerConfirmation(true);
    child<QPushButton>(window, "sellTicket")->click();
    assert(data.tickets.size() == 1 && saleSeats->item(0, 0)->text().startsWith(QString::fromUtf8(u8"×")));
    assert(child<QLabel>(window, "saleResults")->text().contains("#1"));
    assert(child<QTableWidget>(window, "dailyTicketTable")->rowCount() == 1);

    tabs->setCurrentWidget(child<QWidget>(window, "queryTab"));
    assert(child<QLineEdit>(window, "ticketId")->text() == "1");
    assert(child<QLabel>(window, "ticketResult")->text().contains("Film"));
    child<QDateEdit>(window, "reportDate")->setDate(QDate(2026, 10, 19));
    assert(child<QLabel>(window, "reportCount")->text() == "1");
    assert(child<QLabel>(window, "reportRevenue")->text().contains("85"));
    auto* dailyTickets = child<QTableWidget>(window, "dailyTicketTable");
    assert(dailyTickets->item(0, 2)->text() == "Film");
    dailyTickets->clearSelection();
    child<QLineEdit>(window, "ticketId")->clear();
    child<QLabel>(window, "ticketResult")->clear();
    dailyTickets->selectRow(0);
    assert(child<QLineEdit>(window, "ticketId")->text() == "1");
    assert(child<QLabel>(window, "ticketResult")->text().contains("H1-G1"));
    child<QDateEdit>(window, "reportDate")->setDate(QDate(2026, 10, 20));
    assert(dailyTickets->rowCount() == 0);
    assert(child<QLabel>(window, "dailyTicketStatus")->text().contains(QString::fromUtf8(u8"Không có vé")));
    child<QLineEdit>(window, "ticketId")->setText("1");
    child<QPushButton>(window, "lookupTicket")->click();
    assert(child<QDateEdit>(window, "reportDate")->date() == QDate(2026, 10, 19));
    assert(dailyTickets->rowCount() == 1);
    assert(!dailyTickets->selectedItems().isEmpty());

    tabs->setCurrentWidget(child<QWidget>(window, "screeningTab"));
    screeningTable->selectRow(rowForId(screeningTable, 1));
    answerConfirmation(true);
    child<QPushButton>(window, "deleteScreening")->click();
    assert(data.screenings.size() == 1);
    assert(child<QLabel>(window, "screeningResults")->text().contains(QString::fromUtf8(u8"đã bán vé")));
    tabs->setCurrentWidget(child<QWidget>(window, "roomTab"));
    roomTable->selectRow(rowForId(roomTable, 1));
    answerConfirmation(true);
    child<QPushButton>(window, "deleteRoom")->click();
    assert(data.rooms.size() == 1);
    assert(child<QLabel>(window, "roomResults")->text().contains(QString::fromUtf8(u8"đã có suất")));

    if (const auto screenshot = qEnvironmentVariable("PBL2_COMPLETE_UI_SCREENSHOT"); !screenshot.isEmpty()) {
        const char* tabNames[] = {"saleTab", "filmTab", "roomTab", "screeningTab", "queryTab"};
        const char* suffixes[] = {"sale", "films", "rooms", "screenings", "query"};
        window.resize(1100, 720);
        for (int index = 0; index < 5; ++index) {
            tabs->setCurrentWidget(child<QWidget>(window, tabNames[index]));
            QApplication::processEvents();
            assert(window.grab().save(screenshot + "." + suffixes[index] + ".png"));
        }
        window.resize(700, 500);
        tabs->setCurrentWidget(child<QWidget>(window, "queryTab"));
        QApplication::processEvents();
        assert(window.grab().save(screenshot + ".query-small.png"));
        tabs->setCurrentWidget(child<QWidget>(window, "saleTab"));
        QApplication::processEvents();
        assert(window.grab().save(screenshot + ".small.png"));
    }

    answerConfirmation(true);
    assert(window.close());
    ApplicationData reopened(path(directory));
    reopened.load();
    assert(reopened.films.size() == 1 && reopened.rooms.size() == 1);
    assert(reopened.seats.size() == 4 && reopened.screenings.size() == 1 && reopened.tickets.size() == 1);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");
    app.setPalette(app.style()->standardPalette());
    app.setQuitOnLastWindowClosed(false);
    testStartup();
    testFilmsAndClose();
    testCompleteWorkflow();
    std::cout << "MainWindow checks passed\n";
}
