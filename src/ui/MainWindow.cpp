#include "ApplicationData.h"
#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"
#include "ui/MainWindow.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
QString fromUtf8(const std::string& value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}

std::string toUtf8(const QString& value) {
    return value.toUtf8().toStdString();
}

QString money(std::int64_t value) {
    return QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(static_cast<qlonglong>(value))
        + QStringLiteral(" đ");
}

QString seatName(std::size_t row, std::size_t column) {
    return QStringLiteral("H%1-G%2").arg(row + 1).arg(column + 1);
}

EntityId comboId(const QComboBox* combo) {
    return combo->currentData().toLongLong();
}

QTableWidgetItem* setNumber(QTableWidget* table, int row, int column, std::int64_t value) {
    QTableWidgetItem* item = new QTableWidgetItem;
    item->setData(Qt::DisplayRole, QVariant::fromValue<qlonglong>(value));
    table->setItem(row, column, item);
    return item;
}

class MoneyItem : public QTableWidgetItem {
public:
    explicit MoneyItem(std::int64_t value) : QTableWidgetItem(money(value)) {
        setData(Qt::UserRole, QVariant::fromValue<qlonglong>(value));
    }

    bool operator<(const QTableWidgetItem& other) const override {
        return data(Qt::UserRole).toLongLong() < other.data(Qt::UserRole).toLongLong();
    }
};

void configureTable(QTableWidget* table, const QStringList& headers) {
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setWordWrap(false);
    table->horizontalHeader()->setHighlightSections(false);
    table->verticalHeader()->setDefaultSectionSize(32);
    table->verticalHeader()->hide();
}

QLineEdit* positiveInput(QWidget* parent, const char* objectName) {
    QLineEdit* input = new QLineEdit(parent);
    input->setObjectName(objectName);
    input->setMaxLength(19);
    input->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,19}"), input));
    input->setInputMethodHints(Qt::ImhDigitsOnly);
    return input;
}

bool positiveValue(QLineEdit* input, std::int64_t& value) {
    bool valid = false;
    value = input->text().toLongLong(&valid);
    return valid && value > 0;
}

QLabel* dialogError(QVBoxLayout* layout, QWidget* parent, const char* objectName) {
    QLabel* error = new QLabel(parent);
    error->setObjectName(objectName);
    error->setTextFormat(Qt::PlainText);
    error->setWordWrap(true);
    error->setFocusPolicy(Qt::StrongFocus);
    error->hide();
    layout->addWidget(error);
    return error;
}

void reportError(QLabel* error, const QString& text, QWidget* focus) {
    error->setText(text);
    error->show();
    focus->setAccessibleDescription(text);
    focus->setFocus();
}

QString errorText(const std::exception& error) {
    const std::string message = error.what();
    if (message == "Cannot change duration of a scheduled film") {
        return QStringLiteral("Phim đã có suất chiếu: chỉ được sửa tên, không đổi thời lượng.");
    }
    if (message == "Film is referenced by screenings") {
        return QStringLiteral("Không thể xóa phim đã có suất chiếu.");
    }
    if (message == "Room is referenced by screenings") {
        return QStringLiteral("Không thể sửa hoặc xóa phòng đã có suất chiếu.");
    }
    if (message == "Screening overlaps another room booking") {
        return QStringLiteral("Khoảng chiếu và 15 phút nghỉ bị trùng với suất khác trong phòng.");
    }
    if (message == "Screening must start in the future") {
        return QStringLiteral("Giờ bắt đầu phải ở tương lai.");
    }
    if (message == "Screening already has tickets") {
        return QStringLiteral("Suất đã bán vé nên không thể sửa hoặc xóa.");
    }
    if (message == "Screening has already started") {
        return QStringLiteral("Suất chiếu đã bắt đầu, không thể bán vé.");
    }
    if (message == "Seat is already sold") {
        return QStringLiteral("Ghế này vừa được bán. Hãy chọn ghế khác.");
    }
    if (message == "Record not found") {
        return QStringLiteral("Không tìm thấy bản ghi có mã đã nhập.");
    }
    if (message == "Name must not be empty") {
        return QStringLiteral("Tên không được để trống.");
    }
    if (message == "Name contains a control character or delimiter") {
        return QStringLiteral("Tên không được chứa dấu | hoặc ký tự điều khiển.");
    }
    return QStringLiteral("Không thực hiện được thao tác. Chi tiết: ") + fromUtf8(message);
}

bool confirm(QWidget* parent, const QString& title, const QString& text, const QString& accept) {
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    box.setObjectName("confirmation");
    box.setTextFormat(Qt::PlainText);
    QPushButton* yes = box.addButton(accept, QMessageBox::AcceptRole);
    yes->setObjectName("confirmYes");
    QPushButton* cancel = box.addButton(QStringLiteral("Hủy"), QMessageBox::RejectRole);
    cancel->setObjectName("confirmCancel");
    box.setDefaultButton(cancel);
    box.setEscapeButton(cancel);
    box.exec();
    return box.clickedButton() == yes;
}

void populateFilms(QComboBox* combo, CatalogService& catalog, bool includeAll) {
    const QSignalBlocker blocker(combo);
    const EntityId selected = comboId(combo);
    combo->clear();
    if (includeAll) {
        combo->addItem(QStringLiteral("Tất cả phim"), QVariant::fromValue<qlonglong>(0));
    }
    LinkedList<Film> films;
    catalog.films("", films);
    for (const Film& film : films) {
        combo->addItem(QStringLiteral("#%1 — %2").arg(film.id()).arg(fromUtf8(film.title())),
                       QVariant::fromValue<qlonglong>(film.id()));
    }
    const int index = combo->findData(QVariant::fromValue<qlonglong>(selected));
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

void populateRooms(QComboBox* combo, CatalogService& catalog, bool includeAll) {
    const QSignalBlocker blocker(combo);
    const EntityId selected = comboId(combo);
    combo->clear();
    if (includeAll) {
        combo->addItem(QStringLiteral("Tất cả phòng"), QVariant::fromValue<qlonglong>(0));
    }
    LinkedList<Auditorium> rooms;
    catalog.rooms(rooms);
    for (const Auditorium& room : rooms) {
        combo->addItem(QStringLiteral("#%1 — %2").arg(room.id()).arg(fromUtf8(room.name())),
                       QVariant::fromValue<qlonglong>(room.id()));
    }
    const int index = combo->findData(QVariant::fromValue<qlonglong>(selected));
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

class FilmDialog : public QDialog {
public:
    FilmDialog(CatalogService& catalog, EntityId id, QWidget* parent) : QDialog(parent) {
        setObjectName("filmDialog");
        setWindowTitle(id == 0 ? QStringLiteral("Thêm phim") : QStringLiteral("Sửa phim"));
        setMinimumWidth(380);
        QVBoxLayout* layout = new QVBoxLayout(this);
        QFormLayout* form = new QFormLayout;
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        QLineEdit* title = new QLineEdit(this);
        title->setObjectName("filmTitle");
        QLineEdit* duration = positiveInput(this, "filmDuration");
        form->addRow(QStringLiteral("&Tên phim *"), title);
        form->addRow(QStringLiteral("Thời &lượng (phút) *"), duration);
        layout->addLayout(form);
        QLabel* note = new QLabel(QStringLiteral("* Bắt buộc. Dữ liệu được lưu vào TXT khi thoát."), this);
        note->setWordWrap(true);
        layout->addWidget(note);
        QLabel* error = dialogError(layout, this, "filmError");
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Xác nhận"));
        buttons->button(QDialogButtonBox::Ok)->setObjectName("filmSubmit");
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
        layout->addWidget(buttons);
        if (id != 0) {
            const Film& film = catalog.film(id);
            title->setText(fromUtf8(film.title()));
            duration->setText(QString::number(film.durationMinutes()));
        }
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, this, [&, id, title, duration, error] {
            if (title->text().trimmed().isEmpty()) {
                reportError(error, QStringLiteral("Tên phim không được để trống."), title);
                return;
            }
            std::int64_t minutes = 0;
            if (!positiveValue(duration, minutes)) {
                reportError(error, QStringLiteral("Thời lượng phải là số nguyên dương trong miền int64."), duration);
                return;
            }
            try {
                if (id == 0) {
                    filmId = catalog.addFilm(toUtf8(title->text()), minutes);
                }
                else {
                    catalog.updateFilm(id, toUtf8(title->text()), minutes);
                    filmId = id;
                }
                accept();
            } catch (const std::exception& exception) {
                reportError(error, errorText(exception), error);
            }
        });
        title->setFocus();
    }
    EntityId filmId = 0;
};

class RoomDialog : public QDialog {
public:
    RoomDialog(CatalogService& catalog, EntityId id, QWidget* parent) : QDialog(parent) {
        setObjectName("roomDialog");
        setWindowTitle(id == 0 ? QStringLiteral("Thêm phòng") : QStringLiteral("Sửa phòng"));
        setMinimumWidth(380);
        QVBoxLayout* layout = new QVBoxLayout(this);
        QFormLayout* form = new QFormLayout;
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        QLineEdit* name = new QLineEdit(this);
        name->setObjectName("roomName");
        QLineEdit* rows = positiveInput(this, "roomRows");
        QLineEdit* columns = positiveInput(this, "roomColumns");
        form->addRow(QStringLiteral("&Tên phòng *"), name);
        form->addRow(QStringLiteral("Số &hàng *"), rows);
        form->addRow(QStringLiteral("&Ghế mỗi hàng *"), columns);
        layout->addLayout(form);
        QLabel* note = new QLabel(QStringLiteral("Ghế được tự sinh theo hình chữ nhật; không chỉnh từng ghế."), this);
        note->setWordWrap(true);
        layout->addWidget(note);
        QLabel* error = dialogError(layout, this, "roomError");
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Xác nhận"));
        buttons->button(QDialogButtonBox::Ok)->setObjectName("roomSubmit");
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
        layout->addWidget(buttons);
        if (id != 0) {
            const Auditorium& room = catalog.room(id);
            name->setText(fromUtf8(room.name()));
            rows->setText(QString::number(room.rowCount()));
            columns->setText(QString::number(room.seatsPerRow()));
        }
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, this, [&, id, name, rows, columns, error] {
            if (name->text().trimmed().isEmpty()) {
                reportError(error, QStringLiteral("Tên phòng không được để trống."), name);
                return;
            }
            std::int64_t rowCount = 0;
            std::int64_t columnCount = 0;
            if (!positiveValue(rows, rowCount)) {
                reportError(error, QStringLiteral("Số hàng phải là số nguyên dương trong miền int64."), rows);
                return;
            }
            if (!positiveValue(columns, columnCount)) {
                reportError(error, QStringLiteral("Số ghế mỗi hàng phải là số nguyên dương trong miền int64."), columns);
                return;
            }
            try {
                if (id == 0) {
                    roomId = catalog.addRoom(toUtf8(name->text()), rowCount, columnCount);
                }
                else {
                    catalog.updateRoom(id, toUtf8(name->text()), rowCount, columnCount);
                    roomId = id;
                }
                accept();
            } catch (const std::exception& exception) {
                reportError(error, errorText(exception), error);
            }
        });
        name->setFocus();
    }
    EntityId roomId = 0;
};

