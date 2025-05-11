class SiteHeader extends HTMLElement {
    connectedCallback() {
      this.innerHTML = `
        <nav class="site-header">
          <a href="#/">Dashboard</a>
          <a href="#/zones">Zones</a>
          <a href="#/programs">Programs</a>
        </nav>
      `;
    }
  }
  customElements.define('site-header', SiteHeader);
  