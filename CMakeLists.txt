#include "MainWindow.h"
#include "VaultEngine.h"

FileCard::FileCard(const QString &encryptedName, const QString &displayName,
                   const QString &category, qint64 fileSize,
                   const QString &fileType, QWidget *parent)
    : QFrame(parent)
    , m_encryptedName(encryptedName)
    , m_displayName(displayName)
    , m_category(category)
    , m_fileSize(fileSize)
    , m_fileType(fileType)
    , m_isHovered(false)
    , m_isSelected(false)
{
    setupUI();
}

FileCard::~FileCard()
{
}

void FileCard::setupUI()
{
    setFixedSize(180, 160);
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::NoFrame);
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(12);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 16, 12, 12);
    mainLayout->setSpacing(8);
    
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(56, 56);
    
    QString iconStyle = QString(
        "QLabel {"
        "   background-color: %1;"
        "   border-radius: 28px;"
        "   font-size: 28px;"
        "   color: white;"
        "}"
    ).arg(getCategoryColor(m_category).name());
    
    m_iconLabel->setStyleSheet(iconStyle);
    m_iconLabel->setText(getFileIconPath(m_fileType, m_category));
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    
    m_nameLabel = new QLabel(this);
    m_nameLabel->setText(m_displayName.length() > 20 ? 
                         m_displayName.left(17) + "..." : m_displayName);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet(
        "QLabel {"
        "   color: #202124;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   background: transparent;"
        "}"
    );
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    
    m_sizeLabel = new QLabel(formatFileSize(m_fileSize), this);
    m_sizeLabel->setAlignment(Qt::AlignCenter);
    m_sizeLabel->setStyleSheet(
        "QLabel {"
        "   color: #5F6368;"
        "   font-size: 11px;"
        "   background: transparent;"
        "}"
    );
    mainLayout->addWidget(m_sizeLabel, 0, Qt::AlignCenter);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(6);
    
    m_exportButton = new QPushButton("⬇", this);
    m_exportButton->setFixedSize(28, 28);
    m_exportButton->setToolTip("Export File");
    m_exportButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #E8F0FE;"
        "   border: none;"
        "   border-radius: 14px;"
        "   color: #1A73E8;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #D2E3FC;"
        "}"
    );
    connect(m_exportButton, &QPushButton::clicked, this, [this]() {
        emit exportRequested(m_encryptedName);
    });
    
    m_deleteButton = new QPushButton("🗑", this);
    m_deleteButton->setFixedSize(28, 28);
    m_deleteButton->setToolTip("Delete File");
    m_deleteButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #FEE2E2;"
        "   border: none;"
        "   border-radius: 14px;"
        "   color: #DC2626;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #FECACA;"
        "}"
    );
    connect(m_deleteButton, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_encryptedName);
    });
    
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    setStyleSheet(
        "FileCard {"
        "   background-color: #FFFFFF;"
        "   border-radius: 12px;"
        "}"
    );
}

QString FileCard::getFileIconPath(const QString &fileType, const QString &category) const
{
    if (category == "images") return "🖼";
    if (category == "documents") return "📄";
    if (category == "videos") return "🎬";
    if (category == "archives") return "📦";
    if (category == "audio") return "🎵";
    return "📁";
}

QString FileCard::formatFileSize(qint64 bytes) const
{
    qint64 gb = 1024 * 1024 * 1024;
    qint64 mb = 1024 * 1024;
    qint64 kb = 1024;
    
    if (bytes >= gb) {
        return QString("%1 GB").arg(static_cast<double>(bytes) / gb, 0, 'f', 1);
    } else if (bytes >= mb) {
        return QString("%1 MB").arg(static_cast<double>(bytes) / mb, 0, 'f', 1);
    } else if (bytes >= kb) {
        return QString("%1 KB").arg(bytes / kb);
    }
    return QString("%1 B").arg(bytes);
}

void FileCard::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = true;
    setStyleSheet(
        "FileCard {"
        "   background-color: #F8F9FA;"
        "   border-radius: 12px;"
        "   border: 1px solid #E8EAED;"
        "}"
    );
}

void FileCard::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = false;
    if (!m_isSelected) {
        setStyleSheet(
            "FileCard {"
            "   background-color: #FFFFFF;"
            "   border-radius: 12px;"
            "}"
        );
    }
}

void FileCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelected = true;
        setStyleSheet(
            "FileCard {"
            "   background-color: #E8F0FE;"
            "   border-radius: 12px;"
            "   border: 2px solid #1A73E8;"
            "}"
        );
        emit cardClicked(m_encryptedName);
    }
    QFrame::mousePressEvent(event);
}

void FileCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit cardDoubleClicked(m_encryptedName);
    }
    QFrame::mouseDoubleClickEvent(event);
}

void FileCard::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QPE_Widget, &opt, &p, this);
    QFrame::paintEvent(event);
}

void FileCard::setHighlighted(bool highlighted)
{
    m_isSelected = highlighted;
    if (highlighted) {
        setStyleSheet(
            "FileCard {"
            "   background-color: #E8F0FE;"
            "   border-radius: 12px;"
            "   border: 2px solid #1A73E8;"
            "}"
        );
    } else {
        setStyleSheet(
            "FileCard {"
            "   background-color: #FFFFFF;"
            "   border-radius: 12px;"
            "}"
        );
    }
}

CircularProgress::CircularProgress(QWidget *parent)
    : QWidget(parent)
    , m_value(0)
    , m_currentSize(0)
    , m_maxSize(1073741824)
{
    setFixedSize(48, 48);
}

void CircularProgress::setValue(int value)
{
    m_value = qBound(0, value, 100);
    update();
    emit valueChanged(m_value);
}

void CircularProgress::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int side = qMin(width(), height());
    QRectF outerRect((width() - side) / 2.0, (height() - side) / 2.0, side, side);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#E8EAED"));
    painter.drawEllipse(outerRect);
    
    qreal percentage = m_maxSize > 0 ? static_cast<qreal>(m_currentSize) / m_maxSize : 0;
    int spanAngle = static_cast<int>(-percentage * 360 * 16);
    
    if (spanAngle != 0) {
        QRadialGradient gradient(outerRect.center(), side / 2.0);
        gradient.setColorAt(0, "#4285F4");
        gradient.setColorAt(1, "#1A73E8");
        
        QPen pen(QBrush(gradient), 6);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawArc(outerRect.adjusted(6, 6, -6, -6), 90 * 16, spanAngle);
    }
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1A73E8"));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(10);
    painter.setFont(font);
    painter.drawText(outerRect, Qt::AlignCenter, QString("%1%").arg(qRound(percentage)));
}

CategoryCard::CategoryCard(const QString &name, const QString &icon,
                           int count, const QColor &color, QWidget *parent)
    : QFrame(parent)
    , m_categoryName(name)
    , m_isHovered(false)
{
    setupUI();
    
    m_iconLabel->setText(icon);
    m_nameLabel->setText(name);
    m_countLabel->setText(QString::number(count));
    
    QString cardStyle = QString(
        "QFrame {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "}"
    ).arg(color.lighter(120).name());
    
    setStyleSheet(cardStyle);
}

CategoryCard::~CategoryCard()
{
}

void CategoryCard::setupUI()
{
    setFixedSize(140, 100);
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::NoFrame);
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);
    
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_iconLabel->setStyleSheet("font-size: 24px; background: transparent;");
    layout->addWidget(m_iconLabel);
    
    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet(
        "QLabel {"
        "   color: #202124;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   background: transparent;"
        "}"
    );
    layout->addWidget(m_nameLabel);
    
    m_countLabel = new QLabel(this);
    m_countLabel->setStyleSheet(
        "QLabel {"
        "   color: #5F6368;"
        "   font-size: 12px;"
        "   background: transparent;"
        "}"
    );
    layout->addWidget(m_countLabel);
    
    layout->addStretch();
}

void CategoryCard::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = true;
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
}

void CategoryCard::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = false;
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
}

void CategoryCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit categoryClicked(m_categoryName);
    }
    QFrame::mousePressEvent(event);
}

PasswordDialog::PasswordDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
{
    setupUI();
}

PasswordDialog::~PasswordDialog()
{
}