class ScreeningDialog : public QDialog {
public:
    ScreeningDialog(CatalogService& catalog, ScreeningService& screenings,
                    EntityId id, QWidget* parent) : QDialog(parent) {
        setObjectName("screeningDialog");
        setWindowTitle(id == 0 ? QStringLiteral("Thêm suất chiếu") : QStringLiteral("Sửa suất chiếu"));
        setMinimumWidth(430);
        QVBoxLayout* layout = new QVBoxLayout(this);
        QFormLayout* form = new QFormLayout;
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        QComboBox* film = new QComboBox(this);
        film->setObjectName("screeningFilm");
        QComboBox* room = new QComboBox(this);
        room->setObjectName("screeningRoom");
        populateFilms(film, catalog, false);
        populateRooms(room, catalog, false);
        QDateTimeEdit* start = new QDateTimeEdit(this);
        start->setObjectName("screeningStart");
        start->setDisplayFormat("yyyy-MM-dd HH:mm");
        start->setCalendarPopup(true);
        QDateTime initial = QDateTime::currentDateTime().addSecs(3600);
        initial.setTime(QTime(initial.time().hour(), initial.time().minute()));
        start->setDateTime(initial);
        QLineEdit* price = positiveInput(this, "screeningPrice");
        form->addRow(QStringLiteral("&Phim *"), film);
        form->addRow(QStringLiteral("P&hòng *"), room);
        form->addRow(QStringLiteral("&Bắt đầu *"), start);
        form->addRow(QStringLiteral("&Giá vé (VND) *"), price);
        layout->addLayout(form);
        QLabel* note = new QLabel(QStringLiteral("Phòng được giữ đến hết phim và thêm 15 phút nghỉ."), this);
        note->setWordWrap(true);
        layout->addWidget(note);
        QLabel* error = dialogError(layout, this, "screeningError");
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Xác nhận"));
        buttons->button(QDialogButtonBox::Ok)->setObjectName("screeningSubmit");
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
        layout->addWidget(buttons);
        if (id != 0) {
            const Screening& screening = screenings.find(id);
            film->setCurrentIndex(film->findData(QVariant::fromValue<qlonglong>(screening.filmId())));
            room->setCurrentIndex(room->findData(QVariant::fromValue<qlonglong>(screening.auditoriumId())));
            start->setDateTime(QDateTime::fromString(fromUtf8(screening.startTime()), "yyyy-MM-dd HH:mm"));
            price->setText(QString::number(screening.ticketPrice()));
        }
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, this, [&, id, film, room, start, price, error] {
            if (comboId(film) == 0 || comboId(room) == 0) {
                reportError(error, QStringLiteral("Cần có ít nhất một phim và một phòng."), error);
                return;
            }
            std::int64_t ticketPrice = 0;
            if (!positiveValue(price, ticketPrice)) {
                reportError(error, QStringLiteral("Giá vé phải là số nguyên dương trong miền int64."), price);
                return;
            }
            try {
                const std::string startText = toUtf8(start->dateTime().toString("yyyy-MM-dd HH:mm"));
                if (id == 0) {
                    screeningId = screenings.add(comboId(film), comboId(room), startText, ticketPrice);
                }
                else {
                    screenings.update(id, comboId(film), comboId(room), startText, ticketPrice);
                    screeningId = id;
                }
                accept();
            } catch (const std::exception& exception) {
                reportError(error, errorText(exception), error);
            }
        });
        film->setFocus();
    }
    EntityId screeningId = 0;
};
}

bool loadApplicationData(ApplicationData& data, QWidget* parent) {
    try {
        if (data.load() == ApplicationData::LoadResult::NeedsEmptyConfirmation) {
            if (!confirm(parent, QStringLiteral("Khởi tạo dữ liệu"),
                         QStringLiteral("Thư mục chưa có bộ dữ liệu TXT:\n%1\n\n"
                                        "Bắt đầu phiên rỗng? File chỉ được tạo khi thoát và lưu thành công.")
                             .arg(fromUtf8(data.directory().u8string())),
                         QStringLiteral("Bắt đầu phiên rỗng"))) {
                return false;
            }
            data.load(true);
        }
        return true;
    } catch (const std::exception& error) {
        QMessageBox box(QMessageBox::Critical, QStringLiteral("Không thể mở dữ liệu"),
                        QStringLiteral("Chưa mở chế độ chỉnh sửa. Không có file nào bị ghi đè.\n\n")
                            + fromUtf8(error.what()), QMessageBox::Ok, parent);
        box.setTextFormat(Qt::PlainText);
        box.exec();
        return false;
    }
}

MainWindow::MainWindow(ApplicationData& data, CatalogService& catalog,
                       ScreeningService& screenings, TicketService& tickets)
    : data_(data), catalog_(catalog), screenings_(screenings), tickets_(tickets) {
    if (!data.isReady()) {
        throw std::logic_error("MainWindow requires loaded and validated data");
    }
    setObjectName("mainWindow");
    resize(1100, 720);
    setMinimumSize(700, 500);
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    QHBoxLayout* header = new QHBoxLayout;
    QLabel* heading = new QLabel(QStringLiteral("Quản lý rạp chiếu phim"), central);
    QFont headingFont = heading->font();
    headingFont.setBold(true);
    headingFont.setPointSize(headingFont.pointSize() + 3);
    heading->setFont(headingFont);
    header->addWidget(heading);
    header->addStretch();
    QPushButton* exit = new QPushButton(QStringLiteral("&Lưu và thoát"), central);
    exit->setObjectName("exitButton");
    header->addWidget(exit);
    layout->addLayout(header);
    QLabel* policy = new QLabel(QStringLiteral("Thay đổi được giữ trong phiên và chỉ ghi vào tệp khi chọn Lưu và thoát."), central);
    policy->setWordWrap(true);
    layout->addWidget(policy);

    tabs_ = new QTabWidget(central);
    tabs_->setObjectName("mainTabs");
    tabs_->setDocumentMode(true);
    layout->addWidget(tabs_, 1);

    setupSaleTab();
    setupFilmTab();
    setupRoomTab();
    setupScreeningTab();
    setupReportTab();

    setCentralWidget(central);
    saveState_ = new QLabel(this);
    saveState_->setObjectName("saveState");
    statusBar()->addWidget(saveState_, 1);
    statusBar()->setToolTip(fromUtf8(data.directory().u8string()));

    connect(exit, &QPushButton::clicked, this, &QWidget::close);
    screeningDate_->setEnabled(false);
    refreshReferenceCombos();
    refreshFilms();
    refreshRooms();
    refreshScreenings();
    refreshSaleScreenings();
    refreshRevenue();
    updateStatus();
}

