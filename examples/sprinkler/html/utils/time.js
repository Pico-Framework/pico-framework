export function utcToLocalString(utcString, opts = {}) {
    const normalized = utcString.includes('T') ? utcString : utcString.replace(' ', 'T') + 'Z';
    const date = new Date(normalized);
    return date.toLocaleString(undefined, {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit',
        timeZoneName: 'short',
        ...opts
    });
}

export function localTimeToUTCString(localTimeStr, date = new Date()) {
    // localTimeStr = "19:00" or "07:30"
    const [hours, minutes] = localTimeStr.split(':').map(Number);
    date.setHours(hours, minutes, 0, 0);

    const utcMillis = date.getTime() - (date.getTimezoneOffset() * 60000);
    return new Date(utcMillis).toISOString();
}

export function parseLocalDateTimeInput(dateStr, timeStr) {
    // e.g., "2025-05-16", "19:00"
    const [year, month, day] = dateStr.split('-').map(Number);
    const [hours, minutes] = timeStr.split(':').map(Number);

    const local = new Date();
    local.setFullYear(year, month - 1, day);
    local.setHours(hours, minutes, 0, 0);

    const utcMillis = local.getTime() - (local.getTimezoneOffset() * 60000);
    return new Date(utcMillis).toISOString();
}

export function utcToLocalTimeString(utcTimeOnly) {
    // Assume utcTimeOnly is "HH:MM"
    const [hours, minutes] = utcTimeOnly.split(':').map(Number);
  
    // Create a Date in UTC for today at that time
    const utcDate = new Date(Date.UTC(1970, 0, 1, hours, minutes, 0));
  
    // Convert to local time and format as HH:MM
    return utcDate.toLocaleTimeString(undefined, {
      hour: '2-digit',
      minute: '2-digit',
      hour12: false
    });
}

export function isoUtcToLocalTimeString(isoString) {
    // Ensure it's treated as UTC
    const normalized = isoString.endsWith('Z') ? isoString : isoString + 'Z';
    const date = new Date(normalized);
  
    return date.toLocaleTimeString(undefined, {
      hour: '2-digit',
      minute: '2-digit',
      hour12: false
    });
  }
