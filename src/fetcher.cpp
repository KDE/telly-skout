#include "fetcher.h"

#include "database.h"
#include "tvspielfilmfetcher.h"
#include "xmltvsefetcher.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>

#define USE_TVSPIELFILM

Fetcher::Fetcher()
    :
#ifdef USE_TVSPIELFILM
    m_fetcherImpl(new TvSpielfilmFetcher)
#else
    m_fetcherImpl(new XmlTvSeFetcher)
#endif
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_manager->setStrictTransportSecurityEnabled(true);
    m_manager->enableStrictTransportSecurityStore(true);

    connect(m_fetcherImpl.get(), &FetcherImpl::startedFetchingFavorites, this, [this]() {
        Q_EMIT startedFetchingFavorites();
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::finishedFetchingFavorites, this, [this]() {
        Q_EMIT finishedFetchingFavorites();
    });

    connect(m_fetcherImpl.get(), &FetcherImpl::startedFetchingCountry, this, [this](const CountryId &id) {
        Q_EMIT startedFetchingCountry(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::countryUpdated, this, [this](const CountryId &id) {
        Q_EMIT countryUpdated(id);
    });

    connect(m_fetcherImpl.get(), &FetcherImpl::startedFetchingChannel, this, [this](const ChannelId &id) {
        Q_EMIT startedFetchingChannel(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::channelUpdated, this, [this](const ChannelId &id) {
        Q_EMIT channelUpdated(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::channelDetailsUpdated, this, [this](const ChannelId &id, const QString &image) {
        Q_EMIT channelDetailsUpdated(id, image);
    });

    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetching, this, [this](const Error &error) {
        Q_EMIT errorFetching(error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingCountry, this, [this](const CountryId &id, const Error &error) {
        Q_EMIT errorFetchingCountry(id, error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingChannel, this, [this](const ChannelId &id, const Error &error) {
        Q_EMIT errorFetchingChannel(id, error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingProgram, this, [this](const ProgramId &id, const Error &error) {
        Q_EMIT errorFetchingProgram(id, error);
    });
}

void Fetcher::fetchFavorites()
{
    qDebug() << "Starting to fetch favorites";

    Q_EMIT startedFetchingFavorites();

    const QVector<ChannelId> favoriteChannels = Database::instance().favorites();
    for (int i = 0; i < favoriteChannels.length(); i++) {
        m_fetcherImpl->fetchProgram(favoriteChannels.at(i));
    }

    Q_EMIT finishedFetchingFavorites();
}

void Fetcher::fetchCountries()
{
    m_fetcherImpl->fetchCountries();
}

void Fetcher::fetchCountry(const QString &url, const QString &countryId)
{
    fetchCountry(url, CountryId(countryId));
}

void Fetcher::fetchCountry(const QString &url, const CountryId &countryId)
{
    m_fetcherImpl->fetchCountry(url, countryId);
}

void Fetcher::fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url)
{
    m_fetcherImpl->fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url);
}

QString Fetcher::image(const QString &url)
{
    QString path = filePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    download(url);

    return QLatin1String("");
}

void Fetcher::download(const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QFile file(filePath(url));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
        }
        Q_EMIT imageDownloadFinished(url);

        delete reply;
    });
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << "Remove image: " << filePath(url);
    QFile(filePath(url)).remove();
}

QString Fetcher::filePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/")
        + QString::fromStdString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex().toStdString());
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    return m_manager->get(request);
}