void PasswordDialog::setupUI()
{
    setWindowTitle("Sphinx Vault");
    setModal(true);
    setFixedSize(420, 320);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 24);
    mainLayout->setSpacing(16);
    
    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #1A73E8;"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   background: transparent;"
        "}"
    );
    
    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setWordWrap(true);
    m_subtitleLabel->setStyleSheet(
        "QLabel {"
        "   color: #5F6368;"
        "   font-size: 14px;"
        "   background: transparent;"
        "   padding: 0 20px;"
        "}"
    );
    
    if (m_mode == CreateMode) {
        m_titleLabel->setText("🦅 Create Master Password");
        m_subtitleLabel->setText("Create a strong password to secure your vault.\nThis password cannot be recovered if lost.");
    } else {
        m_titleLabel->text() = "🔓 Unlock Vault";
        m_subtitleLabel->setText("Enter your master password to access your files.");
    }
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_subtitleLabel);
    
    mainLayout->addSpacing(16);
    
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("Enter password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(44);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 2px solid #DADCE0;"
        "   border-radius: 8px;"
        "   padding: 0 16px;"
        "   font-size: 14px;"
        "   background: white;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #1A73E8;"
        "}"
        "QLineEdit:text {"
        "   color: #202124;"
        "}"
    );
    mainLayout->addWidget(m_passwordEdit);
    
    if (m_mode == CreateMode) {
        m_confirmEdit = new QLineEdit(this);
        m_confirmEdit->setPlaceholderText("Confirm password");
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setMinimumHeight(44);
        m_confirmEdit->setStyleSheet(
            "QLineEdit {"
            "   border: 2px solid #DADCE0;"
            "   border-radius: 8px;"
            "   padding: 0 16px;"
            "   font-size: 14px;"
            "   background: white;"
            "}"
            "QLineEdit:focus {"
            "   border: 2px solid #1A73E8;"
            "}"
        );
        mainLayout->addWidget(m_confirmEdit);
        
        m_strengthLabel = new QLabel("Password Strength: -", this);
        m_strengthLabel->setAlignment(Qt::AlignCenter);
        m_strengthLabel->setStyleSheet(
            "QLabel {"
            "   color: #5F6368;"
            "   font-size: 12px;"
            "   background: transparent;"
            "}"
        );
        mainLayout->addWidget(m_strengthLabel);
        
        connect(m_confirmEdit, &QLineEdit::textChanged, this, &PasswordDialog::onTextChanged);
    }
    
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &PasswordDialog::onTextChanged);
    
    mainLayout->addStretch();
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setFixedHeight(44);
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F3F4;"
        "   border: none;"
        "   border-radius: 8px;"
        "   color: #5F6368;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   padding: 0 24px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E8EAED;"
        "}"
    );
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    m_okButton = new QPushButton(m_mode == CreateMode ? "Create Vault" : "Unlock", this);
    m_okButton->setFixedHeight(44);
    m_okButton->setEnabled(false);
    m_okButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1A73E8;"
        "   border: none;"
        "   border-radius: 8px;"
        "   color: white;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   padding: 0 24px;"
        "}"
        "QPushButton:hover:!disabled {"
        "   background-color: #1765CC;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #C5CCD3;"
        "}"
    );
    connect(m_okButton, &QPushButton::clicked, this, &PasswordDialog::onOkClicked);
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(m_okButton);
    mainLayout->addLayout(buttonLayout);
    
    setStyleSheet(
        "QDialog {"
        "   background-color: #FFFFFF;"
        "}"
    );
    
    m_passwordEdit->setFocus();
}

QString PasswordDialog::getPassword() const
{
    return m_passwordEdit->text();
}

void PasswordDialog::onOkClicked()
{
    if (m_mode == CreateMode) {
        if (m_passwordEdit->text() != m_confirmEdit->text()) {
            QMessageBox::warning(this, "Error", "Passwords do not match!");
            return;
        }
        
        if (m_passwordEdit->text().length() < 8) {
            QMessageBox::warning(this, "Error", "Password must be at least 8 characters long.");
            return;
        }
    }
    
    if (m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a password.");
        return;
    }
    
    accept();
}

void PasswordDialog::onTextChanged(const QString &text)
{
    Q_UNUSED(text);
    
    bool canProceed = !m_passwordEdit->text().isEmpty();
    
    if (m_mode == CreateMode) {
        canProceed = canProceed && !m_confirmEdit->text().isEmpty() &&
                     (m_passwordEdit->text() == m_confirmEdit->text()) &&
                     m_passwordEdit->text().length() >= 8;
        
        int strength = calculatePasswordStrength(m_passwordEdit->text());
        QString strengthText;
        QColor strengthColor;
        
        if (strength < 25) {
            strengthText = "Password Strength: Weak";
            strengthColor = QColor("#DC2626");
        } else if (strength < 50) {
            strengthText = "Password Strength: Fair";
            strengthColor = QColor("#F59E0B");
        } else if (strength < 75) {
            strengthText = "Password Strength: Good";
            strengthColor = QColor("#10B981");
        } else {
            strengthText = "Password Strength: Strong";
            strengthColor = QColor("#059669");
        }
        
        m_strengthLabel->setText(strengthText);
        m_strengthLabel->setStyleSheet(
            QString(
                "QLabel {"
                "   color: %1;"
                "   font-size: 12px;"
                "   background: transparent;"
                "}"
            ).arg(strengthColor.name())
        );
    }
    
    m_okButton->setEnabled(canProceed);
}

