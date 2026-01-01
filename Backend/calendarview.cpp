#include "calendarview.h"
#include <QDebug>

using namespace project;

CalendarView::CalendarView(DbmPtr db, QObject *parent)
    : QObject(parent),db_(db),
    m_date(QDate::currentDate()),
    m_calendar(QCalendar::System::Gregorian)
{
    // Set to first day of current month
    m_date.setDate(m_date.year(), m_date.month(), 1);

}

int CalendarView::year() const
{
    return m_date.year();
}

int CalendarView::month() const
{
    return m_date.month();
}

void CalendarView::setYear(int y)
{
    if (y == m_date.year())
        return;

    qDebug() << "Setting year to:" << y;
    m_date.setDate(y, m_date.month(), 1);
    emit dateChanged();
}

void CalendarView::setMonth(int m)
{
    // if (m == m_date.month())
    //     return;

    qDebug() << "Setting month to:" << m;
    m_date.setDate(m_date.year(), m, 1);

    QDate end_date;
    end_date.setDate(m_date.year(), m, daysInMonth());

    qInfo() << m_date.toString(Qt::ISODate) << end_date.toString(Qt::ISODate);
    QString sqlCmd = QString("SELECT id, name FROM projects");
    auto results = db_->queryRow(sqlCmd);
    QHash<int, QString> projectMap;
    for(int i = 0; i < results.size(); i+=2)
    {
        int j = i + 1;
        int id_ = results[i].toInt();
        projectMap[id_] = results[j];
    }



    sqlCmd = QString("SELECT project_id, event, timestamp FROM calendars WHERE  timestamp BETWEEN '%1' AND '%2'").arg(m_date.toString(Qt::ISODate), end_date.toString(Qt::ISODate));
    results = db_->queryRow(sqlCmd);
    deadline_dates_.clear();
    for(int i = 0; i < results.size(); i+=3)
    {
        int j = i + 1;
        int k = i + 2;
        int id_ = results[i].toInt();

        if(!projectMap.contains(id_))
            continue;

        auto timestamp = results[k].split(" ").front();
        auto event = QString("[%1]: %3 - %2").arg(projectMap[id_], results[j], timestamp);

        qInfo() << timestamp << event;
        deadline_dates_[timestamp].append(event);
    }

    emit dateChanged();
}

int CalendarView::daysInMonth() const
{
    int days = m_calendar.daysInMonth(m_date.month(), m_date.year());
    qDebug() << "Days in month" << m_date.month() << ":" << days;
    return days;
}

int CalendarView::firstDayOfWeek() const
{
    QDate firstDay(m_date.year(), m_date.month(), 1);
    int dayOfWeek = firstDay.dayOfWeek(); // 1 = Mon, 2 = Tue, ..., 7 = Sun

    qDebug() << "First day of" << m_date.year() << "/" << m_date.month()
             << "is day" << dayOfWeek << "(1=Mon, 7=Sun)";

    return dayOfWeek;
}

bool CalendarView::isWeekend(int day) const
{
    if (day < 1 || day > daysInMonth()) {
        return false;
    }

    QDate date(m_date.year(), m_date.month(), day);
    int dow = date.dayOfWeek(); // 1 = Mon ... 7 = Sun
    return (dow == 6 || dow == 7); // Saturday or Sunday
}

bool CalendarView::hasDeadline(QString date) const
{

    return deadline_dates_.contains(date);
}

QString CalendarView::getEvent(int day) const
{
    QDate eventDay(m_date.year(), m_date.month(), day);
    if(deadline_dates_.contains(eventDay.toString(Qt::ISODate)))
        return deadline_dates_[eventDay.toString(Qt::ISODate)].join("\n");
    return "";
}

void CalendarView::updateCalendarDB(const QString &db_path)
{
    setMonth(m_date.month());
}
