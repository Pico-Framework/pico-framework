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

  async fetchLog() {
    try {
      const res = await fetch('/api/v1/logs/summary');
      if (!res.ok) throw new Error('Failed to fetch log');
      const log = await res.text();
      this.logEl.textContent = log.trim() || '(no logs yet)';
      this.logEl.scrollTop = this.logEl.scrollHeight;
    } catch (err) {
      this.logEl.textContent = 'No logged data available.';
    }
  }
}

customElements.define('sprinkler-log', SprinklerLogViewer);