int PasswordDialog::calculatePasswordStrength(const QString &password) const
{
    int score = 0;
    
    if (password.length() >= 8) score += 20;
    if (password.length() >= 12) score += 20;
    if (password.length() >= 16) score += 10;
    
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (const QChar &c : password) {
        if (c.isLower()) hasLower = true;
        else if (c.isUpper()) hasUpper = true;
        else if (c.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasLower) score += 10;
    if (hasUpper) score += 15;
    if (hasDigit) score += 15;
    if (hasSpecial) score += 20;
    
    return qMin(score, 100);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_vaultEngine(new VaultEngine(this))
    , m_currentCategory("all")
    , m_isGridView(true)
{
    setupUI();
    applyStyleSheet();
    
    connect(m_vaultEngine, &VaultEngine::vaultUnlocked, this, &MainWindow::onVaultUnlocked);
    connect(m_vaultEngine, &VaultEngine::vaultLocked, this, &MainWindow::onVaultLocked);
    connect(m_vaultEngine, &VaultEngine::fileImported, this, &MainWindow::onFileImported);
    connect(m_vaultEngine, &VaultEngine::fileExported, this, &MainWindow::onFileExported);
    connect(m_vaultEngine, &VaultEngine::fileDeleted, this, &MainWindow::onFileDeleted);
    connect(m_vaultEngine, &VaultEngine::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_vaultEngine, &VaultEngine::progressChanged, this, &MainWindow::onProgressChanged);
    
    QString vaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/SphinxVault";
    
    if (!m_vaultEngine->initializeVault(vaultPath)) {
        m_statusBar->showMessage("Failed to initialize vault", 5000);
    }
    
    if (!m_vaultEngine->isVaultInitialized()) {
        QTimer::singleShot(100, this, &MainWindow::showSetupDialog);
    } else {
        QTimer::singleShot(100, this, &MainWindow::showUnlockDialog);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Sphinx - Secure Digital Vault");
    setMinimumSize(1100, 750);
    resize(1280, 850);
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    m_stackWidget = new QStackedWidget(this);
    m_mainLayout->addWidget(m_stackWidget);
    
    setupLockedScreen();
    setupMenuBar();
    setupToolBar();
    setupHeader();
    setupCategories();
    setupFilesArea();
    setupStatusBar();
    
    QWidget *unlockedContainer = new QWidget();
    QVBoxLayout *unlockedLayout = new QVBoxLayout(unlockedContainer);
    unlockedLayout->setContentsMargins(0, 0, 0, 0);
    unlockedLayout->setSpacing(0);
    unlockedLayout->addWidget(m_headerWidget);
    unlockedLayout->addWidget(m_categoriesWidget);
    unlockedLayout->addWidget(m_filesScrollArea, 1);
    
    m_stackWidget->addWidget(m_lockedScreen);
    m_stackWidget->addWidget(unlockedContainer);
    m_stackWidget->setCurrentWidget(m_lockedScreen);
}

void MainWindow::setupLockedScreen()
{
    m_lockedScreen = new QWidget();
    m_lockedLayout = new QVBoxLayout(m_lockedScreen);
    m_lockedLayout->setContentsMargins(48, 64, 48, 48);
    m_lockedLayout->setSpacing(24);
    m_lockedLayout->setAlignment(Qt::AlignCenter);
    
    m_lockedIcon = new QLabel("🦅", m_lockedScreen);
    m_lockedIcon->setAlignment(Qt::AlignCenter);
    m_lockedIcon->setStyleSheet(
        "QLabel {"
        "   font-size: 80px;"
        "   background: transparent;"
        "}"
    );
    m_lockedLayout->addWidget(m_lockedIcon, 0, Qt::AlignCenter);
    
    m_lockedTitle = new QLabel("Sphinx Vault", m_lockedScreen);
    m_lockedTitle->setAlignment(Qt::AlignCenter);
    m_lockedTitle->setStyleSheet(
        "QLabel {"
        "   color: #202124;"
        "   font-size: 32px;"
        "   font-weight: bold;"
        "   background: transparent;"
        "}"
    );
    m_lockedLayout->addWidget(m_lockedTitle, 0, Qt::AlignCenter);
    
    m_lockedSubtitle = new QLabel("Your secure digital vault is locked\nClick below to unlock", m_lockedScreen);
    m_lockedSubtitle->setAlignment(Qt::AlignCenter);
    m_lockedSubtitle->setStyleSheet(
        "QLabel {"
        "   color: #5F6368;"
        "   font-size: 16px;"
        "   background: transparent;"
        "}"
    );
    m_lockedLayout->addWidget(m_lockedSubtitle, 0, Qt::AlignCenter);
    
    m_lockedLayout->addSpacing(32);
    
    m_unlockButton = new QPushButton("🔓 Unlock Vault", m_lockedScreen);
    m_unlockButton->setFixedSize(200, 52);
    m_unlockButton->setCursor(Qt::PointingHandCursor);
    m_unlockButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1A73E8;"
        "   border: none;"
        "   border-radius: 26px;"
        "   color: white;"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1765CC;"
        "}"
    );
    connect(m_unlockButton, &QPushButton::clicked, this, &MainWindow::showUnlockDialog);
    m_lockedLayout->addWidget(m_unlockButton, 0, Qt::AlignCenter);
    
    m_lockedScreen->setStyleSheet(
        "QWidget#lockedScreen {"
        "   background-color: #F8F9FA;"
        "}"
    );
}

void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();
    m_menuBar->setStyleSheet(
        "QMenuBar {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #E8EAED;"
        "   padding: 4px 8px;"
        "   spacing: 8px;"
        "}"
        "QMenuBar::item:selected {"
        "   background-color: #E8F0FE;"
        "   border-radius: 4px;"
        "}"
        "QMenuBar::item {"
        "   padding: 6px 12px;"
        "   color: #5F6368;"
        "}"
    );
    
    m_fileMenu = m_menuBar->addMenu("&File");
    
    QAction *importAction = m_fileMenu->addAction("Import Files...");
    importAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportFilesClicked);
    
    QAction *lockAction = m_fileMenu->addAction("Lock Vault");
    lockAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(lockAction, &QAction::triggered, this, &MainWindow::onLockVaultClicked);
    
    m_fileMenu->addSeparator();
    
    QAction *exitAction = m_fileMenu->addAction("Exit");
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    m_viewMenu = m_menuBar->addMenu("&View");
    
    QAction *gridViewAction = m_viewMenu->addAction("Grid View");
    gridViewAction->setCheckable(true);
    gridViewAction->setChecked(true);
    connect(gridViewAction, &QAction::toggled, this, &MainWindow::onViewModeChanged);
    
    m_helpMenu = m_menuBar->addMenu("&Help");
    
    QAction *aboutAction = m_helpMenu->addAction("About Sphinx");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Sphinx",
            "<h2>Sphinx v1.0</h2>"
            "<p>Secure Digital Vault for Desktop</p>"
            "<p>AES-256 Encryption | Secure File Storage</p>"
            "<p><b>© 2024 Sphinx Security</b></p>");
    });
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar("Main Toolbar");
    m_toolBar->setMovable(false);
    m_toolBar->setStyleSheet(
        "QToolBar {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #E8EAED;"
        "   padding: 8px 16px;"
        "   spacing: 8px;"
        "}"
        "QToolButton {"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   color: #5F6368;"
        "}"
        "QToolButton:hover {"
        "   background-color: #E8F0FE;"
        "   color: #1A73E8;"
        "}"
    );
}