void MainWindow::setupSaleTab() {
    QWidget* saleTab = new QWidget(tabs_);
    saleTab->setObjectName("saleTab");
    tabs_->addTab(saleTab, QStringLiteral("&Bán vé"));
    QVBoxLayout* saleLayout = new QVBoxLayout(saleTab);
    saleLayout->addWidget(new QLabel(QStringLiteral("1. Chọn suất chiếu"), saleTab));
    QHBoxLayout* saleFilters = new QHBoxLayout;
    saleUseDate_ = new QCheckBox(QStringLiteral("Ngày &chiếu"), saleTab);
    saleUseDate_->setObjectName("saleUseDate");
    saleUseDate_->setChecked(true);
    saleDate_ = new QDateEdit(QDate::currentDate(), saleTab);
    saleDate_->setObjectName("saleDate");
    saleDate_->setDisplayFormat("yyyy-MM-dd");
    saleDate_->setCalendarPopup(true);
    saleFilmFilter_ = new QComboBox(saleTab);
    saleFilmFilter_->setObjectName("saleFilmFilter");
    saleRoomFilter_ = new QComboBox(saleTab);
    saleRoomFilter_->setObjectName("saleRoomFilter");
    saleFilters->addWidget(saleUseDate_);
    saleFilters->addWidget(saleDate_);
    saleFilters->addWidget(saleFilmFilter_, 1);
    saleFilters->addWidget(saleRoomFilter_, 1);
    saleLayout->addLayout(saleFilters);
    QGridLayout* saleSplit = new QGridLayout;
    saleScreeningTable_ = new QTableWidget(0, 6, saleTab);
    saleScreeningTable_->setObjectName("saleScreeningTable");
    configureTable(saleScreeningTable_, {QStringLiteral("Mã"), QStringLiteral("Phim"),
        QStringLiteral("Phòng"), QStringLiteral("Bắt đầu"), QStringLiteral("Kết thúc"),
        QStringLiteral("Giá vé (VND)")});
    saleScreeningTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    saleScreeningTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    saleSplit->addWidget(saleScreeningTable_, 0, 0, 1, 2);
    QGroupBox* seatsBox = new QGroupBox(QStringLiteral("2. Chọn ghế"), saleTab);
    QVBoxLayout* seatsLayout = new QVBoxLayout(seatsBox);
    QLabel* seatLegend = new QLabel(QStringLiteral("○ Trống   × Đã bán   ✓ Đang chọn"), seatsBox);
    seatLegend->setWordWrap(true);
    seatsLayout->addWidget(seatLegend);
    saleSeatTable_ = new QTableWidget(seatsBox);
    saleSeatTable_->setObjectName("saleSeatTable");
    saleSeatTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    saleSeatTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    saleSeatTable_->setSelectionBehavior(QAbstractItemView::SelectItems);
    saleSeatTable_->setAccessibleName(QStringLiteral("Sơ đồ ghế của suất chiếu"));
    seatsLayout->addWidget(saleSeatTable_, 1);
    saleSplit->addWidget(seatsBox, 1, 0);
    QGroupBox* detailsBox = new QGroupBox(QStringLiteral("3. Xác nhận bán vé"), saleTab);
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsBox);
    saleDetails_ = new QLabel(QStringLiteral("Chọn một suất chiếu và một ghế trống."), detailsBox);
    saleDetails_->setObjectName("saleDetails");
    saleDetails_->setTextFormat(Qt::PlainText);
    saleDetails_->setWordWrap(true);
    saleDetails_->setMinimumWidth(280);
    detailsLayout->addWidget(saleDetails_);
    detailsLayout->addStretch();
    sellButton_ = new QPushButton(QStringLiteral("&Xác nhận đã nhận tiền mặt"), detailsBox);
    sellButton_->setObjectName("sellTicket");
    sellButton_->setEnabled(false);
    detailsLayout->addWidget(sellButton_);
    saleResults_ = new QLabel(detailsBox);
    saleResults_->setObjectName("saleResults");
    saleResults_->setTextFormat(Qt::PlainText);
    saleResults_->setWordWrap(true);
    detailsLayout->addWidget(saleResults_);
    saleSplit->addWidget(detailsBox, 1, 1);
    saleSplit->setColumnStretch(0, 3);
    saleSplit->setColumnStretch(1, 2);
    saleSplit->setRowStretch(1, 1);
    saleLayout->addLayout(saleSplit, 1);

    connect(saleUseDate_, &QCheckBox::toggled, this, [this](bool enabled) {
        saleDate_->setEnabled(enabled);
        refreshSaleScreenings();
    });
    connect(saleDate_, &QDateEdit::dateChanged, this, [this] {
        if (saleUseDate_->isChecked()) {
            refreshSaleScreenings();
        }
    });
    connect(saleFilmFilter_, &QComboBox::currentIndexChanged, this, [this] { refreshSaleScreenings(); });
    connect(saleRoomFilter_, &QComboBox::currentIndexChanged, this, [this] { refreshSaleScreenings(); });
    connect(saleScreeningTable_, &QTableWidget::itemSelectionChanged, this, &MainWindow::refreshSaleSeats);
    connect(saleSeatTable_, &QTableWidget::cellClicked, this, &MainWindow::selectSaleSeat);
    connect(sellButton_, &QPushButton::clicked, this, &MainWindow::sellTicket);
}

void MainWindow::setupFilmTab() {
    QWidget* filmTab = new QWidget(tabs_);
    filmTab->setObjectName("filmTab");
    tabs_->addTab(filmTab, QStringLiteral("&Phim"));
    QVBoxLayout* filmsLayout = new QVBoxLayout(filmTab);
    QHBoxLayout* filmSearchRow = new QHBoxLayout;
    filmSearchMode_ = new QComboBox(filmTab);
    filmSearchMode_->setObjectName("searchMode");
    filmSearchMode_->addItems({QStringLiteral("Tên phim"), QStringLiteral("Mã phim")});
    filmSearchMode_->setAccessibleName(QStringLiteral("Kiểu tìm kiếm phim"));
    filmSearch_ = new QLineEdit(filmTab);
    filmSearch_->setObjectName("filmSearch");
    filmSearch_->setClearButtonEnabled(true);
    QLabel* filmSearchLabel = new QLabel(QStringLiteral("&Tìm:"), filmTab);
    filmSearchLabel->setBuddy(filmSearch_);
    filmSearchRow->addWidget(filmSearchLabel);
    filmSearchRow->addWidget(filmSearchMode_);
    filmSearchRow->addWidget(filmSearch_, 1);
    filmsLayout->addLayout(filmSearchRow);
    QHBoxLayout* filmActions = new QHBoxLayout;
    QPushButton* filmAdd = new QPushButton(QStringLiteral("&Thêm phim"), filmTab);
    filmAdd->setObjectName("addFilm");
    filmEdit_ = new QPushButton(QStringLiteral("&Sửa phim"), filmTab);
    filmEdit_->setObjectName("editFilm");
    filmRemove_ = new QPushButton(QStringLiteral("&Xóa phim"), filmTab);
    filmRemove_->setObjectName("deleteFilm");
    filmActions->addWidget(filmAdd);
    filmActions->addWidget(filmEdit_);
    filmActions->addStretch();
    filmActions->addWidget(filmRemove_);
    filmsLayout->addLayout(filmActions);
    filmTable_ = new QTableWidget(0, 3, filmTab);
    filmTable_->setObjectName("filmTable");
    filmTable_->setAccessibleName(QStringLiteral("Danh sách phim"));
    configureTable(filmTable_, {QStringLiteral("Mã"), QStringLiteral("Tên phim"),
                                QStringLiteral("Thời lượng (phút)")});
    filmTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    filmTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    filmTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    filmsLayout->addWidget(filmTable_, 1);
    filmResults_ = new QLabel(filmTab);
    filmResults_->setObjectName("filmResults");
    filmResults_->setTextFormat(Qt::PlainText);
    filmResults_->setWordWrap(true);
    filmsLayout->addWidget(filmResults_);
    QLabel* filmRules = new QLabel(QStringLiteral("Phim đã có suất: chỉ sửa tên, không sửa thời lượng hoặc xóa."), filmTab);
    filmRules->setWordWrap(true);
    filmsLayout->addWidget(filmRules);

    connect(filmAdd, &QPushButton::clicked, this, [this] { editFilm(true); });
    connect(filmEdit_, &QPushButton::clicked, this, [this] { editFilm(false); });
    connect(filmRemove_, &QPushButton::clicked, this, &MainWindow::deleteFilm);
    connect(filmSearch_, &QLineEdit::textChanged, this, [this] { refreshFilms(); });
    connect(filmSearchMode_, &QComboBox::currentIndexChanged, this, [this] { refreshFilms(); });
    connect(filmTable_, &QTableWidget::itemSelectionChanged, this, [this] {
        const bool selected = selectedFilm() != 0;
        filmEdit_->setEnabled(selected);
        filmRemove_->setEnabled(selected);
    });
    connect(filmTable_, &QTableWidget::cellDoubleClicked, this, [this] { editFilm(false); });
}

