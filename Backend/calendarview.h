#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <QObject>
#include <QDate>
#include <QCalendar>
#include <QLocale>
#include <QHash>
#include "database.h"

namespace project{
class CalendarView : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int year READ year WRITE setYear NOTIFY dateChanged)
    Q_PROPERTY(int month READ month WRITE setMonth NOTIFY dateChanged)

public:
    explicit CalendarView(DbmPtr db, QObject *parent = nullptr);

    int year() const;
    int month() const;

    Q_INVOKABLE int daysInMonth() const;
    Q_INVOKABLE int firstDayOfWeek() const;
    Q_INVOKABLE bool isWeekend(int day) const;
    Q_INVOKABLE bool hasDeadline(QString date) const;
    Q_INVOKABLE QString getEvent(int day) const;
    Q_INVOKABLE void updateCalendarDB(const QString& db_path);

public slots:
    void setYear(int y);
    void setMonth(int m);

signals:
    void dateChanged();

private:
    QDate m_date;
    QCalendar m_calendar;
    DbmPtr db_;
    QHash<QString, QStringList> deadline_dates_;
};
}

#endif // CALENDARVIEW_H
