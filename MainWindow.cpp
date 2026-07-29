#include "VaultEngine.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QMessageBox>

VaultEngine::VaultEngine(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)
    , m_isLocked(true)
{
}

VaultEngine::~VaultEngine()
{
    if (!m_isLocked) {
        m_encryptionKey.fill(0);
        m_masterKeyHash.fill(0);
    }
}

bool VaultEngine::initializeVault(const QString &vaultPath)
{
    if (vaultPath.isEmpty()) {
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        m_vaultPath = QDir(defaultPath).absoluteFilePath("SphinxVault");
    } else {
        m_vaultPath = vaultPath;
    }
    
    m_configPath = QDir(m_vaultPath).absoluteFilePath("config.json");
    m_filesPath = QDir(m_vaultPath).absoluteFilePath("files");
    
    QDir dir(m_vaultPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit errorOccurred(tr("Failed to create vault directory"));
            return false;
        }
    }
    
    QDir filesDir(m_filesPath);
    if (!filesDir.exists()) {
        if (!filesDir.mkpath(".")) {
            emit errorOccurred(tr("Failed to create files directory"));
            return false;
        }
    }
    
    m_isInitialized = loadConfig();
    
    if (m_isInitialized && !m_masterKeyHash.isEmpty()) {
        m_isLocked = true;
    }
    
    return true;
}

bool VaultEngine::isVaultInitialized() const
{
    return m_isInitialized && !m_masterKeyHash.isEmpty();
}

bool VaultEngine::setMasterPassword(const QString &password)
{
    if (password.length() < 8) {
        emit errorOccurred(tr("Password must be at least 8 characters long"));
        return false;
    }
    
    QByteArray salt = generateRandomBytes(32);
    m_masterKeyHash = deriveKey(password, salt);
    m_encryptionKey = deriveKey(password + "_enc", salt).left(AES_KEY_SIZE);
    
    QJsonObject config;
    config["salt"] = QString(salt.toHex());
    config["keyHash"] = QString(m_masterKeyHash.toHex());
    config["createdDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    config["version"] = "1.0";
    
    QJsonDocument doc(config);
    QFile configFile(m_configPath);
    
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(doc.toJson());
        configFile.close();
        
        QByteArray configData = doc.toJson();
        QFile secureConfig(m_configPath + ".enc");
        if (secureConfig.open(QIODevice::WriteOnly)) {
            secureConfig.write(encryptData(configData));
            secureConfig.close();
        }
        
        m_isInitialized = true;
        m_isLocked = false;
        emit vaultUnlocked();
        return true;
    }
    
    emit errorOccurred(tr("Failed to save configuration"));
    return false;
}

bool VaultEngine::verifyMasterPassword(const QString &password) const
{
    if (m_masterKeyHash.isEmpty()) {
        return false;
    }
    
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray configData = configFile.readAll();
    configFile.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(configData);
    QJsonObject config = doc.object();
    
    QByteArray storedSalt = QByteArray::fromHex(config["salt"].toString().toUtf8());
    QByteArray testHash = deriveKey(password, storedSalt);
    
    return (testHash == m_masterKeyHash);
}

QByteArray VaultEngine::deriveKey(const QString &password, const QByteArray &salt) const
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = passwordBytes + salt;
    
    for (int i = 0; i < PBKDF2_ITERATIONS; ++i) {
        hash = QCryptographicHash::hash(hash, QCryptographicHash::Sha512);
    }
    
    return hash;
}

QByteArray VaultEngine::encryptData(const QByteArray &data) const
{
    if (m_encryptionKey.isEmpty() || data.isEmpty()) {
        return data;
    }
    
    QByteArray result;
    result.resize(data.size());
    
    int keyLength = m_encryptionKey.size();
    
    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ m_encryptionKey[i % keyLength] ^ (i & 0xFF);
        
        if (i % AES_BLOCK_SIZE == 0 && i > 0) {
            byte rotate = (i / AES_BLOCK_SIZE) & 0x07;
            for (int j = i - AES_BLOCK_SIZE; j < i; ++j) {
                result[j] = ((result[j] << rotate) | (result[j] >> (8 - rotate))) & 0xFF;
            }
        }
    }
    
    QByteArray finalHash = QCryptographicHash::hash(result, QCryptographicHash::Sha256);
    result.append(finalHash);
    
    return result.toBase64().toUtf8();
}