void MainWindow::setupRoomTab() {
    QWidget* roomTab = new QWidget(tabs_);
    roomTab->setObjectName("roomTab");
    tabs_->addTab(roomTab, QStringLiteral("Phòng && &ghế"));
    QVBoxLayout* roomLayout = new QVBoxLayout(roomTab);
    QHBoxLayout* roomActions = new QHBoxLayout;
    QPushButton* roomAdd = new QPushButton(QStringLiteral("&Thêm phòng"), roomTab);
    roomAdd->setObjectName("addRoom");
    roomEdit_ = new QPushButton(QStringLiteral("&Sửa phòng"), roomTab);
    roomEdit_->setObjectName("editRoom");
    roomRemove_ = new QPushButton(QStringLiteral("&Xóa phòng"), roomTab);
    roomRemove_->setObjectName("deleteRoom");
    roomActions->addWidget(roomAdd);
    roomActions->addWidget(roomEdit_);
    roomActions->addStretch();
    roomActions->addWidget(roomRemove_);
    roomLayout->addLayout(roomActions);
    roomTable_ = new QTableWidget(0, 5, roomTab);
    roomTable_->setObjectName("roomTable");
    roomTable_->setAccessibleName(QStringLiteral("Danh sách phòng"));
    configureTable(roomTable_, {QStringLiteral("Mã"), QStringLiteral("Tên phòng"),
        QStringLiteral("Số hàng"), QStringLiteral("Ghế / hàng"), QStringLiteral("Sức chứa")});
    roomTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    roomTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    roomLayout->addWidget(roomTable_, 1);
    roomResults_ = new QLabel(roomTab);
    roomResults_->setObjectName("roomResults");
    roomResults_->setWordWrap(true);
    roomLayout->addWidget(roomResults_);
    QGroupBox* roomSeatsBox = new QGroupBox(QStringLiteral("Sơ đồ ghế của phòng đang chọn"), roomTab);
    QVBoxLayout* roomSeatsLayout = new QVBoxLayout(roomSeatsBox);
    roomSeatTable_ = new QTableWidget(roomSeatsBox);
    roomSeatTable_->setObjectName("roomSeatTable");
    roomSeatTable_->setAccessibleName(QStringLiteral("Sơ đồ ghế phòng"));
    roomSeatTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roomSeatTable_->setSelectionMode(QAbstractItemView::NoSelection);
    roomSeatsLayout->addWidget(roomSeatTable_);
    roomLayout->addWidget(roomSeatsBox, 1);

    connect(roomAdd, &QPushButton::clicked, this, [this] { editRoom(true); });
    connect(roomEdit_, &QPushButton::clicked, this, [this] { editRoom(false); });
    connect(roomRemove_, &QPushButton::clicked, this, &MainWindow::deleteRoom);
    connect(roomTable_, &QTableWidget::itemSelectionChanged, this, [this] {
        const bool selected = selectedRoom() != 0;
        roomEdit_->setEnabled(selected);
        roomRemove_->setEnabled(selected);
        refreshRoomSeats();
    });
    connect(roomTable_, &QTableWidget::cellDoubleClicked, this, [this] { editRoom(false); });
}

void MainWindow::setupScreeningTab() {
    QWidget* screeningTab = new QWidget(tabs_);
    screeningTab->setObjectName("screeningTab");
    tabs_->addTab(screeningTab, QStringLiteral("&Suất chiếu"));
    QVBoxLayout* screeningLayout = new QVBoxLayout(screeningTab);
    QHBoxLayout* screeningFilters = new QHBoxLayout;
    screeningIdFilter_ = positiveInput(screeningTab, "screeningIdFilter");
    screeningIdFilter_->setPlaceholderText(QStringLiteral("Ví dụ: 12"));
    screeningIdFilter_->setClearButtonEnabled(true);
    screeningIdFilter_->setMaximumWidth(130);
    QLabel* screeningIdLabel = new QLabel(QStringLiteral("&Mã suất"), screeningTab);
    screeningIdLabel->setBuddy(screeningIdFilter_);
    screeningUseDate_ = new QCheckBox(QStringLiteral("Ngày &chiếu"), screeningTab);
    screeningUseDate_->setObjectName("screeningUseDate");
    screeningDate_ = new QDateEdit(QDate::currentDate(), screeningTab);
    screeningDate_->setObjectName("screeningDate");
    screeningDate_->setDisplayFormat("yyyy-MM-dd");
    screeningDate_->setCalendarPopup(true);
    screeningFilmFilter_ = new QComboBox(screeningTab);
    screeningFilmFilter_->setObjectName("screeningFilmFilter");
    screeningRoomFilter_ = new QComboBox(screeningTab);
    screeningRoomFilter_->setObjectName("screeningRoomFilter");
    screeningFilters->addWidget(screeningIdLabel);
    screeningFilters->addWidget(screeningIdFilter_);
    screeningFilters->addWidget(screeningUseDate_);
    screeningFilters->addWidget(screeningDate_);
    screeningFilters->addWidget(screeningFilmFilter_, 1);
    screeningFilters->addWidget(screeningRoomFilter_, 1);
    screeningLayout->addLayout(screeningFilters);
    QHBoxLayout* screeningActions = new QHBoxLayout;
    QPushButton* screeningAdd = new QPushButton(QStringLiteral("&Thêm suất"), screeningTab);
    screeningAdd->setObjectName("addScreening");
    screeningEdit_ = new QPushButton(QStringLiteral("&Sửa suất"), screeningTab);
    screeningEdit_->setObjectName("editScreening");
    screeningRemove_ = new QPushButton(QStringLiteral("&Xóa suất"), screeningTab);
    screeningRemove_->setObjectName("deleteScreening");
    screeningActions->addWidget(screeningAdd);
    screeningActions->addWidget(screeningEdit_);
    screeningActions->addStretch();
    screeningActions->addWidget(screeningRemove_);
    screeningLayout->addLayout(screeningActions);
    screeningTable_ = new QTableWidget(0, 6, screeningTab);
    screeningTable_->setObjectName("screeningTable");
    screeningTable_->setAccessibleName(QStringLiteral("Danh sách suất chiếu"));
    configureTable(screeningTable_, {QStringLiteral("Mã"), QStringLiteral("Phim"),
        QStringLiteral("Phòng"), QStringLiteral("Bắt đầu"), QStringLiteral("Kết thúc"),
        QStringLiteral("Giá vé (VND)")});
    screeningTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    screeningTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    screeningLayout->addWidget(screeningTable_, 1);
    screeningResults_ = new QLabel(screeningTab);
    screeningResults_->setObjectName("screeningResults");
    screeningResults_->setWordWrap(true);
    screeningLayout->addWidget(screeningResults_);
    QLabel* screeningRules = new QLabel(QStringLiteral("Chọn một dòng để sửa hoặc xóa. Lịch cùng phòng phải cách nhau ít nhất 15 phút."), screeningTab);
    screeningRules->setWordWrap(true);
    screeningLayout->addWidget(screeningRules);

    connect(screeningAdd, &QPushButton::clicked, this, [this] { editScreening(true); });
    connect(screeningEdit_, &QPushButton::clicked, this, [this] { editScreening(false); });
    connect(screeningRemove_, &QPushButton::clicked, this, &MainWindow::deleteScreening);
    connect(screeningIdFilter_, &QLineEdit::textChanged, this, [this] { refreshScreenings(); });
    connect(screeningUseDate_, &QCheckBox::toggled, this, [this](bool enabled) {
        screeningDate_->setEnabled(enabled);
        refreshScreenings();
    });
    connect(screeningDate_, &QDateEdit::dateChanged, this, [this] {
        if (screeningUseDate_->isChecked()) {
            refreshScreenings();
        }
    });
    connect(screeningFilmFilter_, &QComboBox::currentIndexChanged, this, [this] { refreshScreenings(); });
    connect(screeningRoomFilter_, &QComboBox::currentIndexChanged, this, [this] { refreshScreenings(); });
    connect(screeningTable_, &QTableWidget::itemSelectionChanged, this, [this] {
        const bool selected = selectedScreening() != 0;
        screeningEdit_->setEnabled(selected);
        screeningRemove_->setEnabled(selected);
    });
    connect(screeningTable_, &QTableWidget::cellDoubleClicked, this, [this] { editScreening(false); });
}

