import { apiGet } from '../utils/api.js';

class SprinklerLogViewer extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `
      <div class="panel">
        <div class="header">
          <section>
            <h2>System Log</h2>
            <pre class="log-output">Loading log...</pre>
          </section>
        </div>
      </div>
    `;

    this.logEl = this.querySelector('.log-output');
    this.fetchLog();

    this.interval = setInterval(() => this.fetchLog(), 5000); // update every 5 seconds
  }

  disconnectedCallback() {
    clearInterval(this.interval);
  }

  convertLogLineToLocalTime(line) {
    const match = line.match(/^\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\]/);
    if (!match) return line;

    const utcString = match[1].replace(' ', 'T') + 'Z';
    const date = new Date(utcString);

    const localTime = date.toLocaleString(undefined, {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
      timeZoneName: 'short'
    });

    return line.replace(match[0], `[${localTime}]`);
  }

  async fetchLog() {
    try {
      const res = await fetch('/api/v1/logs/summary');
      if (!res.ok) throw new Error('Failed to fetch log');
      const rawLog = await res.text();

      const localized = rawLog
        .split('\n')
        .map(this.convertLogLineToLocalTime.bind(this)) // ensure `this` is correct
        .join('\n')
        .trim();

      this.logEl.textContent = localized || '(no logs yet)';
      this.logEl.scrollTop = this.logEl.scrollHeight;
    } catch (err) {
      this.logEl.textContent = 'No logged data available.';
    }
  }
}

customElements.define('sprinkler-log', SprinklerLogViewer);
