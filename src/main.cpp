#include "ApplicationData.h"
#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QStyle>

#include <exception>
#include <filesystem>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Cinema");
    QApplication::setStyle("Fusion");
    app.setPalette(app.style()->standardPalette());
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Quản lý rạp chiếu phim — dữ liệu TXT offline"));
    parser.addHelpOption();
    parser.addPositionalArgument("data-directory", QStringLiteral("Thư mục dữ liệu; mặc định là data/demo."), "[data-directory]");
    parser.process(app);
    const QStringList arguments = parser.positionalArguments();
    if (arguments.size() > 1) {
        parser.showHelp(1);
    }
    const QString directory = arguments.isEmpty()
        ? QString::fromUtf8(PBL2_DEMO_DATA_DIR)
        : arguments.first();
    try {
        ApplicationData data(std::filesystem::u8path(directory.toUtf8().toStdString()));
        if (!loadApplicationData(data)) {
            return 1;
        }
        CatalogService catalog(data.films, data.rooms, data.seats, data.screenings);
        ScreeningService screenings(data.films, data.rooms, data.screenings, data.tickets);
        TicketService tickets(data.films, data.rooms, data.seats, data.screenings, data.tickets);
        MainWindow window(data, catalog, screenings, tickets);
        window.show();
        return app.exec();
    } catch (const std::exception& error) {
        QMessageBox box(QMessageBox::Critical, QStringLiteral("Không thể khởi động"),
                        QString::fromUtf8(error.what()), QMessageBox::Ok);
        box.setTextFormat(Qt::PlainText);
        box.exec();
        return 1;
    }
}