QByteArray VaultEngine::decryptData(const QByteArray &encryptedData) const
{
    if (m_encryptionKey.isEmpty() || encryptedData.isEmpty()) {
        return encryptedData;
    }
    
    QByteArray decoded = QByteArray::fromBase64(encryptedData);
    
    int hashSize = QCryptographicHash::hashLength(QCryptographicHash::Sha256);
    if (decoded.size() <= hashSize) {
        return QByteArray();
    }
    
    QByteArray storedHash = decoded.right(hashSize);
    QByteArray data = decoded.left(decoded.size() - hashSize);
    
    QByteArray testHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    if (testHash != storedHash) {
        return QByteArray();
    }
    
    QByteArray result;
    result.resize(data.size());
    
    int keyLength = m_encryptionKey.size();
    
    for (int i = 0; i < data.size(); ++i) {
        byte val = data[i];
        
        if (i % AES_BLOCK_SIZE == 0 && i > 0) {
            byte rotate = (i / AES_BLOCK_SIZE) & 0x07;
            byte invRotate = 8 - rotate;
            for (int j = i - AES_BLOCK_SIZE; j < i; ++j) {
                val = ((val << invRotate) | (val >> rotate)) & 0xFF;
                if (j == i - 1) {
                    data[j] = val;
                }
            }
        }
        
        result[i] = val ^ m_encryptionKey[i % keyLength] ^ (i & 0xFF);
    }
    
    return result;
}

QString VaultEngine::generateEncryptedName(const QString &originalName) const
{
    QByteArray nameData = originalName.toUtf8();
    QByteArray hash = QCryptographicHash::hash(
        nameData + QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8() + 
        generateRandomBytes(8),
        QCryptographicHash::Sha256
    );
    
    QString ext = ".svf";
    return hash.toHex().left(32) + ext;
}

QString VaultEngine::getFileCategory(const QString &fileName) const
{
    QString suffix = QFileInfo(fileName).suffix().toLower();
    
    QStringList imageExts = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "svg", "ico", "tiff", "heic"};
    QStringList documentExts = {"pdf", "doc", "docx", "txt", "rtf", "odt", "xls", "xlsx", "ppt", "pptx"};
    QStringList videoExts = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v"};
    QStringList archiveExts = {"zip", "rar", "7z", "tar", "gz", "bz2", "xz"};
    QStringList audioExts = {"mp3", "wav", "flac", "aac", "ogg", "wma", "m4a"};
    
    if (imageExts.contains(suffix)) return "images";
    if (documentExts.contains(suffix)) return "documents";
    if (videoExts.contains(suffix)) return "videos";
    if (archiveExts.contains(suffix)) return "archives";
    if (audioExts.contains(suffix)) return "audio";
    
    return "other";
}

QByteArray VaultEngine::generateRandomBytes(int length) const
{
    QByteArray bytes;
    bytes.resize(length);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    
    for (int i = 0; i < length; ++i) {
        bytes[i] = static_cast<char>(dist(gen));
    }
    
    return bytes;
}

bool VaultEngine::importFile(const QString &filePath, const QString &category)
{
    if (m_isLocked) {
        emit errorOccurred(tr("Vault is locked. Please unlock first."));
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        emit errorOccurred(tr("File does not exist: %1").arg(filePath));
        return false;
    }
    
    if (fileInfo.size() == 0) {
        emit errorOccurred(tr("Cannot import empty file"));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open file for reading: %1").arg(filePath));
        return false;
    }
    
    QByteArray fileContent = file.readAll();
    file.close();
    
    QString encryptedName = generateEncryptedName(fileInfo.fileName());
    QString fileCategory = category.isEmpty() ? getFileCategory(fileInfo.fileName()) : category;
    
    QByteArray encryptedContent = encryptData(fileContent);
    
    QString destinationPath = QDir(m_filesPath).absoluteFilePath(encryptedName);
    QFile destFile(destinationPath);
    
    if (!destFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("Cannot write to vault: %1").arg(destinationPath));
        return false;
    }
    
    destFile.write(encryptedContent);
    destFile.close();
    
    QJsonObject metadata = createFileMetadata(fileInfo, encryptedName, fileCategory);
    metadata["contentHash"] = QString(QCryptographicHash::hash(fileContent, QCryptographicHash::Sha256).toHex());
    
    QString metaPath = QDir(m_filesPath).absoluteFilePath(encryptedName + ".meta");
    QFile metaFile(metaPath);
    if (metaFile.open(QIODevice::WriteOnly)) {
        QJsonDocument metaDoc(metadata);
        metaFile.write(encryptData(metaDoc.toJson()));
        metaFile.close();
    }
    
    emit progressChanged(100);
    emit fileImported(fileInfo.fileName());
    
    return true;
}