void MainWindow::setupReportTab() {
    QWidget* queryTab = new QWidget(tabs_);
    queryTab->setObjectName("queryTab");
    tabs_->addTab(queryTab, QStringLiteral("&Vé và doanh thu"));
    QGridLayout* queryLayout = new QGridLayout(queryTab);
    QGroupBox* reportBox = new QGroupBox(QStringLiteral("Tổng quan theo ngày bán"), queryTab);
    QHBoxLayout* reportLayout = new QHBoxLayout(reportBox);
    QFormLayout* reportForm = new QFormLayout;
    reportDate_ = new QDateEdit(QDate::currentDate(), reportBox);
    reportDate_->setObjectName("reportDate");
    reportDate_->setDisplayFormat("yyyy-MM-dd");
    reportDate_->setCalendarPopup(true);
    reportForm->addRow(QStringLiteral("&Ngày bán"), reportDate_);
    reportCount_ = new QLabel(reportBox);
    reportCount_->setObjectName("reportCount");
    reportRevenue_ = new QLabel(reportBox);
    reportRevenue_->setObjectName("reportRevenue");
    reportForm->addRow(QStringLiteral("Số vé"), reportCount_);
    reportForm->addRow(QStringLiteral("Doanh thu"), reportRevenue_);
    reportLayout->addLayout(reportForm);
    reportLayout->addSpacing(24);
    QLabel* reportNote = new QLabel(QStringLiteral("Tính theo thời điểm bán và giá đã ghi trên vé, không theo ngày chiếu."), reportBox);
    reportNote->setWordWrap(true);
    reportLayout->addWidget(reportNote, 1);
    queryLayout->addWidget(reportBox, 0, 0, 1, 2);
    QGroupBox* dailyTicketsBox = new QGroupBox(QStringLiteral("Danh sách vé"), queryTab);
    QVBoxLayout* dailyTicketsLayout = new QVBoxLayout(dailyTicketsBox);
    QHBoxLayout* ticketSearch = new QHBoxLayout;
    QLabel* ticketSearchLabel = new QLabel(QStringLiteral("&Đi đến mã vé"), dailyTicketsBox);
    ticketId_ = positiveInput(dailyTicketsBox, "ticketId");
    ticketId_->setClearButtonEnabled(true);
    ticketId_->setPlaceholderText(QStringLiteral("Ví dụ: 42"));
    ticketId_->setMaximumWidth(180);
    ticketSearchLabel->setBuddy(ticketId_);
    QPushButton* ticketLookup = new QPushButton(QStringLiteral("&Tìm vé"), dailyTicketsBox);
    ticketLookup->setObjectName("lookupTicket");
    ticketLookup->setToolTip(QStringLiteral("Mở ngày bán và chọn đúng vé trong danh sách"));
    ticketSearch->addWidget(ticketSearchLabel);
    ticketSearch->addWidget(ticketId_);
    ticketSearch->addWidget(ticketLookup);
    ticketSearch->addStretch();
    dailyTicketsLayout->addLayout(ticketSearch);
    dailyTicketTable_ = new QTableWidget(0, 6, dailyTicketsBox);
    dailyTicketTable_->setObjectName("dailyTicketTable");
    dailyTicketTable_->setAccessibleName(QStringLiteral("Danh sách vé đã bán theo ngày"));
    configureTable(dailyTicketTable_, {QStringLiteral("Mã vé"), QStringLiteral("Giờ bán"),
        QStringLiteral("Phim"), QStringLiteral("Phòng"), QStringLiteral("Ghế"),
        QStringLiteral("Giá (VND)")});
    dailyTicketTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dailyTicketTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    dailyTicketsLayout->addWidget(dailyTicketTable_, 1);
    dailyTicketStatus_ = new QLabel(dailyTicketsBox);
    dailyTicketStatus_->setObjectName("dailyTicketStatus");
    dailyTicketStatus_->setWordWrap(true);
    dailyTicketsLayout->addWidget(dailyTicketStatus_);
    queryLayout->addWidget(dailyTicketsBox, 1, 0);
    QGroupBox* ticketBox = new QGroupBox(QStringLiteral("Chi tiết vé đang chọn"), queryTab);
    QVBoxLayout* ticketLayout = new QVBoxLayout(ticketBox);
    ticketResult_ = new QLabel(QStringLiteral("Chọn một vé trong danh sách hoặc nhập mã vé để xem chi tiết."), ticketBox);
    ticketResult_->setObjectName("ticketResult");
    ticketResult_->setTextFormat(Qt::PlainText);
    ticketResult_->setWordWrap(true);
    ticketResult_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ticketResult_->setMinimumWidth(280);
    ticketLayout->addWidget(ticketResult_, 1);
    queryLayout->addWidget(ticketBox, 1, 1);
    queryLayout->setColumnStretch(0, 3);
    queryLayout->setColumnStretch(1, 2);
    queryLayout->setRowStretch(1, 1);

    connect(ticketLookup, &QPushButton::clicked, this, &MainWindow::lookupTicket);
    connect(ticketId_, &QLineEdit::returnPressed, this, &MainWindow::lookupTicket);
    connect(reportDate_, &QDateEdit::dateChanged, this, [this] {
        ticketId_->clear();
        ticketResult_->setText(QStringLiteral("Chọn một vé trong danh sách hoặc nhập mã vé để xem chi tiết."));
        refreshRevenue();
    });
    connect(dailyTicketTable_, &QTableWidget::itemSelectionChanged, this, [this] {
        const QTableWidgetItem* item = dailyTicketTable_->item(dailyTicketTable_->currentRow(), 0);
        if (item == nullptr || dailyTicketTable_->selectedItems().isEmpty()) {
            return;
        }
        const EntityId id = item->data(Qt::DisplayRole).toLongLong();
        ticketId_->setText(QString::number(id));
        showTicketDetails(id);
        dailyTicketStatus_->setText(QStringLiteral("Đang xem vé #%1.").arg(id));
    });
}

EntityId MainWindow::selectedFilm() const {
    const QTableWidgetItem* item = filmTable_->item(filmTable_->currentRow(), 0);
    if (item == nullptr || filmTable_->selectedItems().isEmpty()) {
        return 0;
    }
    return item->data(Qt::DisplayRole).toLongLong();
}

EntityId MainWindow::selectedRoom() const {
    const QTableWidgetItem* item = roomTable_->item(roomTable_->currentRow(), 0);
    if (item == nullptr || roomTable_->selectedItems().isEmpty()) {
        return 0;
    }
    return item->data(Qt::DisplayRole).toLongLong();
}

EntityId MainWindow::selectedScreening() const {
    const QTableWidgetItem* item = screeningTable_->item(screeningTable_->currentRow(), 0);
    if (item == nullptr || screeningTable_->selectedItems().isEmpty()) {
        return 0;
    }
    return item->data(Qt::DisplayRole).toLongLong();
}

EntityId MainWindow::selectedSaleScreening() const {
    const QTableWidgetItem* item = saleScreeningTable_->item(saleScreeningTable_->currentRow(), 0);
    if (item == nullptr || saleScreeningTable_->selectedItems().isEmpty()) {
        return 0;
    }
    return item->data(Qt::DisplayRole).toLongLong();
}

void MainWindow::refreshReferenceCombos() {
    populateFilms(screeningFilmFilter_, catalog_, true);
    populateRooms(screeningRoomFilter_, catalog_, true);
    populateFilms(saleFilmFilter_, catalog_, true);
    populateRooms(saleRoomFilter_, catalog_, true);
}

