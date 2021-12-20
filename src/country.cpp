#include "country.h"

#include "channelsmodel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>
#include <QSqlQuery>

Country::Country(CountryData data)
    : QObject(nullptr)
    , m_data(data)
{
    connect(&Fetcher::instance(), &Fetcher::startedFetchingCountry, this, [this](const CountryId &id) {
        if (id == m_data.m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::countryUpdated, this, [this](const CountryId &id) {
        if (id == m_data.m_id) {
            setRefreshing(false);
            Q_EMIT channelCountChanged();
            m_error.reset();
        }
    });
    connect(&Fetcher::instance(), &Fetcher::errorFetchingCountry, this, [this](const CountryId &id, const Error &error) {
        if (id == m_data.m_id) {
            setError(error);
            setRefreshing(false);
        }
    });

    m_channels = new ChannelsModel(this);
}

Country::~Country()
{
}

QString Country::id() const
{
    return m_data.m_id.value();
}

QString Country::name() const
{
    return m_data.m_name;
}

QString Country::url() const
{
    return m_data.m_url;
}

int Country::channelCount() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM CountryChannels where country=:country;"));
    query.bindValue(QStringLiteral(":country"), m_data.m_id.value());
    Database::instance().execute(query);
    if (!query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

bool Country::refreshing() const
{
    return m_refreshing;
}

int Country::errorId() const
{
    return m_error.m_id;
}

QString Country::errorString() const
{
    return m_error.m_message;
}

void Country::setName(const QString &name)
{
    m_data.m_name = name;
    Q_EMIT nameChanged(m_data.m_name);
}

void Country::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Country::setError(const Error &error)
{
    m_error = error;
    Q_EMIT errorIdChanged(m_error.m_id);
    Q_EMIT errorStringChanged(m_error.m_message);
}