void MainWindow::setupHeader()
{
    m_headerWidget = new QWidget();
    m_headerWidget->setFixedHeight(80);
    m_headerWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #E8EAED;"
        "}"
    );
    
    m_headerLayout = new QHBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(32, 16, 32, 16);
    m_headerLayout->setSpacing(16);
    
    QLabel *logoLabel = new QLabel("🦅 Sphinx", m_headerWidget);
    logoLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "   color: #1A73E8;"
        "   background: transparent;"
        "}"
    );
    m_headerLayout->addWidget(logoLabel);
    
    QWidget *searchContainer = new QWidget();
    searchContainer->setFixedWidth(400);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    
    m_searchBar = new QLineEdit(searchContainer);
    m_searchBar->setPlaceholderText("🔍 Search in vault...");
    m_searchBar->setFixedHeight(42);
    m_searchBar->setStyleSheet(
        "QLineEdit {"
        "   background-color: #F1F3F4;"
        "   border: 2px solid transparent;"
        "   border-radius: 21px;"
        "   padding: 0 20px;"
        "   font-size: 14px;"
        "   color: #202124;"
        "}"
        "QLineEdit:focus {"
        "   background-color: #FFFFFF;"
        "   border: 2px solid #1A73E8;"
        "}"
    );
    connect(m_searchBar, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    searchLayout->addWidget(m_searchBar);
    
    m_headerLayout->addWidget(searchContainer, 1);
    
    m_importButton = new QPushButton("+ Import", m_headerWidget);
    m_importButton->setFixedSize(90, 38);
    m_importButton->setCursor(Qt::PointingHandCursor);
    m_importButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1A73E8;"
        "   border: none;"
        "   border-radius: 19px;"
        "   color: white;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1765CC;"
        "}"
    );
    connect(m_importButton, &QPushButton::clicked, this, &MainWindow::onImportFilesClicked);
    m_headerLayout->addWidget(m_importButton);
    
    m_lockButton = new QPushButton("🔒 Lock", m_headerWidget);
    m_lockButton->setFixedSize(80, 38);
    m_lockButton->setCursor(Qt::PointingHandCursor);
    m_lockButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFF3E0;"
        "   border: none;"
        "   border-radius: 19px;"
        "   color: #E65100;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #FFE0B2;"
        "}"
    );
    connect(m_lockButton, &QPushButton::clicked, this, &MainWindow::onLockVaultClicked);
    m_headerLayout->addWidget(m_lockButton);
    
    m_storageIndicator = new CircularProgress(m_headerWidget);
    m_headerLayout->addWidget(m_storageIndicator);
    
    m_storageLabel = new QLabel(m_headerWidget);
    m_storageLabel->setStyleSheet(
        "QLabel {"
        "   color: #5F6368;"
        "   font-size: 11px;"
        "   background: transparent;"
        "}"
    );
    m_headerLayout->addWidget(m_storageLabel);
}

