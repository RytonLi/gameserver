using System;

public class TimeStampConvert
{
    static public long Now()
    {
        return DateTime.Now.Ticks / 10000;
    }

    static public DateTime GetDateTime(long timeStamp)//ʱ���Timestampת��������
    {
        DateTime dtStart = TimeZone.CurrentTimeZone.ToLocalTime(new DateTime(1970, 1, 1));
        long lTime = (timeStamp * 10000000);
        TimeSpan toNow = new TimeSpan(lTime);
        DateTime targetDt = dtStart.Add(toNow);
        return targetDt;
    }
}