void MainWindow::refreshFilms(EntityId selectId) {
    const QSignalBlocker block(filmTable_);
    filmTable_->setSortingEnabled(false);
    filmTable_->setRowCount(0);
    filmEdit_->setEnabled(false);
    filmRemove_->setEnabled(false);
    try {
        LinkedList<Film> films;
        const QString query = filmSearch_->text().trimmed();
        if (query.isEmpty() || filmSearchMode_->currentIndex() == 0) {
            catalog_.films(toUtf8(query), films);
        } else {
            bool valid = false;
            const EntityId id = query.toLongLong(&valid);
            if (!valid || id <= 0) {
                filmTable_->setSortingEnabled(true);
                filmResults_->setText(QStringLiteral("Mã phim phải là số nguyên dương trong miền int64."));
                return;
            }
            try { films.pushBack(catalog_.film(id)); } catch (const std::out_of_range&) {}
        }
        for (const Film& film : films) {
            const int row = filmTable_->rowCount();
            filmTable_->insertRow(row);
            setNumber(filmTable_, row, 0, film.id());
            QTableWidgetItem* title = new QTableWidgetItem(fromUtf8(film.title()));
            title->setToolTip(title->text());
            filmTable_->setItem(row, 1, title);
            setNumber(filmTable_, row, 2, film.durationMinutes());
        }
        filmTable_->setSortingEnabled(true);
        for (int row = 0; row < filmTable_->rowCount(); ++row) {
            if (filmTable_->item(row, 0)->data(Qt::DisplayRole).toLongLong() == selectId) {
                filmTable_->selectRow(row);
                filmEdit_->setEnabled(true);
                filmRemove_->setEnabled(true);
                break;
            }
        }
        filmResults_->setText(filmTable_->rowCount() == 0
            ? QStringLiteral("Không có phim phù hợp. Chọn Thêm phim để tạo bản ghi.")
            : QStringLiteral("%1 phim").arg(filmTable_->rowCount()));
    } catch (const std::exception& error) {
        filmResults_->setText(errorText(error));
    }
}

void MainWindow::editFilm(bool creating) {
    const EntityId id = creating ? 0 : selectedFilm();
    if (!creating && id == 0) {
        return;
    }
    try {
        FilmDialog dialog(catalog_, id, this);
        if (dialog.exec() == QDialog::Accepted) {
            filmSearch_->clear();
            refreshFilms(dialog.filmId);
            refreshReferenceCombos();
            refreshScreenings();
            refreshSaleScreenings();
            updateStatus();
        }
    } catch (const std::exception& error) {
        filmResults_->setText(errorText(error));
    }
}

void MainWindow::deleteFilm() {
    const EntityId id = selectedFilm();
    if (id == 0) {
        return;
    }
    try {
        const QString name = fromUtf8(catalog_.film(id).title());
        if (!confirm(this, QStringLiteral("Xác nhận xóa"),
                     QStringLiteral("Xóa phim #%1 — %2?").arg(id).arg(name), QStringLiteral("Xóa phim"))) {
            return;
        }
        catalog_.eraseFilm(id);
        refreshFilms();
        refreshReferenceCombos();
        refreshScreenings();
        refreshSaleScreenings();
        updateStatus();
    } catch (const std::exception& error) {
        filmResults_->setText(errorText(error));
    }
}

void MainWindow::refreshRooms(EntityId selectId) {
    const QSignalBlocker block(roomTable_);
    roomTable_->setSortingEnabled(false);
    roomTable_->setRowCount(0);
    roomEdit_->setEnabled(false);
    roomRemove_->setEnabled(false);
    try {
        LinkedList<Auditorium> rooms;
        catalog_.rooms(rooms);
        for (const Auditorium& room : rooms) {
            const int row = roomTable_->rowCount();
            roomTable_->insertRow(row);
            setNumber(roomTable_, row, 0, room.id());
            roomTable_->setItem(row, 1, new QTableWidgetItem(fromUtf8(room.name())));
            setNumber(roomTable_, row, 2, room.rowCount());
            setNumber(roomTable_, row, 3, room.seatsPerRow());
            setNumber(roomTable_, row, 4, room.capacity());
        }
        roomTable_->setSortingEnabled(true);
        for (int row = 0; row < roomTable_->rowCount(); ++row) {
            if (roomTable_->item(row, 0)->data(Qt::DisplayRole).toLongLong() == selectId) {
                roomTable_->selectRow(row);
                roomEdit_->setEnabled(true);
                roomRemove_->setEnabled(true);
                break;
            }
        }
        roomResults_->setText(roomTable_->rowCount() == 0
            ? QStringLiteral("Chưa có phòng. Chọn Thêm phòng để tự sinh sơ đồ ghế.")
            : QStringLiteral("%1 phòng").arg(roomTable_->rowCount()));
        refreshRoomSeats();
    } catch (const std::exception& error) {
        roomResults_->setText(errorText(error));
    }
}

void MainWindow::refreshRoomSeats() {
    roomSeatTable_->clear();
    roomSeatTable_->setRowCount(0);
    roomSeatTable_->setColumnCount(0);
    const EntityId id = selectedRoom();
    if (id == 0) {
        return;
    }
    try {
        std::unique_ptr<SeatMap> map = catalog_.seatMap(id);
        roomSeatTable_->setRowCount(static_cast<int>(map->rows()));
        roomSeatTable_->setColumnCount(static_cast<int>(map->columns()));
        QStringList columns;
        for (std::size_t column = 0; column < map->columns(); ++column) {
            columns << QString::number(column + 1);
        }
        QStringList rows;
        for (std::size_t row = 0; row < map->rows(); ++row) {
            rows << QStringLiteral("H%1").arg(row + 1);
            for (std::size_t column = 0; column < map->columns(); ++column) {
                QTableWidgetItem* item = new QTableWidgetItem(seatName(row, column));
                item->setTextAlignment(Qt::AlignCenter);
                item->setToolTip(QStringLiteral("Mã ghế: %1").arg(map->at(row, column).seatId));
                roomSeatTable_->setItem(static_cast<int>(row), static_cast<int>(column), item);
            }
        }
        roomSeatTable_->setHorizontalHeaderLabels(columns);
        roomSeatTable_->setVerticalHeaderLabels(rows);
        roomSeatTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        roomSeatTable_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } catch (const std::exception& error) {
        roomResults_->setText(errorText(error));
    }
}

void MainWindow::editRoom(bool creating) {
    const EntityId id = creating ? 0 : selectedRoom();
    if (!creating && id == 0) {
        return;
    }
    try {
        RoomDialog dialog(catalog_, id, this);
        if (dialog.exec() == QDialog::Accepted) {
            refreshRooms(dialog.roomId);
            refreshReferenceCombos();
            refreshScreenings();
            refreshSaleScreenings();
            updateStatus();
        }
    } catch (const std::exception& error) {
        roomResults_->setText(errorText(error));
    }
}

void MainWindow::deleteRoom() {
    const EntityId id = selectedRoom();
    if (id == 0) {
        return;
    }
    try {
        const QString name = fromUtf8(catalog_.room(id).name());
        if (!confirm(this, QStringLiteral("Xác nhận xóa"),
                     QStringLiteral("Xóa phòng #%1 — %2 và toàn bộ ghế của phòng?").arg(id).arg(name),
                     QStringLiteral("Xóa phòng"))) {
            return;
        }
        catalog_.eraseRoom(id);
        refreshRooms();
        refreshReferenceCombos();
        refreshScreenings();
        refreshSaleScreenings();
        updateStatus();
    } catch (const std::exception& error) {
        roomResults_->setText(errorText(error));
    }
}