void MainWindow::setupCategories()
{
    m_categoriesWidget = new QWidget();
    m_categoriesWidget->setFixedHeight(130);
    m_categoriesWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #F8F9FA;"
        "   border-bottom: 1px solid #E8EAED;"
        "}"
    );
    
    m_categoriesLayout = new QHBoxLayout(m_categoriesWidget);
    m_categoriesLayout->setContentsMargins(32, 16, 32, 16);
    m_categoriesLayout->setSpacing(16);
    m_categoriesLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    QStringList categories = {"all", "images", "documents", "videos", "archives", "audio"};
    QMap<QString, QString> icons = {
        {"all", "📁"}, {"images", "🖼"}, {"documents", "📄"},
        {"videos", "🎬"}, {"archives", "📦"}, {"audio", "🎵"}
    };
    
    for (const QString &cat : categories) {
        CategoryCard *card = new CategoryCard(cat, icons[cat], 0, getCategoryColor(cat), m_categoriesWidget);
        connect(card, &CategoryCard::categoryClicked, this, &MainWindow::onCategoryClicked);
        m_categoriesLayout->addWidget(card);
        m_categoryCards[cat] = card;
    }
    
    m_categoriesLayout->addStretch();
}

void MainWindow::setupFilesArea()
{
    m_filesScrollArea = new QScrollArea();
    m_filesScrollArea->setWidgetResizable(true);
    m_filesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_filesScrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: #F8F9FA;"
        "   border: none;"
        "}"
        "QScrollBar:vertical {"
        "   width: 10px;"
        "   background: transparent;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #DADCE0;"
        "   border-radius: 5px;"
        "   min-height: 40px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #BDC1C6;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );
    
    m_filesContainer = new QWidget();
    m_filesGrid = new QGridLayout(m_filesContainer);
    m_filesGrid->setContentsMargins(32, 24, 32, 24);
    m_filesGrid->setHorizontalSpacing(16);
    m_filesGrid->setVerticalSpacing(16);
    m_filesGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    m_filesContainer->setStyleSheet("background-color: #F8F9FA;");
    m_filesScrollArea->setWidget(m_filesContainer);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->setStyleSheet(
        "QStatusBar {"
        "   background-color: #FFFFFF;"
        "   border-top: 1px solid #E8EAED;"
        "   color: #5F6368;"
        "   font-size: 12px;"
        "   padding: 4px 16px;"
        "}"
    );
    m_statusBar->showMessage("Ready");
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(
        "QMainWindow {"
        "   background-color: #F8F9FA;"
        "}"
        "QToolTip {"
        "   background-color: #323232;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   padding: 6px 10px;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
    );
}

void MainWindow::showSetupDialog()
{
    PasswordDialog dialog(PasswordDialog::CreateMode, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString password = dialog.getPassword();
        
        if (m_vaultEngine->setMasterPassword(password)) {
            m_statusBar->showMessage("Vault created successfully!", 5000);
            onVaultUnlocked();
        } else {
            QMessageBox::critical(this, "Error", "Failed to create vault. Please try again.");
            close();
        }
    } else {
        close();
    }
}

void MainWindow::showUnlockDialog()
{
    PasswordDialog dialog(PasswordDialog::VerifyMode, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString password = dialog.getPassword();
        
        if (m_vaultEngine->unlockVault(password)) {
            m_statusBar->showMessage("Vault unlocked successfully!", 3000);
        } else {
            QMessageBox::warning(this, "Access Denied", "Invalid password. Please try again.");
            showUnlockDialog();
        }
    } else {
        close();
    }
}

void MainWindow::onVaultUnlocked()
{
    m_stackWidget->setCurrentIndex(1);
    refreshCategories();
    refreshFileList();
    updateStorageIndicator();
    m_statusBar->showMessage("Vault is now open", 3000);
}