bool VaultEngine::exportFile(const QString &encryptedName, const QString &destinationPath)
{
    if (m_isLocked) {
        emit errorOccurred(tr("Vault is locked. Please unlock first."));
        return false;
    }
    
    QString sourcePath = QDir(m_filesPath).absoluteFilePath(encryptedName);
    QFileInfo sourceInfo(sourcePath);
    
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        emit errorOccurred(tr("File not found in vault: %1").arg(encryptedName));
        return false;
    }
    
    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot read vault file: %1").arg(sourcePath));
        return false;
    }
    
    QByteArray encryptedContent = sourceFile.readAll();
    sourceFile.close();
    
    QByteArray decryptedContent = decryptData(encryptedContent);
    
    if (decryptedContent.isEmpty()) {
        emit errorOccurred(tr("Decryption failed. File may be corrupted."));
        return false;
    }
    
    QString metaPath = sourcePath + ".meta";
    QJsonObject metadata;
    QFile metaFile(metaPath);
    if (metaFile.open(QIODevice::ReadOnly)) {
        QByteArray encryptedMeta = metaFile.readAll();
        metaFile.close();
        QByteArray metaJson = decryptData(encryptedMeta);
        QJsonDocument metaDoc = QJsonDocument::fromJson(metaJson);
        metadata = metaDoc.object();
    }
    
    QString originalName = metadata.contains("originalName") ? 
                           metadata["originalName"].toString() : 
                           QFileInfo(encryptedName).baseName() + ".decrypted";
    
    QString finalDestPath;
    if (destinationPath.endsWith('/') || destinationPath.endsWith('\\')) {
        finalDestPath = QDir(destinationPath).absoluteFilePath(originalName);
    } else if (QFileInfo(destinationPath).isDir()) {
        finalDestPath = QDir(destinationPath).absoluteFilePath(originalName);
    } else {
        finalDestPath = destinationPath;
    }
    
    QFile destFile(finalDestPath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("Cannot write to destination: %1").arg(finalDestPath));
        return false;
    }
    
    destFile.write(decryptedContent);
    destFile.close();
    
    emit progressChanged(100);
    emit fileExported(originalName);
    
    return true;
}

bool VaultEngine::deleteFileSecurely(const QString &encryptedName)
{
    if (m_isLocked) {
        emit errorOccurred(tr("Vault is locked. Please unlock first."));
        return false;
    }
    
    QString filePath = QDir(m_filesPath).absoluteFilePath(encryptedName);
    QString metaPath = filePath + ".meta";
    
    if (!secureDelete(metaPath)) {
        qDebug() << "Warning: Could not securely delete metadata file";
    }
    
    if (secureDelete(filePath)) {
        emit fileDeleted(encryptedName);
        return true;
    }
    
    emit errorOccurred(tr("Failed to delete file from vault"));
    return false;
}

bool VaultEngine::secureDelete(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.exists()) {
        return true;
    }
    
    if (!file.open(QIODevice::ReadWrite)) {
        return file.remove();
    }
    
    qint64 fileSize = file.size();
    QByteArray overwritePattern;
    overwritePattern.fill(0x00, qMin(static_cast<qint64>(4096), fileSize));
    
    for (qint64 pos = 0; pos < fileSize; pos += overwritePattern.size()) {
        file.seek(pos);
        qint64 writeSize = qMin(overwritePattern.size(), fileSize - pos);
        file.write(overwritePattern.left(writeSize));
        file.flush();
    }
    
    overwritePattern.fill(0xFF);
    file.seek(0);
    for (qint64 pos = 0; pos < fileSize; pos += overwritePattern.size()) {
        file.seek(pos);
        qint64 writeSize = qMin(overwritePattern.size(), fileSize - pos);
        file.write(overwritePattern.left(writeSize));
        file.flush();
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    
    file.seek(0);
    for (qint64 pos = 0; pos < fileSize; ++pos) {
        overwritePattern[pos % overwritePattern.size()] = static_cast<char>(dist(gen));
        if ((pos + 1) % overwritePattern.size() == 0 || pos == fileSize - 1) {
            file.seek(pos - (overwritePattern.size() - 1));
            file.write(overwritePattern);
            file.flush();
        }
    }
    
    file.close();
    
#ifdef _WIN32
    std::wstring widePath = filePath.toStdWString();
    DeleteFile(widePath.c_str());
#endif
    
    return file.remove();
}

QStringList VaultEngine::listFiles() const
{
    QDir filesDir(m_filesPath);
    QStringList filters;
    filters << "*.svf";
    return filesDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
}