void MainWindow::refreshScreenings(EntityId selectId) {
    const QSignalBlocker block(screeningTable_);
    screeningTable_->setSortingEnabled(false);
    screeningTable_->setRowCount(0);
    screeningEdit_->setEnabled(false);
    screeningRemove_->setEnabled(false);
    try {
        EntityId requestedId = 0;
        if (!screeningIdFilter_->text().isEmpty()) {
            bool valid = false;
            requestedId = screeningIdFilter_->text().toLongLong(&valid);
            if (!valid || requestedId <= 0) {
                screeningTable_->setSortingEnabled(true);
                screeningResults_->setText(QStringLiteral("Mã suất phải là số nguyên dương trong miền int64."));
                return;
            }
        }
        const std::string date = screeningUseDate_->isChecked()
            ? toUtf8(screeningDate_->date().toString("yyyy-MM-dd")) : std::string{};
        LinkedList<Screening> screenings;
        screenings_.list(date, comboId(screeningFilmFilter_), comboId(screeningRoomFilter_),
                         false, screenings);
        for (const Screening& screening : screenings) {
            if (requestedId != 0 && screening.id() != requestedId) {
                continue;
            }
            const int row = screeningTable_->rowCount();
            screeningTable_->insertRow(row);
            setNumber(screeningTable_, row, 0, screening.id());
            screeningTable_->setItem(row, 1, new QTableWidgetItem(fromUtf8(catalog_.film(screening.filmId()).title())));
            screeningTable_->setItem(row, 2, new QTableWidgetItem(fromUtf8(catalog_.room(screening.auditoriumId()).name())));
            screeningTable_->setItem(row, 3, new QTableWidgetItem(fromUtf8(screening.startTime())));
            screeningTable_->setItem(row, 4, new QTableWidgetItem(fromUtf8(screenings_.endTime(screening.id()))));
            screeningTable_->setItem(row, 5, new MoneyItem(screening.ticketPrice()));
        }
        screeningTable_->setSortingEnabled(true);
        for (int row = 0; row < screeningTable_->rowCount(); ++row) {
            if (screeningTable_->item(row, 0)->data(Qt::DisplayRole).toLongLong() == selectId) {
                screeningTable_->selectRow(row);
                screeningEdit_->setEnabled(true);
                screeningRemove_->setEnabled(true);
                break;
            }
        }
        screeningResults_->setText(screeningTable_->rowCount() == 0
            ? QStringLiteral("Không có suất phù hợp. Có thể bỏ bộ lọc hoặc thêm suất mới.")
            : QStringLiteral("%1 suất chiếu").arg(screeningTable_->rowCount()));
    } catch (const std::exception& error) {
        screeningResults_->setText(errorText(error));
    }
}

void MainWindow::editScreening(bool creating) {
    const EntityId id = creating ? 0 : selectedScreening();
    if (!creating && id == 0) {
        return;
    }
    try {
        ScreeningDialog dialog(catalog_, screenings_, id, this);
        if (dialog.exec() == QDialog::Accepted) {
            screeningIdFilter_->clear();
            screeningUseDate_->setChecked(false);
            refreshScreenings(dialog.screeningId);
            refreshSaleScreenings(dialog.screeningId);
            updateStatus();
        }
    } catch (const std::exception& error) {
        screeningResults_->setText(errorText(error));
    }
}

void MainWindow::deleteScreening() {
    const EntityId id = selectedScreening();
    if (id == 0) {
        return;
    }
    try {
        if (!confirm(this, QStringLiteral("Xác nhận xóa"),
                     QStringLiteral("Xóa suất chiếu #%1?").arg(id), QStringLiteral("Xóa suất"))) {
            return;
        }
        screenings_.erase(id);
        refreshScreenings();
        refreshSaleScreenings();
        updateStatus();
    } catch (const std::exception& error) {
        screeningResults_->setText(errorText(error));
    }
}

void MainWindow::refreshSaleScreenings(EntityId selectId) {
    const QSignalBlocker block(saleScreeningTable_);
    saleScreeningTable_->setSortingEnabled(false);
    saleScreeningTable_->setRowCount(0);
    selectedSaleSeat_ = 0;
    sellButton_->setEnabled(false);
    saleSeatTable_->clear();
    saleSeatTable_->setRowCount(0);
    saleSeatTable_->setColumnCount(0);
    saleDetails_->setText(QStringLiteral("Chọn một suất chiếu và một ghế trống."));
    saleResults_->clear();
    try {
        const std::string date = saleUseDate_->isChecked()
            ? toUtf8(saleDate_->date().toString("yyyy-MM-dd")) : std::string{};
        LinkedList<Screening> screenings;
        screenings_.list(date, comboId(saleFilmFilter_), comboId(saleRoomFilter_), true, screenings);
        for (const Screening& screening : screenings) {
            const int row = saleScreeningTable_->rowCount();
            saleScreeningTable_->insertRow(row);
            setNumber(saleScreeningTable_, row, 0, screening.id());
            saleScreeningTable_->setItem(row, 1, new QTableWidgetItem(fromUtf8(catalog_.film(screening.filmId()).title())));
            saleScreeningTable_->setItem(row, 2, new QTableWidgetItem(fromUtf8(catalog_.room(screening.auditoriumId()).name())));
            saleScreeningTable_->setItem(row, 3, new QTableWidgetItem(fromUtf8(screening.startTime())));
            saleScreeningTable_->setItem(row, 4, new QTableWidgetItem(fromUtf8(screenings_.endTime(screening.id()))));
            saleScreeningTable_->setItem(row, 5, new MoneyItem(screening.ticketPrice()));
        }
        saleScreeningTable_->setSortingEnabled(true);
        for (int row = 0; row < saleScreeningTable_->rowCount(); ++row) {
            if (saleScreeningTable_->item(row, 0)->data(Qt::DisplayRole).toLongLong() == selectId) {
                saleScreeningTable_->selectRow(row);
                refreshSaleSeats();
                break;
            }
        }
        if (saleScreeningTable_->rowCount() == 0) {
            saleResults_->setText(QStringLiteral("Không có suất chưa bắt đầu phù hợp với bộ lọc."));
        }
    } catch (const std::exception& error) {
        saleResults_->setText(errorText(error));
    }
}

void MainWindow::refreshSaleSeats() {
    selectedSaleSeat_ = 0;
    sellButton_->setEnabled(false);
    saleSeatTable_->clear();
    saleSeatTable_->setRowCount(0);
    saleSeatTable_->setColumnCount(0);
    const EntityId screeningId = selectedSaleScreening();
    if (screeningId == 0) {
        saleDetails_->setText(QStringLiteral("Chọn một suất chiếu và một ghế trống."));
        return;
    }
    try {
        const Screening& screening = screenings_.find(screeningId);
        const Film& film = catalog_.film(screening.filmId());
        const Auditorium& room = catalog_.room(screening.auditoriumId());
        saleDetails_->setText(QStringLiteral("Phim: %1\nPhòng: %2\nChiếu: %3 — %4\nGiá: %5\n\nChọn một ghế trống.")
            .arg(fromUtf8(film.title()), fromUtf8(room.name()), fromUtf8(screening.startTime()),
                 fromUtf8(screenings_.endTime(screeningId)), money(screening.ticketPrice())));
        std::unique_ptr<SeatMap> map = tickets_.seatMap(screeningId);
        saleSeatTable_->setRowCount(static_cast<int>(map->rows()));
        saleSeatTable_->setColumnCount(static_cast<int>(map->columns()));
        QStringList columns;
        for (std::size_t column = 0; column < map->columns(); ++column) {
            columns << QString::number(column + 1);
        }
        QStringList rows;
        for (std::size_t row = 0; row < map->rows(); ++row) {
            rows << QStringLiteral("H%1").arg(row + 1);
            for (std::size_t column = 0; column < map->columns(); ++column) {
                const SeatStatus& seat = map->at(row, column);
                const QString name = seatName(row, column);
                QTableWidgetItem* item = new QTableWidgetItem((seat.sold ? QStringLiteral("× ") : QStringLiteral("○ ")) + name);
                item->setTextAlignment(Qt::AlignCenter);
                item->setData(Qt::UserRole, QVariant::fromValue<qlonglong>(seat.seatId));
                item->setData(Qt::UserRole + 1, seat.sold);
                item->setToolTip(seat.sold ? name + QStringLiteral(" — đã bán") : name + QStringLiteral(" — còn trống"));
                if (seat.sold) {
                    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
                }
                saleSeatTable_->setItem(static_cast<int>(row), static_cast<int>(column), item);
            }
        }
        saleSeatTable_->setHorizontalHeaderLabels(columns);
        saleSeatTable_->setVerticalHeaderLabels(rows);
        saleSeatTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        saleSeatTable_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        saleResults_->clear();
    } catch (const std::exception& error) {
        saleResults_->setText(errorText(error));
    }
}