void MainWindow::onVaultLocked()
{
    m_stackWidget->setCurrentIndex(0);
    clearFileCards();
    m_statusBar->showMessage("Vault is now locked", 3000);
}

void MainWindow::onFileImported(const QString &fileName)
{
    m_statusBar->showMessage(QString("Imported: %1").arg(fileName), 5000);
    refreshFileList();
    refreshCategories();
    updateStorageIndicator();
}

void MainWindow::onFileExported(const QString &fileName)
{
    m_statusBar->showMessage(QString("Exported: %1").arg(fileName), 5000);
}

void MainWindow::onFileDeleted(const QString &fileName)
{
    m_statusBar->showMessage(QString("Deleted: %1").arg(fileName), 5000);
    refreshFileList();
    refreshCategories();
    updateStorageIndicator();
}

void MainWindow::onErrorOccurred(const QString &errorMessage)
{
    m_statusBar->showMessage(errorMessage, 8000);
    QMessageBox::warning(this, "Error", errorMessage);
}

void MainWindow::onProgressChanged(int percent)
{
    Q_UNUSED(percent);
}

void MainWindow::onImportFilesClicked()
{
    if (m_vaultEngine->isLocked()) {
        QMessageBox::information(this, "Locked", "Please unlock the vault first.");
        return;
    }
    
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "Select Files to Import",
        QString(),
        "All Files (*);;Images (*.png *.jpg *.jpeg *.gif *.bmp);;Documents (*.pdf *.doc *.docx *.txt);;Videos (*.mp4 *.avi *.mkv)"
    );
    
    if (filePaths.isEmpty()) {
        return;
    }
    
    int successCount = 0;
    for (const QString &path : filePaths) {
        if (m_vaultEngine->importFile(path)) {
            successCount++;
        }
    }
    
    if (successCount > 0) {
        QMessageBox::information(this, "Success",
            QString("Successfully imported %1 of %2 file(s).").arg(successCount).arg(filePaths.size()));
    }
}

void MainWindow::onExportFileClicked(const QString &encryptedName)
{
    if (m_vaultEngine->isLocked()) return;
    
    QString destinationDir = QFileDialog::getExistingDirectory(
        this,
        "Select Export Location",
        QStandardPaths::writableLocation(QStandardPaths::DownloadsLocation)
    );
    
    if (destinationDir.isEmpty()) return;
    
    if (m_vaultEngine->exportFile(encryptedName, destinationDir)) {
        QMessageBox::information(this, "Success", "File exported successfully!");
    }
}

void MainWindow::onDeleteFileClicked(const QString &encryptedName)
{
    if (m_vaultEngine->isLocked()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Delete",
        "Are you sure you want to securely delete this file?\n\nThis action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_vaultEngine->deleteFileSecurely(encryptedName)) {
            QMessageBox::information(this, "Deleted", "File has been securely deleted.");
        }
    }
}

void MainWindow::onFileCardClicked(const QString &encryptedName)
{
    m_selectedFile = encryptedName;
    
    for (auto it = m_fileCards.begin(); it != m_fileCards.end(); ++it) {
        it.value()->setHighlighted(it.key() == encryptedName);
    }
}

void MainWindow::onCategoryClicked(const QString &category)
{
    m_currentCategory = category;
    filterFiles(m_searchBar->text());
    
    for (auto it = m_categoryCards.begin(); it != m_categoryCards.end(); ++it) {
        CategoryCard *card = it.value();
        if (card) {
            card->setStyleSheet(QString(
                "QFrame {"
                "   background-color: %1;"
                "   border-radius: 16px;"
                "   border: 2px solid %2;"
                "}"
            ).arg(
                it.key() == category ? getCategoryColor(category).lighter(110).name() : getCategoryColor(it.key()).lighter(120).name(),
                it.key() == category ? "#1A73E8" : "transparent"
            ));
        }
    }
}

void MainWindow::onLockVaultClicked()
{
    if (!m_vaultEngine->isLocked()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Lock Vault",
            "Are you sure you want to lock the vault?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            m_vaultEngine->lockVault();
        }
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    filterFiles(text);
}

void MainWindow::onViewModeChanged(bool isGridView)
{
    m_isGridView = isGridView;
    refreshFileList();
}

void MainWindow::refreshFileList()
{
    clearFileList();
    
    QJsonArray filesInfo = m_vaultEngine->getAllFilesInfo();
    
    for (const QJsonValue &value : filesInfo) {
        QJsonObject fileInfo = value.toObject();
        addFileCard(fileInfo);
    }
    
    m_statusBar->showMessage(QString("Showing %1 file(s)").arg(filesInfo.size()), 3000);
}