QStringList VaultEngine::listFilesByCategory(const QString &category) const
{
    QStringList allFiles = listFiles();
    QStringList filtered;
    
    for (const QString &file : allFiles) {
        QJsonObject info = getFileInfo(file);
        if (info["category"].toString() == category) {
            filtered.append(file);
        }
    }
    
    return filtered;
}

QJsonObject VaultEngine::getFileInfo(const QString &encryptedName) const
{
    QString filePath = QDir(m_filesPath).absoluteFilePath(encryptedName);
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists()) {
        return QJsonObject();
    }
    
    QString metaPath = filePath + ".meta";
    QFile metaFile(metaPath);
    
    if (metaFile.open(QIODevice::ReadOnly)) {
        QByteArray encryptedMeta = metaFile.readAll();
        metaFile.close();
        
        if (!m_isLocked && !m_encryptionKey.isEmpty()) {
            QByteArray metaJson = decryptData(encryptedMeta);
            QJsonDocument metaDoc = QJsonDocument::fromJson(metaJson);
            return metaDoc.object();
        } else {
            return QJsonObject();
        }
    }
    
    QJsonObject basicInfo;
    basicInfo["encryptedName"] = encryptedName;
    basicInfo["size"] = fileInfo.size();
    basicInfo["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
    
    return basicInfo;
}

QJsonArray VaultEngine::getAllFilesInfo() const
{
    QJsonArray filesArray;
    QStringList files = listFiles();
    
    for (const QString &file : files) {
        QJsonObject info = getFileInfo(file);
        if (!info.isEmpty()) {
            filesArray.append(info);
        }
    }
    
    return filesArray;
}

qint64 VaultEngine::getVaultSize() const
{
    QDir filesDir(m_filesPath);
    qint64 totalSize = 0;
    
    QFileInfoList fileList = filesDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : fileList) {
        totalSize += info.size();
    }
    
    return totalSize;
}

qint64 VaultEngine::getTotalEncryptedSize() const
{
    return getVaultSize();
}

int VaultEngine::getFileCount() const
{
    return listFiles().count();
}

bool VaultEngine::lockVault()
{
    if (m_isLocked) {
        return true;
    }
    
    m_encryptionKey.fill(0);
    m_isLocked = true;
    
    emit vaultLocked();
    return true;
}

bool VaultEngine::unlockVault(const QString &password)
{
    if (!m_isInitialized) {
        emit errorOccurred(tr("Vault not initialized"));
        return false;
    }
    
    if (verifyMasterPassword(password)) {
        QFile configFile(m_configPath);
        if (configFile.open(QIODevice::ReadOnly)) {
            QByteArray configData = configFile.readAll();
            configFile.close();
            
            QJsonDocument doc = QJsonDocument::fromJson(configData);
            QJsonObject config = doc.object();
            
            QByteArray salt = QByteArray::fromHex(config["salt"].toString().toUtf8());
            m_encryptionKey = deriveKey(password + "_enc", salt).left(AES_KEY_SIZE);
            
            m_isLocked = false;
            emit vaultUnlocked();
            return true;
        }
    }
    
    emit errorOccurred(tr("Invalid password"));
    return false;
}

bool VaultEngine::isLocked() const
{
    return m_isLocked;
}

bool VaultEngine::saveConfig() const
{
    QJsonObject config;
    config["version"] = "1.0";
    config["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(config);
    QFile configFile(m_configPath);
    
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(doc.toJson());
        configFile.close();
        return true;
    }
    
    return false;
}

bool VaultEngine::loadConfig()
{
    QFile configFile(m_configPath);
    
    if (!configFile.exists()) {
        return false;
    }
    
    if (!configFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray configData = configFile.readAll();
    configFile.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(configData);
    QJsonObject config = doc.object();
    
    if (config.contains("keyHash") && config.contains("salt")) {
        m_masterKeyHash = QByteArray::fromHex(config["keyHash"].toString().toUtf8());
        return true;
    }
    
    return false;
}

QJsonObject VaultEngine::createFileMetadata(const QFileInfo &fileInfo, const QString &encryptedName, const QString &category) const
{
    QJsonObject metadata;
    metadata["originalName"] = fileInfo.fileName();
    metadata["encryptedName"] = encryptedName;
    metadata["category"] = category;
    metadata["originalSize"] = fileInfo.size();
    metadata["fileType"] = fileInfo.suffix().toLower();
    metadata["addedDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["pathInVault"] = encryptedName;
    
    return metadata;
}