void MainWindow::selectSaleSeat(int row, int column) {
    QTableWidgetItem* selected = saleSeatTable_->item(row, column);
    if (selected == nullptr || selected->data(Qt::UserRole + 1).toBool()) {
        return;
    }
    try {
        for (int currentRow = 0; currentRow < saleSeatTable_->rowCount(); ++currentRow) {
            for (int currentColumn = 0; currentColumn < saleSeatTable_->columnCount(); ++currentColumn) {
                QTableWidgetItem* item = saleSeatTable_->item(currentRow, currentColumn);
                if (item && !item->data(Qt::UserRole + 1).toBool()) {
                    item->setText(QStringLiteral("○ ") + seatName(currentRow, currentColumn));
                }
            }
        }
        selectedSaleSeat_ = selected->data(Qt::UserRole).toLongLong();
        const SaleDetails preview = tickets_.preview(selectedSaleScreening(), selectedSaleSeat_);
        selected->setText(QStringLiteral("✓ ") + seatName(row, column));
        saleDetails_->setText(QStringLiteral("Phim: %1\nPhòng: %2\nChiếu: %3 — %4\nGhế: %5\nGiá: %6")
            .arg(fromUtf8(preview.film.title()), fromUtf8(preview.room.name()),
                 fromUtf8(preview.screening.startTime()), fromUtf8(preview.endTime),
                 seatName(static_cast<std::size_t>(preview.seat.rowNumber() - 1),
                          static_cast<std::size_t>(preview.seat.seatNumber() - 1)),
                 money(preview.screening.ticketPrice())));
        saleResults_->clear();
        sellButton_->setEnabled(true);
    } catch (const std::exception& error) {
        selectedSaleSeat_ = 0;
        sellButton_->setEnabled(false);
        const QString message = errorText(error);
        refreshSaleSeats();
        saleResults_->setText(message);
    }
}

void MainWindow::sellTicket() {
    const EntityId screeningId = selectedSaleScreening();
    if (screeningId == 0 || selectedSaleSeat_ == 0) {
        return;
    }
    try {
        const SaleDetails preview = tickets_.preview(screeningId, selectedSaleSeat_);
        const QString name = seatName(static_cast<std::size_t>(preview.seat.rowNumber() - 1),
                                   static_cast<std::size_t>(preview.seat.seatNumber() - 1));
        if (!confirm(this, QStringLiteral("Xác nhận bán vé"),
                     QStringLiteral("Đã nhận %1 tiền mặt cho %2 — %3, ghế %4?")
                         .arg(money(preview.screening.ticketPrice()), fromUtf8(preview.film.title()),
                              fromUtf8(preview.room.name()), name),
                     QStringLiteral("Đã nhận tiền"))) {
            return;
        }
        const EntityId ticketId = tickets_.sell(screeningId, selectedSaleSeat_, true);
        refreshSaleSeats();
        saleResults_->setText(QStringLiteral("Đã bán vé #%1. Chưa lưu vào tệp; có thể xem lại trong tab Vé và doanh thu.").arg(ticketId));
        ticketId_->setText(QString::number(ticketId));
        lookupTicket();
        updateStatus();
    } catch (const std::exception& error) {
        const QString message = errorText(error);
        refreshSaleSeats();
        saleResults_->setText(message);
    }
}

void MainWindow::lookupTicket() {
    std::int64_t id = 0;
    if (!positiveValue(ticketId_, id)) {
        dailyTicketTable_->clearSelection();
        dailyTicketStatus_->setText(QStringLiteral("Nhập mã vé hợp lệ để chuyển đến vé cần xem."));
        ticketResult_->setText(QStringLiteral("Mã vé phải là số nguyên dương trong miền int64."));
        ticketId_->setFocus();
        return;
    }
    try {
        const TicketDetails details = tickets_.find(id);
        const QSignalBlocker block(reportDate_);
        reportDate_->setDate(QDate::fromString(fromUtf8(details.ticket.soldAt().substr(0, 10)), "yyyy-MM-dd"));
        refreshRevenue(id);
        showTicketDetails(id);
    } catch (const std::exception& error) {
        dailyTicketTable_->clearSelection();
        dailyTicketStatus_->setText(QStringLiteral("Không thể mở mã vé đã nhập."));
        ticketResult_->setText(errorText(error));
    }
}

void MainWindow::showTicketDetails(EntityId ticketId) {
    try {
        const TicketDetails details = tickets_.find(ticketId);
        const QString seat = seatName(static_cast<std::size_t>(details.sale.seat.rowNumber() - 1),
                                   static_cast<std::size_t>(details.sale.seat.seatNumber() - 1));
        ticketResult_->setText(QStringLiteral("Vé #%1\nPhim: %2\nSuất #%3: %4 — %5\nPhòng: %6\nGhế: %7\nBán lúc: %8\nGiá đã bán: %9")
            .arg(details.ticket.id()).arg(fromUtf8(details.sale.film.title()))
            .arg(details.sale.screening.id()).arg(fromUtf8(details.sale.screening.startTime()))
            .arg(fromUtf8(details.sale.endTime)).arg(fromUtf8(details.sale.room.name()))
            .arg(seat).arg(fromUtf8(details.ticket.soldAt())).arg(money(details.ticket.unitPrice())));
    } catch (const std::exception& error) {
        ticketResult_->setText(errorText(error));
    }
}

void MainWindow::refreshRevenue(EntityId selectTicketId) {
    const QSignalBlocker block(dailyTicketTable_);
    dailyTicketTable_->setSortingEnabled(false);
    dailyTicketTable_->setRowCount(0);
    try {
        const std::string saleDate = toUtf8(reportDate_->date().toString("yyyy-MM-dd"));
        const DailyRevenue revenue = tickets_.revenue(saleDate);
        reportCount_->setText(QString::number(static_cast<qulonglong>(revenue.ticketCount)));
        reportRevenue_->setText(money(revenue.amount));
        LinkedList<TicketDetails> tickets;
        tickets_.listBySaleDate(saleDate, tickets);
        for (const TicketDetails& details : tickets) {
            const int row = dailyTicketTable_->rowCount();
            dailyTicketTable_->insertRow(row);
            setNumber(dailyTicketTable_, row, 0, details.ticket.id());
            dailyTicketTable_->setItem(row, 1, new QTableWidgetItem(fromUtf8(details.ticket.soldAt().substr(11, 5))));
            dailyTicketTable_->setItem(row, 2, new QTableWidgetItem(fromUtf8(details.sale.film.title())));
            dailyTicketTable_->setItem(row, 3, new QTableWidgetItem(fromUtf8(details.sale.room.name())));
            dailyTicketTable_->setItem(row, 4, new QTableWidgetItem(seatName(
                static_cast<std::size_t>(details.sale.seat.rowNumber() - 1),
                static_cast<std::size_t>(details.sale.seat.seatNumber() - 1))));
            dailyTicketTable_->setItem(row, 5, new MoneyItem(details.ticket.unitPrice()));
        }
        dailyTicketTable_->setSortingEnabled(true);
        for (int row = 0; row < dailyTicketTable_->rowCount(); ++row) {
            if (dailyTicketTable_->item(row, 0)->data(Qt::DisplayRole).toLongLong() == selectTicketId) {
                dailyTicketTable_->selectRow(row);
                dailyTicketTable_->scrollToItem(dailyTicketTable_->item(row, 0));
                break;
            }
        }
        dailyTicketStatus_->setText(dailyTicketTable_->rowCount() == 0
            ? QStringLiteral("Không có vé bán trong ngày này.")
            : selectTicketId == 0
                ? QStringLiteral("Chọn một vé trong bảng để xem đầy đủ thông tin.")
                : QStringLiteral("Đã mở vé #%1 trong ngày bán tương ứng.").arg(selectTicketId));
    } catch (const std::exception& error) {
        reportCount_->setText(QStringLiteral("—"));
        reportRevenue_->setText(errorText(error));
        dailyTicketTable_->setSortingEnabled(true);
        dailyTicketStatus_->setText(errorText(error));
    }
}

void MainWindow::updateStatus() {
    setWindowModified(data_.isDirty());
    setWindowTitle(QStringLiteral("Quản lý rạp chiếu phim[*]"));
    saveState_->setText(data_.isDirty() ? QStringLiteral("Có thay đổi chưa lưu")
                                       : QStringLiteral("Không có thay đổi chưa lưu"));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    event->ignore();
    if (!confirm(this, QStringLiteral("Lưu và thoát"),
                 QStringLiteral("Ghi toàn bộ dữ liệu vào TXT rồi thoát? Nếu lưu lỗi, cửa sổ sẽ giữ nguyên."),
                 QStringLiteral("Lưu và thoát"))) {
        return;
    }
    try {
        if (data_.needsManualRecovery()) {
            data_.retryRecovery();
        }
        data_.save();
        updateStatus();
        event->accept();
    } catch (const std::exception& error) {
        updateStatus();
        QMessageBox box(QMessageBox::Critical, QStringLiteral("Lưu thất bại"),
                        QStringLiteral("Ứng dụng vẫn mở và giữ dữ liệu trên RAM. "
                                       "Sửa nguyên nhân rồi chọn Lưu và thoát để thử lại.\n\n")
                            + fromUtf8(error.what()), QMessageBox::Ok, this);
        box.setTextFormat(Qt::PlainText);
        box.exec();
    }
}