void MainWindow::clearFileList()
{
    while (QLayoutItem *item = m_filesGrid->takeAt(0)) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_fileCards.clear();
}

void MainWindow::filterFiles(const QString &searchText)
{
    clearFileList();
    
    QJsonArray allFiles = m_vaultEngine->getAllFilesInfo();
    
    for (const QJsonValue &value : allFiles) {
        QJsonObject fileInfo = value.toObject();
        
        QString name = fileInfo["originalName"].toString().toLower();
        QString category = fileInfo["category"].toString();
        
        bool matchesCategory = (m_currentCategory == "all" || m_currentCategory == category);
        bool matchesSearch = searchText.isEmpty() || name.contains(searchText.toLower());
        
        if (matchesCategory && matchesSearch) {
            addFileCard(fileInfo);
        }
    }
}

void MainWindow::addFileCard(const QJsonObject &fileInfo)
{
    QString encryptedName = fileInfo["encryptedName"].toString();
    QString displayName = fileInfo["originalName"].toString();
    QString category = fileInfo["category"].toString();
    qint64 fileSize = fileInfo["originalSize"].toVariant().toLongLong();
    QString fileType = fileInfo["fileType"].toString();
    
    FileCard *card = new FileCard(encryptedName, displayName, category, fileSize, fileType, m_filesContainer);
    
    connect(card, &FileCard::cardClicked, this, &MainWindow::onFileCardClicked);
    connect(card, &FileCard::cardDoubleClicked, this, [this, encryptedName]() {
        onExportFileClicked(encryptedName);
    });
    connect(card, &FileCard::deleteRequested, this, &MainWindow::onDeleteFileClicked);
    connect(card, &FileCard::exportRequested, this, &MainWindow::onExportFileClicked);
    
    int row = m_fileCards.size() / 6;
    int col = m_fileCards.size() % 6;
    
    m_filesGrid->addWidget(card, row, col, Qt::AlignLeft | Qt::AlignTop);
    m_fileCards[encryptedName] = card;
}

void MainWindow::clearFileCards()
{
    clearFileList();
}

void MainWindow::refreshCategories()
{
    QMap<QString, int> counts;
    counts["all"] = m_vaultEngine->getFileCount();
    
    QStringList categories = {"images", "documents", "videos", "archives", "audio"};
    
    for (const QString &cat : categories) {
        counts[cat] = m_vaultEngine->listFilesByCategory(cat).size();
    }
    
    for (auto it = m_categoryCards.begin(); it != m_categoryCards.end(); ++it) {
        CategoryCard *card = it.value();
        if (card && counts.contains(it.key())) {
            card->m_countLabel->setText(QString::number(counts[it.key()]));
        }
    }
}

void MainWindow::updateStorageIndicator()
{
    qint64 currentSize = m_vaultEngine->getTotalEncryptedSize();
    qint64 maxSize = 1073741824LL;
    
    m_storageIndicator->setCurrentSizeBytes(currentSize);
    m_storageIndicator->setMaximumSizeBytes(maxSize);
    m_storageIndicator->setValue(static_cast<int>((currentSize * 100) / maxSize));
    
    m_storageLabel->setText(formatFileSize(currentSize) + " used");
}

QString MainWindow::formatFileSize(qint64 bytes) const
{
    qint64 gb = 1024 * 1024 * 1024;
    qint64 mb = 1024 * 1024;
    qint64 kb = 1024;
    
    if (bytes >= gb) {
        return QString("%1 GB").arg(static_cast<double>(bytes) / gb, 0, 'f', 2);
    } else if (bytes >= mb) {
        return QString("%1 MB").arg(static_cast<double>(bytes) / mb, 0, 'f', 1);
    } else if (bytes >= kb) {
        return QString("%1 KB").arg(bytes / kb);
    }
    return QString("%1 B").arg(bytes);
}

QColor MainWindow::getCategoryColor(const QString &category) const
{
    if (category == "all") return QColor("#E8F0FE");
    if (category == "images") return QColor("#FEF7E0");
    if (category == "documents") return QColor("#E6F4EA");
    if (category == "videos") return QColor("#FCE8E6");
    if (category == "archives") return QColor("#F3E8FD");
    if (category == "audio") return QColor("#E8DFD8");
    if (category == "other") return QColor("#F1F3F4");
    return QColor("#E8EAED");
}

QString MainWindow::getCategoryIcon(const QString &category) const
{
    if (category == "all") return "📁";
    if (category == "images") return "🖼";
    if (category == "documents") return "📄";
    if (category == "videos") return "🎬";
    if (category == "archives") return "📦";
    if (category == "audio") return "🎵";
    return "📌";
}
